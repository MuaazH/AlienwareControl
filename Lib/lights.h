#ifndef ALIENWARE_LIGHTS_H
#define ALIENWARE_LIGHTS_H


#include "../../mlib/ArrayDeque.h"
#include <wtypes.h>

namespace alienware {

	#define ALEINWARE_VID 0x187c
	#define ALEINWARE_LEN 34

//	enum LightsVersion {
//		API_ACPI = 0, //128
//		API_V8 = 8, //65
//		API_V7 = 7, //65
//		API_V6 = 6, //65
//		API_V5 = 5, //64
//		API_V4 = 4, //34
//		API_V3 = 3, //12
//		API_V2 = 2, //9
//		API_V1 = 1, //8
//		API_UNKNOWN = -1
//	};

//	struct Command {
//		byte i, val;
//	};

	enum ActionType {
		AlienFX_A_Color = 0,
		AlienFX_A_Pulse = 1,
		AlienFX_A_Morph = 2,
		AlienFX_A_Breathing= 3,
		AlienFX_A_Spectrum = 4,
		AlienFX_A_Rainbow = 5,
		AlienFX_A_Power = 6
	};

	struct Action { // atomic light action phase
		byte type; // one of Action values - action type
		byte time; // How long this phase stay
		byte tempo; // How fast it should transform
		byte r; // phase color r
		byte g; // phase color g
		byte b; // phase color b
	};

	struct LightBlock { // light action block
		byte index;
		ArrayDeque<Action *> act;
	};

	class Lights {
	private:
		HANDLE m_DevHandle = nullptr;
		int m_ProductID = -1;
		int m_VenderID = -1;
		bool m_WasReset = false;
		byte m_Buffer[ALEINWARE_LEN];

		void SetCommand(const byte *command);
		bool Send();
		bool ProbeDevice(void* hDevInfo, void* devData, int vid, int pid);
		void WaitForBusy();
		void WaitForReady();
		byte GetDeviceStatus();

		// false - not ready, true - ready, 0xff - stalled
		BYTE IsDeviceReady();
		bool UpdateColors();
		bool SetAction(byte index, ArrayDeque<Action *> *act);
		bool SetPowerAction(ArrayDeque<LightBlock *> *act, bool save);

	public:
		Lights();
		~Lights();

		int Init(int vid, int pid = -1);
		bool Reset();
		void Close();
		bool Update(ArrayDeque<LightBlock *> *act, bool store = false);
		bool TurnOn(unsigned int brightness);
	};
}

#endif