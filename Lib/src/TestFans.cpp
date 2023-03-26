
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
	std::cout << "[ .... ] Powering up fans" << std::endl;
	if (fans.SetGMode(true)) {
		std::cout << "[  OK  ] Powering up fans" << std::endl;
		Sleep(5000);
		std::cout << "[ .... ] Powering down fans" << std::endl;
		if (fans.SetGMode(false)) {
			std::cout << "[  OK  ] Powering down fans" << std::endl;
		} else {
			std::cout << "[ FAIL ] Powering down fans" << std::endl;
		}
	} else {
		std::cout << "[ FAIL ] Powering up fans" << std::endl;
	}
	std::cout << "[ .... ] Quiting" << std::endl;
	return 0;
}