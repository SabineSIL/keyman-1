/*
  Name:             k32_lowlevelkeyboardhook
  Copyright:        Copyright (C) SIL International.
  Documentation:
  Description:
  Create Date:      3 Aug 2014

  Modified Date:    25 Oct 2016
  Authors:          mcdurdin
  Related Files:
  Dependencies:

  Bugs:
  Todo:
  Notes:
  History:          03 Aug 2014 - mcdurdin - I4326 - V9.0 - Switch-off hotkey not working, then keyboard hotkey stopped working (win 8.1 jeremy) [High]
                    13 Oct 2014 - mcdurdin - I4451 - V9.0 - Language hotkeys are not working
                    27 Mar 2015 - mcdurdin - I4641 - V9.0 - Keyman can crash silently on exit due to null hotkeys being addressed
                    22 Apr 2015 - mcdurdin - I4674 - V9.0 - Hotkeys do not always work consistently
                    14 May 2015 - mcdurdin - I4714 - V9.0 - Keyboard and language hotkeys don't always work
                    09 Aug 2015 - mcdurdin - I4844 - Tidy up PostDummyKeyEvent calls
                    25 Oct 2016 - mcdurdin - I5136 - Remove additional product references from Keyman Engine
*/
// I4326
#include "pch.h"
#include "serialkeyeventserver.h"
#include "kbd.h"	/* DDK kbdlayout */

#ifdef _WIN64
#error k32_lowlevelkeyboardhook.cpp is not required for win64.
#endif

BOOL ProcessHotkey(UINT vkCode, BOOL isUp, DWORD ShiftState);

LRESULT _kmnLowLevelKeyboardProc(
  _In_  int nCode,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
);

// Local variables only used by single thread -- low level keyboard proc thread
DWORD FHotkeyShiftState = 0;

LRESULT CALLBACK kmnLowLevelKeyboardProc(
  _In_  int nCode,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
) {
  LRESULT res = 0;
#ifdef _DEBUG_EXCEPTION
	res = _kmnLowLevelKeyboardProc(nCode,wParam,lParam);
#else
  __try {
	  res = _kmnLowLevelKeyboardProc(nCode,wParam,lParam);
	}
  __except(ExceptionMessage("kmnLowLevelKeyboardProc", GetExceptionInformation())) {
	}
#endif
  return res;
}

BOOL KeyLanguageSwitchPress(WPARAM wParam, BOOL extended, BOOL isUp, DWORD ShiftState);
int ProcessLanguageSwitchShiftKey(WPARAM wParam, BOOL isUp);
BOOL IsLanguageSwitchWindowVisible();
void SendToLanguageSwitchWindow(WPARAM wParam, LPARAM lParam);

LPARAM LLKHFFlagstoWMKeymanKeyEventFlags(PKBDLLHOOKSTRUCT hs) {
  return (hs->scanCode << 16) |
    ((hs->flags & LLKHF_EXTENDED) ? KEYEVENTF_EXTENDEDKEY : 0) |
    ((hs->flags & LLKHF_UP) ? KEYEVENTF_KEYUP : 0);
}

/*
  We don't attempt to serialize input to the console windows because they
  behave somewhat differently to normal windows. For now, this should be
  sufficient. In the future, we may want to find a way to interrogate the
  focused process to find out which window actually has focus for posting
  messages, because we appear to post the messages to the wrong thread
  for console windows.
*/
BOOL IsConsoleWindow(HWND hwnd) {
  static HWND last_hwnd = 0;
  static BOOL last_isConsoleWindow = FALSE;

  if (last_hwnd == hwnd) {
    return last_isConsoleWindow;
  }

  char buf[64];

  last_hwnd = hwnd;
  last_isConsoleWindow = GetClassName(hwnd, buf, 64) && !strcmp(buf, "ConsoleWindowClass");

  return last_isConsoleWindow;
}


/*
 Test for touch panel visibility (#2450). UpdateTouchPanelVisibility is called periodically by
 keyman.exe to refresh the visibility flag.
*/

static BOOL touchPanelVisible;

void WINAPI Keyman_UpdateTouchPanelVisibility(BOOL isVisible) {
  touchPanelVisible = isVisible;
  SendDebugMessageFormat(0, sdmAIDefault, 0, "Keyman_UpdateTouchPanelVisibility: isVisible=%d", touchPanelVisible);
}

BOOL IsTouchPanelVisible() {
  // Note: GetCurrentInputMessageSource does not work in this context
  // Using IFrameworkInputPaneHandler events only works for a specific window, so not helpful for us.
  return touchPanelVisible;
}

/*
  Cache UseRegisterHotkey debug flag for this session
*/
BOOL UseRegisterHotkey() {
  static BOOL flag_UseRegisterHotkey = FALSE;
  static BOOL loaded = FALSE;
  if (!loaded) {
    loaded = TRUE;
    flag_UseRegisterHotkey = Reg_GetDebugFlag(REGSZ_Flag_UseRegisterHotkey, FALSE);
  }
  return flag_UseRegisterHotkey;
}

BOOL UseCachedHotkeyModifierState() {
  static BOOL flag_UseCachedHotkeyModifierState = FALSE;
  static BOOL loaded = FALSE;
  if (!loaded) {
    loaded = TRUE;
    flag_UseCachedHotkeyModifierState = Reg_GetDebugFlag(REGSZ_Flag_UseCachedHotkeyModifierState, FALSE);
  }
  return flag_UseCachedHotkeyModifierState;
}

LRESULT _kmnLowLevelKeyboardProc(
  _In_  int nCode,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
) {

  if(nCode < 0) {
    return CallNextHookEx(Globals::get_hhookLowLevelKeyboardProc(), nCode, wParam, lParam);
  }


  PKBDLLHOOKSTRUCT hs = (PKBDLLHOOKSTRUCT) lParam;

  BOOL extended = hs->flags & LLKHF_EXTENDED ? TRUE : FALSE;
  BOOL isUp = hs->flags & LLKHF_UP ? TRUE : FALSE;

  SendDebugMessageFormat(0, sdmAIDefault, 0, "kmnLowLevelKeyboardProc: wparam: %x  lparam: %x [vk:%s scan:%x flags:%x extra:%x]", wParam, lParam, Debug_VirtualKey((WORD) hs->vkCode), hs->scanCode, hs->flags, hs->dwExtraInfo);   // I4674

  DWORD Flag = 0;
  if (!UseCachedHotkeyModifierState()) {
    // #5190: Don't cache modifier state because sometimes we won't receive
    // modifier change events (e.g. on lock screen)
    FHotkeyShiftState = 0;
    if (GetKeyState(VK_LCONTROL) < 0) FHotkeyShiftState |= HK_CTRL;
    if (GetKeyState(VK_RCONTROL) < 0) FHotkeyShiftState |= HK_RCTRL_INVALID;
    if (GetKeyState(VK_LMENU) < 0) FHotkeyShiftState |= HK_CTRL;
    if (GetKeyState(VK_RMENU) < 0) FHotkeyShiftState |= HK_RALT_INVALID;
    if (GetKeyState(VK_LSHIFT) < 0) FHotkeyShiftState |= HK_SHIFT;
    if (GetKeyState(VK_RSHIFT) < 0) FHotkeyShiftState |= HK_RSHIFT_INVALID;
  }
  else if (UseRegisterHotkey()) {
    // The old RegisterHotkey pattern does not support chiral modifier keys
    switch (hs->vkCode) {
      case VK_LCONTROL:
      case VK_RCONTROL:
      case VK_CONTROL:  Flag = HK_CTRL; break;
      case VK_LMENU:
      case VK_RMENU:
      case VK_MENU:     Flag = HK_ALT; break;
      case VK_LSHIFT:
      case VK_RSHIFT:
      case VK_SHIFT:    Flag = HK_SHIFT; break;
    }
  }
  else {
    // #4619: We differentiate between Left and Right Ctrl/Shift/Alt. The right modifiers are
    // not used for hotkeys, leaving them available for use as keystroke modifiers within a
    // Keyman keyboard. But we need to track them anyway, so that a user doesn't press them
    // and receive a hotkey event when they shouldn't (e.g. RALT+F4 if the hotkey is F4).
    switch (hs->vkCode) {
      case VK_LCONTROL: Flag = HK_CTRL; break;
      case VK_RCONTROL: Flag = HK_RCTRL_INVALID; break;
      case VK_CONTROL:  Flag = extended ? HK_RCTRL_INVALID : HK_CTRL; break;
      case VK_LMENU:    Flag = HK_ALT; break;
      case VK_RMENU:    Flag = HK_RALT_INVALID; break;
      case VK_MENU:     Flag = extended ? HK_RALT_INVALID : HK_ALT; break;
      case VK_LSHIFT:   Flag = HK_SHIFT; break;
      case VK_RSHIFT:   Flag = HK_RSHIFT_INVALID; break;
      case VK_SHIFT:    Flag = hs->scanCode == SCANCODE_RSHIFT ? HK_RSHIFT_INVALID : HK_SHIFT; break;
    }
  }

  if(Flag != 0) {
    if(isUp) FHotkeyShiftState &= ~Flag;
    else FHotkeyShiftState |= Flag;
  }

  if(IsLanguageSwitchWindowVisible()) {
    SendDebugMessageFormat(0, sdmAIDefault, 0, "kmnLowLevelKeyboardProc: Sending to language switch window %x %x", wParam, lParam);
    SendToLanguageSwitchWindow(hs->vkCode, hs->flags);
    if (ProcessLanguageSwitchShiftKey(hs->vkCode, isUp) == 1) return 1;
  }
  else if (KeyLanguageSwitchPress(hs->vkCode, extended, isUp, FHotkeyShiftState)) {
    if (ProcessLanguageSwitchShiftKey(hs->vkCode, isUp) == 1) return 1;
  }

  if (ProcessHotkey(hs->vkCode, isUp, FHotkeyShiftState)) {
    return 1;
  }

  /*

    Not a registered hotkey, so we will use the serialized input model

  */

  if (hs->dwExtraInfo != 0 || hs->scanCode == SCAN_FLAG_KEYMAN_KEY_EVENT || hs->vkCode == VK_PROCESSKEY || hs->vkCode == VK_PACKET) {
    // This key event was generated by Keyman, so pass it through
    // dwExtraInfo is set to 0x4321DCBA by mstsc which does prefiltering. So we ignore for anything where dwExtraInfo!=0 because it
    // probably is not hardware generated and may cause more issues to filter it
    return CallNextHookEx(Globals::get_hhookLowLevelKeyboardProc(), nCode, wParam, lParam);
  }

  if (IsTouchPanelVisible()) {
    // See #2450. The touch panel will close automatically if we reprocess key events
    // So we don't want to reprocess events when it is visible.
    //SendDebugMessageFormat(0, sdmAIDefault, 0, "kmnLowLevelKeyboardProc: touch panel is visible. Not reprocessing keystrokes");
    return CallNextHookEx(Globals::get_hhookLowLevelKeyboardProc(), nCode, wParam, lParam);
  }

  if (flag_ShouldSerializeInput) {
    GUITHREADINFO gui = { 0 };
    gui.cbSize = sizeof(GUITHREADINFO);
    if (GetGUIThreadInfo(NULL, &gui)) {
      SendDebugMessageFormat(0, sdmGlobal, 0, "LowLevelHook: Active=%x Focus=%x Key=%s flags=%x",
        gui.hwndActive, gui.hwndFocus, Debug_VirtualKey((WORD)hs->vkCode), LLKHFFlagstoWMKeymanKeyEventFlags(hs));

      HWND hwnd = gui.hwndFocus ? gui.hwndFocus : gui.hwndActive;
      if (!IsConsoleWindow(hwnd)) {
        PostMessage(ISerialKeyEventServer::GetServer()->GetWindow(), wm_keyman_keyevent, hs->vkCode, LLKHFFlagstoWMKeymanKeyEventFlags(hs));
        return 1;
      }
      //else SendDebugMessageFormat(0, sdmGlobal, 0, "LowLevelHook: console window, not serializing"); // too noisy
    }
    else {
      SendDebugMessageFormat(0, sdmGlobal, 0, "LowLevelHook: Failed to get Gui thread info with error %d", GetLastError());
    }
  }

  return CallNextHookEx(Globals::get_hhookLowLevelKeyboardProc(), nCode, wParam, lParam);
}

BOOL ProcessHotkey(UINT vkCode, BOOL isUp, DWORD ShiftState) {
  if (UseRegisterHotkey()) {
    return FALSE;
  }

  Hotkeys *hotkeys = Hotkeys::Instance();   // I4641
  if (!hotkeys) {
    return FALSE;
  }

  Hotkey *hotkey = hotkeys->GetHotkey(ShiftState | vkCode);   // I4641
  if (!hotkey) {
    return FALSE;
  }

  if (isUp) {
    return TRUE;
  }

  if (hotkey->HotkeyType == hktInterface) {
    Globals::PostMasterController(wm_keyman_control, MAKELONG(KMC_INTERFACEHOTKEY, hotkey->Target), 0);
  }
  else {
    ReportKeyboardChanged(PC_HOTKEYCHANGE, hotkey->hkl == 0 ? TF_PROFILETYPE_INPUTPROCESSOR : TF_PROFILETYPE_KEYBOARDLAYOUT, 0, hotkey->hkl, GUID_NULL, hotkey->profileGUID);
  }

  /* Generate a dummy keystroke to block menu activations, etc but let the shift key through */
  PostDummyKeyEvent();  // I3301 - this is imperfect because we don't deal with HC_NOREMOVE.  But good enough?   // I3534   // I4844

  return TRUE;
}
