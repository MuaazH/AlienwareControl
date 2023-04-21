
#include <iostream>
#include "fans/fans.h"
#include <Windows.h>

using namespace alienware;

int main() {

	FansControl fans;
	std::cout << "[ .... ] Initializing" << std::endl;
	if (!fans.Init()) {
		std::cout << "[ FAIL ] Initializing" << std::endl;
		return 0;
	}
	std::cout << "[  OK  ] Initializing" << std::endl;

	std::cout << "[ .... ] Found " << fans.GetFanCount() << " fan(s) and " << fans.GetSensorCount() << " sensor(s)" << std::endl;
	int temp;

	while (1) {
		temp = 0;
		std::cout << std::endl << std::endl << std::endl << std::endl << std::endl;
		for (unsigned int i = 0; i < fans.GetFanCount(); i++) {
			int t = fans.GetTemperature(i);
			temp = t < temp ? temp : t;
			std::cout << "[ .... ] Fan " << fans.GetFanID(i) 
					  << " RPM " << fans.GetFanRPM(i) << "/" << fans.GetMaxRPM(i)
					  << " Boost " << fans.GetFanBoost(i)
					  << " Temperature: " << t
					  << std::endl;
		}
		fans.SetGMode(temp > 70);
		Sleep(10000);
	}

//	std::cout << "[ .... ] Press enter to power down fans" << std::endl;
//	std::cin.ignore();

	return 0;
}