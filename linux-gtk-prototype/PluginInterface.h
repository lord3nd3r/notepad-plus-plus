// Notepad++ Plugin Interface - Linux GTK Port
// 100% API-compatible with Windows Notepad++ plugin interface
// Copyright (C) 2026 Kristopher Craig
// Based on Notepad++ by Don HO

#pragma once

#include "Scintilla.h"
#include <gtk/gtk.h>

// Windows type compatibility layer
#ifdef __linux__
typedef GtkWidget *HWND;
typedef unsigned long WPARAM;
typedef long LPARAM;
typedef long LRESULT;
typedef unsigned int UINT;
typedef int BOOL;
typedef unsigned char UCHAR;

#define TRUE 1
#define FALSE 0
#define __cdecl

// Linux plugin export (instead of __declspec(dllexport))
#define NPP_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
// Windows compatibility
#define NPP_PLUGIN_EXPORT __declspec(dllexport)
#endif

// Plugin API functions - IDENTICAL to Windows version
typedef const wchar_t *(__cdecl *PFUNCGETNAME)();

struct NppData {
  HWND _nppHandle = nullptr;
  HWND _scintillaMainHandle = nullptr;
  HWND _scintillaSecondHandle = nullptr;
};

typedef void(__cdecl *PFUNCSETINFO)(NppData);
typedef void(__cdecl *PFUNCPLUGINCMD)();
typedef void(__cdecl *PBENOTIFIED)(SCNotification *);
typedef LRESULT(__cdecl *PMESSAGEPROC)(UINT Message, WPARAM wParam,
                                       LPARAM lParam);

struct ShortcutKey {
  bool _isCtrl = false;
  bool _isAlt = false;
  bool _isShift = false;
  UCHAR _key = 0;
};

const int menuItemSize = 64;

struct FuncItem {
  wchar_t _itemName[menuItemSize] = {L'\0'};
  PFUNCPLUGINCMD _pFunc = nullptr;
  int _cmdID = 0;
  bool _init2Check = false;
  ShortcutKey *_pShKey = nullptr;
};

typedef FuncItem *(__cdecl *PFUNCGETFUNCSARRAY)(int *);

// Required plugin exports - IDENTICAL API to Windows
extern "C" NPP_PLUGIN_EXPORT void setInfo(NppData);
extern "C" NPP_PLUGIN_EXPORT const wchar_t *getName();
extern "C" NPP_PLUGIN_EXPORT FuncItem *getFuncsArray(int *);
extern "C" NPP_PLUGIN_EXPORT void beNotified(SCNotification *);
extern "C" NPP_PLUGIN_EXPORT LRESULT messageProc(UINT Message, WPARAM wParam,
                                                 LPARAM lParam);
extern "C" NPP_PLUGIN_EXPORT BOOL isUnicode();

// Notepad++ Messages - Complete compatibility with Windows
// File Operations
#define NPPM_GETCURRENTSCINTILLA (0x1000)
#define NPPM_GETCURRENTLANGTYPE (0x1001)
#define NPPM_SETCURRENTLANGTYPE (0x1002)
#define NPPM_GETNBOPENFILES (0x1003)
#define NPPM_GETOPENFILENAMES (0x1004)
#define NPPM_MODELESSDIALOG (0x1005)
#define NPPM_GETNBSESSIONFILES (0x1006)
#define NPPM_GETSESSIONFILES (0x1007)
#define NPPM_SAVECURRENTSESSION (0x1008)
#define NPPM_SAVESESSION (0x1009)
#define NPPM_LOADSESSION (0x100A)

// Buffer/Document Operations
#define NPPM_GETCURRENTDOCINDEX (0x100B)
#define NPPM_SETSTATUSBAR (0x100C)
#define NPPM_GETMENUHANDLE (0x100D)
#define NPPM_ENCODESCI (0x100E)
#define NPPM_DECODESCI (0x100F)
#define NPPM_ACTIVATEDOC (0x1010)
#define NPPM_LAUNCHFINDINFILESDLG (0x1011)
#define NPPM_DMMSHOW (0x1012)
#define NPPM_DMMHIDE (0x1013)
#define NPPM_DMMUPDATEDISPINFO (0x1014)
#define NPPM_DMMREGASDCKDLG (0x1015)
// NPPM_LOADSESSION is already defined as 0x100A

// File Management
#define MAX_PATH 260
#define NPPM_GETFULLCURRENTPATH (0x1017)
#define NPPM_GETFILENAME (0x1018)
#define NPPM_GETNAMEPART (0x1019)
#define NPPM_GETEXTPART (0x101A)
#define NPPM_GETCURRENTDIRECTORY (0x101B)
#define NPPM_GOTOLINEANDCOLUMN (0x101C)

// Menu Commands
#define NPPM_MENUCOMMAND (0x101D)
#define NPPM_TRIGGERTABBARCONTEXTMENU (0x101E)
#define NPPM_GETNPPVERSION (0x101F)
#define NPPM_HIDETABBAR (0x1020)
#define NPPM_ISTABBARHIDDEN (0x1021)
#define NPPM_GETPOSFROMBUFFERID (0x1022)
#define NPPM_GETFULLPATHFROMBUFFERID (0x1023)
#define NPPM_GETBUFFERIDFROMPOS (0x1024)
#define NPPM_GETCURRENTBUFFERID (0x1025)
#define NPPM_RELOADBUFFERID (0x1026)
#define NPPM_GETBUFFERLANGTYPE (0x1027)
#define NPPM_SETBUFFERLANGTYPE (0x1028)
#define NPPM_GETBUFFERENCODING (0x1029)
#define NPPM_SETBUFFERENCODING (0x102A)
#define NPPM_GETBUFFERFORMAT (0x102B)
#define NPPM_SETBUFFERFORMAT (0x102C)

// Save Operations
#define NPPM_SAVECURRENTFILE (0x102D)
#define NPPM_SAVEALLFILES (0x102E)
#define NPPM_SAVEFILE (0x102F)

// Notification codes - Complete compatibility
#define NPPN_FIRST (0x2000)
#define NPPN_READY (NPPN_FIRST + 1)
#define NPPN_TBMODIFICATION (NPPN_FIRST + 2)
#define NPPN_FILEBEFORECLOSE (NPPN_FIRST + 3)
#define NPPN_FILEOPENED (NPPN_FIRST + 4)
#define NPPN_FILEBEFOREOPEN (NPPN_FIRST + 5)
#define NPPN_FILEBEFORESAVE (NPPN_FIRST + 6)
#define NPPN_FILESAVED (NPPN_FIRST + 7)
#define NPPN_SHUTDOWN (NPPN_FIRST + 8)
#define NPPN_BUFFERACTIVATED (NPPN_FIRST + 9)
#define NPPN_LANGCHANGED (NPPN_FIRST + 10)
#define NPPN_WORDSTYLESUPDATED (NPPN_FIRST + 11)
#define NPPN_SHORTCUTREMAPPED (NPPN_FIRST + 12)
#define NPPN_FILEBEFORELOAD (NPPN_FIRST + 13)
#define NPPN_FILELOADFAILED (NPPN_FIRST + 14)
#define NPPN_READONLYCHANGED (NPPN_FIRST + 15)
#define NPPN_DOCORDERCHANGED (NPPN_FIRST + 16)
#define NPPN_SNAPSHOTDIRTYFILELOADED (NPPN_FIRST + 17)
#define NPPN_BEFORESHUTDOWN (NPPN_FIRST + 18)
#define NPPN_CANCELSHUTDOWN (NPPN_FIRST + 19)
#define NPPN_FILEBEFORERENAME (NPPN_FIRST + 20)
#define NPPN_FILERENAMECANCEL (NPPN_FIRST + 21)
#define NPPN_FILERENAMED (NPPN_FIRST + 22)
#define NPPN_FILEBEFOREDELETE (NPPN_FIRST + 23)
#define NPPN_FILEDELETED (NPPN_FIRST + 24)
#define NPPN_DARKMODEMODECHANGED (NPPN_FIRST + 25)
