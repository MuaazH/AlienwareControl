#define WIN32_LEAN_AND_MEAN

#include "alienfan-SDK.h"
#include "alienfan-controls.h"

#pragma comment(lib, "wbemuuid.lib")

//#define _TRACE_

namespace AlienFan_SDK {

	Control::Control() {
        GUID CLSID_WbemLocator = {0x4590f811, 0x1d3a, 0x11d0, 0x89, 0x1f,0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24 };
        GUID IID_IWbemLocator = { 0xdc12a687, 0x737f, 0x11cf, 0x88, 0x4d, 0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24 };

		IWbemLocator* m_WbemLocator;
		CoInitializeEx(nullptr, COINIT::COINIT_MULTITHREADED);
		CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
			RPC_C_AUTHN_LEVEL_NONE, //RPC_C_AUTHN_LEVEL_CONNECT,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			nullptr, EOAC_NONE, nullptr);

		CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&m_WbemLocator);
		m_WbemLocator->ConnectServer((BSTR)L"ROOT\\WMI", nullptr, nullptr, nullptr, NULL, nullptr, nullptr, &m_WbemServices);

		m_WbemLocator->Release();
	}

	Control::~Control() {
		if (m_DiskService)
			m_DiskService->Release();
		if (m_OHMService)
			m_OHMService->Release();
		if (m_InParamaters)
			m_InParamaters->Release();
		if (m_AWCCGetObj)
			m_AWCCGetObj->Release();
		if (m_WbemServices)
			m_WbemServices->Release();
		CoUninitialize();
	}

	int Control::CallWMIMethod(byte com, byte arg1, byte arg2) {
		VARIANT result{ VT_I4 };
		result.intVal = -1;
		if (sysType >= 0) {
			IWbemClassObject* m_outParameters = NULL;
			VARIANT parameters = { VT_I4 };
			parameters.uintVal = ALIENFAN_INTERFACE{ dev_controls[com], arg1, arg2, 0 }.args;
			m_InParamaters->Put(L"arg2", NULL, &parameters, 0);
			if (m_WbemServices->ExecMethod(m_instancePath.bstrVal,
				commandList[functionID[sysType][com]], 0, NULL, m_InParamaters, &m_outParameters, NULL) == S_OK && m_outParameters) {
				m_outParameters->Get(L"argr", 0, &result, nullptr, nullptr);
				m_outParameters->Release();
			}
		}
		return result.intVal;
	}

	void Control::EnumSensors(IEnumWbemClassObject* enum_obj, byte type) {
		IWbemClassObject* spInstance[64];
		ULONG uNumOfInstances = 0;
		//BSTR instansePath{ (BSTR)L"__Path" }, valuePath{ (BSTR)L"Temperature" };
		LPCWSTR instansePath = L"__Path", valuePath = L"Temperature";
		VARIANT cTemp, instPath;
		string name;

		switch (type) {
		case 0:
			name = "ESIF sensor ";
			break;
		case 2:
			name = "SSD sensor ";
			instansePath = L"StorageReliabilityCounter";
			break;
		case 4:
			valuePath = L"Value";
		}

		enum_obj->Next(3000, 64, spInstance, &uNumOfInstances);
		byte senID = 0;
		for (byte ind = 0; ind < uNumOfInstances; ind++) {
			if (type == 4) { // OHM sensors
				VARIANT type;
				spInstance[ind]->Get(L"SensorType", 0, &type, 0, 0);
				if (type.bstrVal == wstring(L"Temperature")) {
					VARIANT vname;
					spInstance[ind]->Get(L"Name", 0, &vname, 0, 0);
					wstring sname{ vname.bstrVal };
					name = string(sname.begin(), sname.end());
				}
				else {
					continue;
				}
			}
			spInstance[ind]->Get(instansePath, 0, &instPath, 0, 0);
			spInstance[ind]->Get(valuePath, 0, &cTemp, 0, 0);
			spInstance[ind]->Release();
			if (cTemp.uintVal > 0 || cTemp.fltVal > 0)
				sensors.push_back({ { senID++,type }, name + (type != 4 ? to_string(senID) : ""), instPath.bstrVal, (BSTR)valuePath });
		}
		enum_obj->Release();
#ifdef _TRACE_
		printf("%d sensors of #%d added, %d total\n", uNumOfInstances, type, (int)sensors.size());
#endif
	}

	bool Control::Probe() {
		if (m_WbemServices && m_WbemServices->GetObject((BSTR)L"AWCCWmiMethodFunction", NULL, nullptr, &m_AWCCGetObj, nullptr) == S_OK) {
			// need to get instance
			IEnumWbemClassObject* enum_obj;
			if (m_WbemServices->CreateInstanceEnum((BSTR)L"AWCCWmiMethodFunction", WBEM_FLAG_FORWARD_ONLY, NULL, &enum_obj) == S_OK) {
				IWbemClassObject* spInstance;
				ULONG uNumOfInstances = 0;
				enum_obj->Next(10000, 1, &spInstance, &uNumOfInstances);
				spInstance->Get((BSTR)L"__Path", 0, &m_instancePath, 0, 0);
				spInstance->Release();
				enum_obj->Release();
				isAlienware = true;

				isGmode = m_AWCCGetObj->GetMethod(commandList[2], NULL, nullptr, nullptr) == S_OK;
				// check system type and fill inParams
				for (int type = 0; type < 2; type++)
					if ((isSupported = m_AWCCGetObj->GetMethod(commandList[functionID[type][getPowerID]], NULL, &m_InParamaters, nullptr) == S_OK) && m_InParamaters) {
						sysType = type;
						systemID = CallWMIMethod(getSysID, 2);
						int fIndex = 0; byte funcID;
						// Scan for available fans...
						while ((funcID = CallWMIMethod(getPowerID, fIndex) & 0xff) > 0x2f) {
							fans.push_back({ funcID, 0xff });
							fIndex++;
						}
						// AWCC temperature sensors.
						while (funcID && funcID < 0xa0) {
							string sName = sensors.size() < 2 ? temp_names[sensors.size()] : "Sensor #" + to_string(sensors.size());
							// fan mappings...
							for (auto fan = fans.begin(); fan != fans.end(); fan++)
								if (funcID == CallWMIMethod(getFanSensor, fan->id)) {
									fan->type = (byte)sensors.size();
								}
							sensors.push_back({ { funcID, 1 }, sName });
							fIndex++;
							funcID = CallWMIMethod(getPowerID, fIndex) & 0xff;
						}
						// Power modes.
						powers.push_back(0); // Manual mode
						while (funcID && funcID != 0xff) {
							powers.push_back(funcID);
							fIndex++;
							funcID = CallWMIMethod(getPowerID, fIndex) & 0xff;
						}
						if (sysType) {
							// Modes 1 and 2 for R7 desktop
							powers.push_back(1);
							powers.push_back(2);
						}
						break;
					}
			}
		}
		return isSupported;
	}

	int Control::GetFanRPM(int fanID) {
		return fanID < fans.size() ? CallWMIMethod(getFanRPM, (byte)fans[fanID].id) : -1;
	}
	int Control::GetMaxRPM(int fanID)
	{
		return fanID < fans.size() ? CallWMIMethod(getMaxRPM, (byte)fans[fanID].id) : -1;
	}
	int Control::GetFanPercent(int fanID) {
		return fanID < fans.size() ? CallWMIMethod(getFanPercent, (byte)fans[fanID].id) : -1;
	}
	int Control::GetFanBoost(int fanID) {
		return fanID < fans.size() ? CallWMIMethod(getFanBoost, (byte)fans[fanID].id) : -1;
	}
	int Control::SetFanBoost(int fanID, byte value) {
		return fanID < fans.size() ? CallWMIMethod(setFanBoost, (byte)fans[fanID].id, value) : -1;
	}
	int Control::GetTempValue(int TempID) {
		IWbemClassObject* sensorObject = NULL;
		IWbemServices* serviceObject = m_WbemServices;
		VARIANT temp;
		if (TempID < sensors.size()) {
			switch (sensors[TempID].type) {
			case 1: { // AWCC
				int awt = CallWMIMethod(getTemp, sensors[TempID].index);
				// Bugfix for AWCC temp - it can be up to 5000C!
				return awt > 200 ? -1 : awt;
			} break;
			case 2: // SSD
				serviceObject = m_DiskService;
				break;
			case 4: // OHM
				serviceObject = m_OHMService;
				break;
			}
			if (serviceObject->GetObject(sensors[TempID].instance, NULL, nullptr, &sensorObject, nullptr) == S_OK) {
				sensorObject->Get(sensors[TempID].valueName, 0, &temp, 0, 0);
				sensorObject->Release();
				return sensors[TempID].type == 4 ? (int)temp.fltVal : temp.uintVal;
			}
		}
		return -1;
	}
	int Control::Unlock() {
		return SetPower(0);
	}
	int Control::SetPower(byte level) {
		return CallWMIMethod(setPowerMode, level);
	}
	int Control::GetPower() {
		int pl = CallWMIMethod(getPowerMode);
		for (int i = 0; pl >= 0 && i < powers.size(); i++)
			if (powers[i] == pl)
				return i;
		return -1;
	}

	int Control::SetGMode(bool state)
	{
		if (isGmode) {
			if (state)
				SetPower(0xAB);
			return CallWMIMethod(setGMode, state);
		}
		return -1;
	}

	int Control::GetGMode() {
		return isGmode ? 
			GetPower() < 0 ? 1 : CallWMIMethod(getGMode)
			: 0;
	}

}
