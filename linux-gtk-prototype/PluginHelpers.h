// Notepad++ Plugin Helpers - Cross-platform utilities
// Copyright (C) 2026 Kristopher Craig

#pragma once

#include "PluginInterface.h"
#include <codecvt>
#include <cstring>
#include <locale>
#include <string>

// Cross-platform string conversion utilities
namespace NppPlugin {

#ifdef __linux__
// Wide string to UTF-8 conversion
inline std::string wstring_to_utf8(const std::wstring &wstr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(wstr);
}

// UTF-8 to wide string conversion
inline std::wstring utf8_to_wstring(const std::string &str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.from_bytes(str);
}

// Helper to convert wchar_t* to const char* for GTK
inline const char *wchar_to_utf8_temp(const wchar_t *wstr) {
  static thread_local std::string temp;
  temp = wstring_to_utf8(wstr);
  return temp.c_str();
}

// SendMessage wrapper for Linux
inline LRESULT SendNppMessage(HWND hwnd, UINT msg, WPARAM wParam = 0,
                              LPARAM lParam = 0) {
  // This will be implemented by the plugin manager
  // Plugins call this instead of Windows SendMessage
  extern LRESULT NppPluginSendMessage(HWND hwnd, UINT msg, WPARAM wParam,
                                      LPARAM lParam);
  return NppPluginSendMessage(hwnd, msg, wParam, lParam);
}

// MessageBox wrapper for Linux
inline int MessageBox(HWND parent, const wchar_t *text, const wchar_t *title,
                      unsigned int type) {
  GtkWidget *dialog = gtk_message_dialog_new(
      GTK_WINDOW(parent), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
      "%s", wchar_to_utf8_temp(text));
  gtk_window_set_title(GTK_WINDOW(dialog), wchar_to_utf8_temp(title));
  int result = gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  return result;
}

#else
// Windows versions - use native API
inline LRESULT SendNppMessage(HWND hwnd, UINT msg, WPARAM wParam = 0,
                              LPARAM lParam = 0) {
  return ::SendMessage(hwnd, msg, wParam, lParam);
}
#endif

// Cross-platform helper to get current Scintilla handle
inline HWND GetCurrentScintilla(const NppData &nppData) {
  int which = -1;
  SendNppMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0,
                 (LPARAM)&which);
  return (which == 0) ? nppData._scintillaMainHandle
                      : nppData._scintillaSecondHandle;
}

// Helper to send Scintilla message to current view
inline LRESULT SendScintillaMessage(const NppData &nppData, UINT msg,
                                    WPARAM wParam = 0, LPARAM lParam = 0) {
  HWND curScintilla = GetCurrentScintilla(nppData);
#ifdef __linux__
  return scintilla_send_message(SCINTILLA_OBJECT(curScintilla), msg, wParam,
                                lParam);
#else
  return ::SendMessage(curScintilla, msg, wParam, lParam);
#endif
}

// Get current file path
inline std::wstring GetCurrentFilePath(const NppData &nppData) {
  wchar_t path[MAX_PATH] = {0};
  SendNppMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH,
                 (LPARAM)path);
  return path;
}

// Get current file name
inline std::wstring GetCurrentFileName(const NppData &nppData) {
  wchar_t name[MAX_PATH] = {0};
  SendNppMessage(nppData._nppHandle, NPPM_GETFILENAME, MAX_PATH, (LPARAM)name);
  return name;
}

} // namespace NppPlugin
