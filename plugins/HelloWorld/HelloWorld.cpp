// HelloWorld Plugin for Notepad++
// Example plugin demonstrating 100% API compatibility
// Works on both Windows and Linux without code changes!

#include "../linux-gtk-prototype/PluginHelpers.h"
#include "../linux-gtk-prototype/PluginInterface.h"

// Plugin data
const wchar_t PLUGIN_NAME[] = L"Hello World";
static NppData nppData;
static FuncItem funcItem[2]; // 2 menu items

// Function prototypes
void helloWorld();
void aboutPlugin();

// Required plugin exports
extern "C" NPP_PLUGIN_EXPORT void setInfo(NppData notpadPlusData) {
  nppData = notpadPlusData;
}

extern "C" NPP_PLUGIN_EXPORT const wchar_t *getName() { return PLUGIN_NAME; }

extern "C" NPP_PLUGIN_EXPORT FuncItem *getFuncsArray(int *nbF) {
  *nbF = 2; // Number of menu items

  // Menu item 1: Say Hello
  wcscpy(funcItem[0]._itemName, L"Say Hello");
  funcItem[0]._pFunc = helloWorld;
  funcItem[0]._cmdID = 0;
  funcItem[0]._init2Check = false;
  funcItem[0]._pShKey = nullptr;

  // Menu item 2: About
  wcscpy(funcItem[1]._itemName, L"About");
  funcItem[1]._pFunc = aboutPlugin;
  funcItem[1]._cmdID = 1;
  funcItem[1]._init2Check = false;
  funcItem[1]._pShKey = nullptr;

  return funcItem;
}

extern "C" NPP_PLUGIN_EXPORT void beNotified(SCNotification *notifyCode) {
  // Handle notifications from Scintilla/Notepad++
  switch (notifyCode->nmhdr.code) {
  case NPPN_READY:
    // Notepad++ is ready
    break;
  case NPPN_FILESAVED:
    // File was saved
    break;
  case NPPN_SHUTDOWN:
    // Notepad++ is shutting down
    break;
  }
}

extern "C" NPP_PLUGIN_EXPORT LRESULT messageProc(UINT Message, WPARAM wParam,
                                                 LPARAM lParam) {
  return TRUE;
}

extern "C" NPP_PLUGIN_EXPORT BOOL isUnicode() { return TRUE; }

// Plugin functions
void helloWorld() {
  // Get current file name
  std::wstring fileName = NppPlugin::GetCurrentFileName(nppData);

  // Build message
  std::wstring message = L"Hello from Notepad++ plugin!\n\n";
  if (!fileName.empty()) {
    message += L"Current file: " + fileName;
  } else {
    message += L"No file is currently open.";
  }

  // Show message box (works on both Windows and Linux!)
  NppPlugin::MessageBox(nppData._nppHandle, message.c_str(),
                        L"Hello World Plugin", 0);

  // Insert text into current editor
  HWND curScintilla = NppPlugin::GetCurrentScintilla(nppData);
  const char *text = "/* Hello from plugin! */\n";

#ifdef __linux__
  scintilla_send_message(SCINTILLA_OBJECT(curScintilla), SCI_ADDTEXT,
                         strlen(text), (sptr_t)text);
#else
  ::SendMessage(curScintilla, SCI_ADDTEXT, strlen(text), (LPARAM)text);
#endif
}

void aboutPlugin() {
  std::wstring about = L"Hello World Plugin v1.0\n\n";
  about += L"Demonstrates 100% API compatibility\n";
  about += L"between Windows and Linux!\n\n";
  about += L"Works on:\n";
  about += L"• Windows Notepad++\n";
  about += L"• Linux Notepad++ GTK\n\n";
  about += L"Same source code, different platforms!";

  NppPlugin::MessageBox(nppData._nppHandle, about.c_str(), L"About", 0);
}
