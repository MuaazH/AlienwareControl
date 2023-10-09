#include <Windows.h>
#include "fans.h"

using namespace alienware;

// variable to store the HANDLE to the hook. Don't declare it anywhere else then globally
// or you will get problems since every function uses this variable.
HHOOK _hook;
FansControl fans;
bool gmodOn = false;

// This is the callback function. Consider it the event that is raised when, in this case,
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0)
	{
		// the action is valid: HC_ACTION.
		if (wParam == WM_KEYDOWN)
		{
			KBDLLHOOKSTRUCT kbdStruct;
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

			if (kbdStruct.scanCode == 104 && kbdStruct.vkCode == 128) {
				// Toggle G-MODE
				gmodOn = !gmodOn;
				fans.SetGMode(gmodOn);
			}
		}
	}

	// call the next hook in the hook chain.
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook() {
	// Set the hook and set it to use the callback function above
	// WH_KEYBOARD_LL means it will set a low level keyboard hook. More information about it at MSDN.
	// The last 2 parameters are NULL, 0 because the callback function is in the same thread and window as the
	// function that sets and releases the hook.
	if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
		const char *a = "Failed to install hook!";
		const char *b = "Error";
		MessageBox(NULL, a, b, MB_ICONERROR);
	}
}

//void ReleaseHook() {
//	UnhookWindowsHookEx(_hook);
//}

int main() {
	if (!fans.Init()) {
		return 0;
	}

	// set the hook
	SetHook();

	// loop to keep the console application running.
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
	}
}
