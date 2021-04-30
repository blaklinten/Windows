// Comple: cl remap-capslock.cpp
#include <iostream>
#include <signal.h>
#include <windows.h>

// Tell the VS linker to add the user32 lib
#pragma comment(lib, "user32")

static HHOOK hook;

static const WORD fromKey = VK_CAPITAL;
static const WORD toKey = VK_ESCAPE;

LRESULT CALLBACK keyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
  KBDLLHOOKSTRUCT *const keyEvent = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

  switch (keyEvent->vkCode) {
  case fromKey: {
    // Send a new keypress
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms646271(v=vs.85).aspx
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.dwExtraInfo = 0;
    input.ki.time = 0;
    input.ki.wScan = 0;
    input.ki.wVk = toKey;
    input.ki.dwFlags = wParam == WM_KEYUP ? KEYEVENTF_KEYUP : 0;

    if (SendInput(1, &input, sizeof(input)) == 0) {
      std::cerr << "Failed to insert keypress: " << GetLastError() << std::endl;
    }

    // swallow the remapped key key
    return 1;
  }

  default:
    return CallNextHookEx(hook, nCode, wParam, lParam);
  }
}

void onSignal(int signal) {
  if (signal == SIGINT) {
    exit(0);
  }
}

int main() {
  hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHook, NULL, 0);
  if (!hook) {
    std::cerr << "Failed to set keyboard hook: " << GetLastError() << std::endl;
    return EXIT_FAILURE;
  }

  signal(SIGINT, onSignal);

  // Sleep forever
  // (this is a console application with no messages)
  // Sleep(INFINITE) doesn't work as the hooks are never triggered.
  MSG msg;
  GetMessage(&msg, NULL, 0, 0);
}