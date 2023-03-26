
#include "fans.h"

static const BSTR commandList[7]{
	(BSTR)L"Thermal_Information",	// 0x14
	(BSTR)L"Thermal_Control",		// 0x15
	(BSTR)L"GameShiftStatus",		// 0x25
	(BSTR)L"SystemInformation",		// 0x1A
	(BSTR)L"GetFanSensors",			// 0x13
	(BSTR)L"GetThermalInfo2",		// 0x10
	(BSTR)L"SetThermalControl2"		// 0x11
};

//static const BSTR colorList[2]{
//	(BSTR)L"Set24BitsLEDColor",		// 0x12
//	(BSTR)L"LEDBrightness"			// 0x03
//};

static const byte functionID[2][13] {
	{ 0,0,0,0,0,0,1,1,2,2,3,4,0 },
	{ 5,5,5,5,5,5,6,6,2,2,3,4,5 }
};

static const byte devControls[13]{
	3, // PowerID
	5, // RPM
	6, // Percent
	0xc, // Get Boost
	4, // Temp
	0xb, // Get Power
	2, // Set boost
	1, // Set Power
	2, // Get G-Mode
	1, // Toggle G-Mode
	2, // Get system ID
	2, // Get fan sensor ID
	9  // Get Max. RPM
};

/*
static const char* tempNames[2]{
		"CPU Internal Thermistor",
		"GPU Internal Thermistor"//,
		//"Motherboard Thermistor",
		//"CPU External Thermistor",
		//"Memory Thermistor",
		//"GPU External Thermistor"
};
*/

union AlienFanInterface {
	struct {
		byte sub;
		byte arg1;
		byte arg2;
		byte reserved;
	};
	DWORD args;
};

namespace alienware {

	FansControl::FansControl() {
		m_pInParamaters = nullptr;
		m_pWbemServices = nullptr;
		m_pAWCCGetObj = nullptr;
		m_SysType = -1;
		m_SensorCount = 0;
		m_FanCount = 0;
	}

	FansControl::~FansControl() {
		if (m_pInParamaters)
			m_pInParamaters->Release();

		if (m_pWbemServices)
			m_pWbemServices->Release();

		if (m_pAWCCGetObj)
			m_pAWCCGetObj->Release();

		CoUninitialize();
	}

	bool FansControl::Init() {
		GUID CLSID_WbemLocator = { 0x4590f811, 0x1d3a, 0x11d0, 0x89, 0x1f,0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24 };
		GUID IID_IWbemLocator = { 0xdc12a687, 0x737f, 0x11cf, 0x88, 0x4d, 0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24 };

		if (CoInitializeEx(nullptr, COINIT::COINIT_MULTITHREADED) != S_OK) {
			return false;
		}
		if (CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr) != S_OK) {
			return false;
		}

		IWbemLocator* pWbemLocator = nullptr;

		bool ok = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&pWbemLocator) == S_OK;
		if (ok)
			ok = 0 == pWbemLocator->ConnectServer((BSTR)L"ROOT\\WMI", nullptr, nullptr, nullptr, 0, nullptr, nullptr, &m_pWbemServices);

		if (pWbemLocator)
			pWbemLocator->Release();

		return ok && Probe();
	}

	bool FansControl::Probe() {
		bool isSupported = false;
		if (m_pWbemServices && m_pWbemServices->GetObject((BSTR)L"AWCCWmiMethodFunction", 0, nullptr, &m_pAWCCGetObj, nullptr) == S_OK) {
			// need to get instance
			IEnumWbemClassObject* enum_obj;
			if (m_pWbemServices->CreateInstanceEnum((BSTR)L"AWCCWmiMethodFunction", WBEM_FLAG_FORWARD_ONLY, NULL, &enum_obj) == S_OK) {
				IWbemClassObject* spInstance;
				ULONG uNumOfInstances = 0;
				enum_obj->Next(10000, 1, &spInstance, &uNumOfInstances);
				spInstance->Get((BSTR)L"__Path", 0, &m_instancePath, 0, 0);
				spInstance->Release();
				enum_obj->Release();

				m_pAWCCGetObj->GetMethod(commandList[2], 0, nullptr, nullptr);

				// check system type and fill inParams
				for (int type = 0; type < 2; type++) {
					isSupported = m_pAWCCGetObj->GetMethod(commandList[functionID[type][getPowerID]], 0, &m_pInParamaters, nullptr) == S_OK;
					if (isSupported && m_pInParamaters) {
						m_SysType = type;
						m_systemID = CallWMIMethod(getSysID, 2);
						int fIndex = 0;
						byte funcID;
						// Scan for available fans...
						while ((funcID = CallWMIMethod(getPowerID, fIndex) & 0xff) > 0x2f) {
							m_Fans[m_FanCount].id = funcID;
							m_Fans[m_FanCount].type = 0xff;
							m_FanCount++;
							fIndex++;
						}
						// AWCC temperature sensors.
						while (funcID && funcID < 0xa0) {
							// fan mappings...
							for (unsigned int i = 0; i < m_FanCount; i++) {
								if (funcID == CallWMIMethod(getFanSensor, m_Fans[i].id)) {
									m_Fans[i].type = (byte)m_SensorCount;
								}
							}
							m_Sensors[m_SensorCount] = funcID;
							m_SensorCount++;
							fIndex++;
							funcID = CallWMIMethod(getPowerID, fIndex) & 0xff;
						}
						// Power modes.
						m_powers.Add(0); // Manual mode
						while (funcID && funcID != 0xff) {
							m_powers.Add(funcID);
							fIndex++;
							funcID = CallWMIMethod(getPowerID, fIndex) & 0xff;
						}
						//if (sysType) {
						//	// Modes 1 and 2 for R7 desktop
						//	m_powers.Add(1);
						//	m_powers.Add(2);
						//}
						break;
					}
				}
			}
		}
		return isSupported;
	}

	bool FansControl::SetPower(byte level) {
		return !CallWMIMethod(setPowerMode, level);
	}

	bool FansControl::SetGMode(bool state) {
		if (state) {
			return SetPower(0xAB) && CallWMIMethod(setGMode, state) == 1;
		} else {
			return SetPower(0x0);
		}
	}

	int FansControl::CallWMIMethod(byte com, byte arg1, byte arg2) {
		VARIANT result { VT_I4 };
		result.intVal = -1;
		if (m_SysType >= 0) {
			IWbemClassObject* pOutParameters = NULL;
			VARIANT parameters = { VT_I4 };
			parameters.uintVal = AlienFanInterface{ devControls[com], arg1, arg2, 0 }.args;
			m_pInParamaters->Put(L"arg2", 0, &parameters, 0);
			if (m_pWbemServices->ExecMethod(m_instancePath.bstrVal,
				commandList[functionID[m_SysType][com]], 0, NULL, m_pInParamaters, &pOutParameters, NULL) == S_OK && pOutParameters) {
				pOutParameters->Get(L"argr", 0, &result, nullptr, nullptr);
				pOutParameters->Release();
			}
			return result.intVal;
		}
		return -1;
	}
}
