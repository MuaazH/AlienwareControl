#pragma once

#include <string>
#include <vector>
#include <wbemidl.h>
#include <wtypes.h>

using namespace std;

#define byte BYTE

namespace AlienFan_SDK {

	struct ALIENFAN_SEN_INFO {
		union {
			struct {
				byte index;
				byte type;
			};
			WORD sid; // LOBYTE - index, HIBYTE - type: 0 = ESIF, 1 = AWCC, 2 - Disk, 3 - AMD, 4 = OHM
		};
		string name;
		BSTR instance; // for ESIF/OHM/SSD sensors
		BSTR valueName;
	};

	struct ALIENFAN_FAN_INFO {
		byte id, type;
	};

	enum {
		getPowerID	 =	0,
		getFanRPM	 =	1,
		getFanPercent=	2,
		getFanBoost	 =	3,
		getTemp		 =	4,
		getPowerMode =	5,
		setFanBoost	 =	6,
		setPowerMode =	7,
		getGMode	 =	8,
		setGMode	 =	9,
		getSysID	 =	10,
		getFanSensor =	11,
		getMaxRPM	 =	12
	};

	union ALIENFAN_INTERFACE {
		struct {
			byte sub,
			     arg1,
			     arg2,
			     reserved;
		};
		DWORD args;
	};

	class Control {
	private:
		DWORD systemID = 0;
		byte sysType = -1;
		void EnumSensors(IEnumWbemClassObject* enum_obj, byte type);
	public:
		VARIANT m_instancePath;
		IWbemServices* m_WbemServices = NULL, * m_OHMService = NULL, * m_DiskService = NULL;
		IWbemClassObject* m_InParamaters = NULL;
		bool isAlienware = false,
			 isSupported = false,
			 isGmode = false;

		Control();
		~Control();

		// Probe hardware, sensors, fans, power modes and fill structures.
		// Result: true - compatible hardware found, false - not found.
		bool Probe();

		// Get RPM for the fan index fanID at fans[]
		// Result: fan RPM
		int GetFanRPM(int fanID);

		// Get Max. RPM for fan index fanID
		int GetMaxRPM(int fanID);

		// Get fan RPMs as a percent of RPM
		// Result: percent of the fan speed
		int GetFanPercent(int fanID);

		// Get boost value for the fan index fanID at fans[]. If force, raw value returned, otherwise cooked by boost
		// Result: Error or raw value if forced, otherwise cooked by boost.
		int GetFanBoost(int fanID/*, bool force = false*/);

		// Set boost value for the fan index fanID at fans[]. If force, raw value set, otherwise cooked by boost.
		// Result: value or error
		int SetFanBoost(int fanID, byte value/*, bool force = false*/);

		// Get temperature value for the sensor index TanID at sensors[]
		// Result: temperature value or error
		int GetTempValue(int TempID);

		// Unlock manual fan operations. The same as SetPower(0)
		// Result: raw value set or error
		int Unlock();

		// Set system power profile to defined power code (NOT level)
		// Result: raw value set or error
		int SetPower(byte level);

		// Get current system power value index at powers[]
		// Result: power value index in powers[] or error
		int GetPower();

		// Set system GPU limit level (0 - no limit, 3 - min. limit)
		// Result: success or error
		//int SetGPU(int power);

		// Toggle G-mode on some systems
		int SetGMode(bool state);

		// Check G-mode state
		int GetGMode();

		// Return current device ID
		inline DWORD GetSystemID() { return systemID; };

		// Call custom Alienware method trough WMI
		int CallWMIMethod(byte com, byte arg1 = 0, byte arg2 = 0);

		// Arrays of sensors, fans, max. boosts and power values detected at Probe()
		vector<ALIENFAN_SEN_INFO> sensors;
		vector<ALIENFAN_FAN_INFO> fans;
		vector<byte> powers;

		IWbemClassObject* m_AWCCGetObj = NULL;
	};


}
