
#include "lights.h"
extern "C" {
#include <hidclass.h>
#include <hidsdi.h>
}
#include <SetupAPI.h>
#include "lightsConstants.h"

// #include <iostream>

namespace alienware {

	// Statuses for v1-v3
	#define ALIENFX_V2_RESET 0x06
	#define ALIENFX_V2_READY 0x10
	#define ALIENFX_V2_BUSY 0x11
	#define ALIENFX_V2_UNKNOWN 0x12
	// Statuses for apiv4
	#define ALIENFX_V4_READY 33
	#define ALIENFX_V4_BUSY 34
	#define ALIENFX_V4_WAITCOLOR 35
	#define ALIENFX_V4_WAITUPDATE 36
	#define ALIENFX_V4_WASON 38
	// Statuses for apiv5
	#define ALIENFX_V5_STARTCOMMAND 0x8c
	#define ALIENFX_V5_WAITUPDATE 0x80
	#define ALIENFX_V5_INCOMMAND 0xcc

	Lights::Lights() {
	}

	Lights::~Lights() {
		Close();
	}

#define openDevHandleFlags1 GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE
#define openDevHandleFlags2 OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN | SECURITY_ANONYMOUS /*FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH*/
	bool Lights::ProbeDevice(void* hDevInfo, void* devData, int vid, int pid) {
		DWORD dwRequiredSize = 0;
		bool flag = false;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, (PSP_DEVICE_INTERFACE_DATA)devData, nullptr, 0, &dwRequiredSize, nullptr);
		SP_DEVICE_INTERFACE_DETAIL_DATA* deviceInterfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA*)new byte[dwRequiredSize];
		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		if (SetupDiGetDeviceInterfaceDetail(hDevInfo, (PSP_DEVICE_INTERFACE_DATA)devData, deviceInterfaceDetailData, dwRequiredSize, nullptr, nullptr)) {
			m_DevHandle = CreateFile(deviceInterfaceDetailData->DevicePath, openDevHandleFlags1, nullptr, openDevHandleFlags2, nullptr);
			if (m_DevHandle != INVALID_HANDLE_VALUE) {
				PHIDP_PREPARSED_DATA prep_caps;
				HIDP_CAPS caps;
				HIDD_ATTRIBUTES attributes{ sizeof(HIDD_ATTRIBUTES) };
				if (HidD_GetAttributes(m_DevHandle, &attributes) &&
					(vid == -1 || attributes.VendorID == vid) && (pid == -1 || attributes.ProductID == pid)) {
					HidD_GetPreparsedData(m_DevHandle, &prep_caps);
					HidP_GetCaps(prep_caps, &caps);
					HidD_FreePreparsedData(prep_caps);
					int length = caps.OutputReportByteLength;
					m_VenderID = attributes.VendorID;

					// Alienware
					if (ALEINWARE_VID == m_VenderID && ALEINWARE_LEN == length) {
						// m_Version = API_V4;
						flag = true;
						m_ProductID = attributes.ProductID;
					} else {
						CloseHandle(m_DevHandle);
						m_DevHandle = nullptr;
						m_ProductID = -1;
					}
				}
			}
		}
		delete[] deviceInterfaceDetailData;
		return flag;
	}

	//Use this method for general devices, vid = -1 for any vid, pid = -1 for any pid.
	int Lights::Init(int vid, int pid) {
		if (m_ProductID > 0) {
			return m_ProductID;
		}
		GUID guid;
		SP_DEVICE_INTERFACE_DATA deviceInterfaceData{ sizeof(SP_DEVICE_INTERFACE_DATA) };

		HidD_GetHidGuid(&guid);
		HDEVINFO hDevInfo = SetupDiGetClassDevs(&guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (hDevInfo != INVALID_HANDLE_VALUE) {
			DWORD dw = 0;
			while (SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, &guid, dw, &deviceInterfaceData) && !ProbeDevice(hDevInfo, &deviceInterfaceData, vid, pid))
				dw++;
			SetupDiDestroyDeviceInfoList(hDevInfo);
		}
		return m_ProductID;
	}

	void Lights::SetCommand(const byte *command) {
		memset(m_Buffer, 0, ALEINWARE_LEN);
		// m_Buffer[0] = reportIDList[m_Version];
		m_Buffer[0] = 0;
		DWORD size = command[0];
		memcpy(m_Buffer + 1, command + 1, size);
	}

	bool Lights::Send() {
//		for (int i = 0; i < ALEINWARE_LEN; i++) {
//			std::cout << ((int)m_Buffer[i] < 16 ? "0" : "") << std::hex << (int) m_Buffer[i];
//		}
//		std::cout << std::endl;

		DWORD written;
		bool res = DeviceIoControl(m_DevHandle, IOCTL_HID_SET_OUTPUT_REPORT, m_Buffer, ALEINWARE_LEN, 0, 0, &written, nullptr);
		res &= DeviceIoControl(m_DevHandle, IOCTL_HID_GET_INPUT_REPORT, 0, 0, m_Buffer, ALEINWARE_LEN, &written, nullptr);
		return res;
	}

	bool Lights::Update(ArrayDeque<LightBlock *> *act, bool save) {
		if (save)
			return SetPowerAction(act, true);

		if (!m_WasReset)
			Reset();

		auto size = act->Size();
		for (unsigned int i = 0; i < size; i++) {
			auto pLight = (*act)[i];
			if (!SetAction(pLight->index, &pLight->act)) {
				Reset();
				return false;
			}
		}
		return UpdateColors();
	}

	bool Lights::SetAction(byte index, ArrayDeque<Action *> *pActions) {
		// fixme: this code is full of bad practices by the original author

		// byte modsSelect[2] = {6, index};
		SetCommand(COMMV4_colorSel);
		m_Buffer[6] = index;
		if (!Send()) {
			return false;
		}

		DWORD bPos = 3;
		SetCommand(COMMV4_colorSet);
		ArrayDeque<Action *>& actions = *pActions;
		for (unsigned int i = 0; i < actions.Size(); i++) {
			auto ca = actions[i];
			// 3 actions per record..

			m_Buffer[bPos++] = (byte) (ca->type < AlienFX_A_Breathing ? ca->type : AlienFX_A_Morph);
			m_Buffer[bPos++] = ca->time;
			m_Buffer[bPos++] = v4OpCodes[ca->type];
			bPos++;
			m_Buffer[bPos++] = ca->type == AlienFX_A_Color ? 0xfa : ca->tempo;
			m_Buffer[bPos++] = ca->r;
			m_Buffer[bPos++] = ca->g;
			m_Buffer[bPos++] = ca->b;

			if (bPos + 8 >= ALEINWARE_LEN) {
				if (!Send()) {
					return false;
				}
				SetCommand(COMMV4_colorSet);
				bPos = 3;
			}
		}
		if (bPos > 3) {
			return Send();
		}
		return true;
	}

	bool Lights::SetPowerAction(ArrayDeque<LightBlock *> *act, bool save) {
		UpdateColors();
		// byte cmd1[4] = { 4, 0x4, 6, 0x61 };
		SetCommand(COMMV4_control);
		m_Buffer[4] = 0x4;
		m_Buffer[6] = 0x61;
		if (!Send()) {
			return false;
		}

		// byte cmd2[4] = { 4, 0x1, 6, 0x61 };
		SetCommand(COMMV4_control);
		m_Buffer[4] = 0x1;
		m_Buffer[6] = 0x61;
		if (!Send()) {
			return false;
		}

		auto sz = act->Size();
		for (unsigned int i = 0; i < sz; i++) {
			auto ca = (*act)[i];
			if (ca->act[0]->type != AlienFX_A_Power)
				if (!SetAction(ca->index, &ca->act))
					return false;
		}

		// byte cmd3[4] = { 4, 0x2, 6, 0x61 };
		SetCommand(COMMV4_control);
		m_Buffer[4] = 0x2;
		m_Buffer[6] = 0x61;
		if (!Send()) {
			return false;
		}

		// byte cmd4[4] = { 4, 0x6, 6, 0x61 };
		SetCommand(COMMV4_control);
		m_Buffer[4] = 0x6;
		m_Buffer[6] = 0x61;
		return Send();
	}

	bool Lights::Reset() {
		// byte cmd1[2] = {4, 4};
		SetCommand(COMMV4_control);
		m_Buffer[4] = 4;
		if (!Send()) {
			return false;
		}
		
		// byte cmd2[2] = {4, 1};
		SetCommand(COMMV4_control);
		m_Buffer[4] = 1;
		bool result = Send();
		m_WasReset = result;
		return result;
	}

	bool Lights::UpdateColors() {
		bool res = false;
		if (m_WasReset) {
			SetCommand(COMMV4_control);
			res = Send();
			WaitForReady();
			m_WasReset = false;
			Sleep(5); // Fix for ultra-fast updates, or next command will fail sometimes.
		}
		return res;
	}

	void Lights::Close() {
		if (m_DevHandle) {
			CloseHandle(m_DevHandle);
			m_DevHandle = nullptr;
		}
	}

	void Lights::WaitForBusy() {
		int i = 0;
		for (i = 0; i < 500; i++) {
			if (GetDeviceStatus() == ALIENFX_V4_BUSY) {
				break;
			}
			Sleep(5);
		}
	}

	void Lights::WaitForReady() {
		while (!IsDeviceReady()) {
			Sleep(5);
		}
	}

	byte Lights::GetDeviceStatus() {
		byte buffer[ALEINWARE_LEN];
		DWORD written = 0;
		byte res = 0;
		if (DeviceIoControl(m_DevHandle, IOCTL_HID_GET_INPUT_REPORT, 0, 0, buffer, ALEINWARE_LEN, &written, NULL))
			res = buffer[2];

		return res;
	}

	byte Lights::IsDeviceReady() {
		int status = GetDeviceStatus();
		return status ? status != ALIENFX_V4_BUSY : 0xff;
	}

	bool Lights::TurnOn(unsigned int brightness) {
//		vector<Afx_icommand> mods{{3,(byte)(0x64 - bright)}};
//		byte pos = 6;
//		for (auto i = mappings->begin(); i < mappings->end(); i++)
//			if (pos < length && (!i->flags || power)) {
//				mods.push_back({pos,(byte)i->lightid});
//				pos++;
//			}
//		mods.push_back({5,(byte)mappings->size()});
//		return Send(COMMV4_turnOn,  &mods);
		return false;
	}
}
