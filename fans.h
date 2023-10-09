#ifndef ALIENWARE_FANS_H
#define ALIENWARE_FANS_H

#include "../mlib/ArrayDeque.h"
#include <wtypes.h>
#include <wbemidl.h>

namespace alienware {

	struct FanInfo {
		int id, type;
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

#define MAX_SENSOR_COUNT 32
#define MAX_FAN_COUNT 32

	class FansControl {
	private:
		byte                   m_SysType;
		DWORD                  m_systemID;
		VARIANT                m_instancePath;
		IWbemClassObject      *m_pInParamaters;
		IWbemServices         *m_pWbemServices;
		IWbemClassObject      *m_pAWCCGetObj;
		// Arrays of sensors, fans, max. boosts and power values detected at Probe()
		byte                   m_Sensors[MAX_SENSOR_COUNT];
		FanInfo                m_Fans[MAX_FAN_COUNT];
		unsigned int           m_SensorCount;
		unsigned int           m_FanCount;
		ArrayDeque<byte>       m_powers;

		bool Probe();

	public:

		FansControl();
		~FansControl();

		// Probe hardware, sensors, fans, power modes and fill structures.
		// Result: true - compatible hardware found, false - not found.
		bool Init();

		// Set boost value for the fan index fanID at fans[]. If force, raw value set, otherwise cooked by boost.
		// Result: value or error
		int SetFanBoost(unsigned int fan, byte value);

		// Get boost value for the fan index fanID at fans[]. If force, raw value returned, otherwise cooked by boost
		// Result: Error or raw value if forced, otherwise cooked by boost.
		int GetFanBoost(unsigned int fan);

		// Get RPM for the fan index fanID at fans[]
		// Result: fan RPM
		int GetFanRPM(unsigned int fan);

		// Get Max. RPM for fan index fanID
		int GetMaxRPM(unsigned int fan);

		// Get fan RPMs as a percent of RPM
		// Result: percent of the fan speed
		int GetFanPercent(unsigned int fan);

		// Get temperature value for the sensor index TanID at sensors[]
		// Result: temperature value or error
		int GetTemperature(unsigned int sensor);

		// Set system power profile to defined power code (NOT level)
		// Result: raw value set or error
		bool SetPower(byte level);

		// Get current system power value index at powers[]
		// Result: power value index in powers[] or error
		int GetPower();

		// Toggle G-mode on some systems
		bool SetGMode(bool state);

		// Check G-mode state
		int GetGMode();

		// Call custom Alienware method trough WMI
		int CallWMIMethod(byte com, byte arg1 = 0, byte arg2 = 0);

		// Return current device ID
		inline DWORD GetSystemID() { return m_systemID; };

		unsigned int GetFanCount() { return m_FanCount; }

		unsigned int GetSensorCount() { return m_SensorCount; }

		int GetFanID(unsigned int fan) { return fan < m_FanCount ? m_Fans[fan].id : -1; }
	};

}

#endif