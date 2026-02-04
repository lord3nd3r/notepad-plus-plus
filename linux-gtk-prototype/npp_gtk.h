// Notepad++ GTK Port - Main Header
#pragma once

#include "ILexer.h"
#include "Lexilla.h"
#include "Scintilla.h"
#include "ScintillaWidget.h"
#include <ctime>
#include <gtk/gtk.h>
#include <memory>
#include <string>
#include <vector>

// Menu command IDs (simplified from menuCmdID.h)
#define IDM_FILE_NEW 1001
#define IDM_FILE_OPEN 1002
#define IDM_FILE_SAVE 1003
#define IDM_FILE_SAVEAS 1004
#define IDM_FILE_SAVEALL 1005
#define IDM_FILE_CLOSE 1006
#define IDM_FILE_CLOSEALL 1007
#define IDM_FILE_PRINT 1008
#define IDM_FILE_EXIT 1009

#define IDM_EDIT_UNDO 2001
#define IDM_EDIT_REDO 2002
#define IDM_EDIT_CUT 2003
#define IDM_EDIT_COPY 2004
#define IDM_EDIT_PASTE 2005
#define IDM_EDIT_DELETE 2006
#define IDM_EDIT_SELECTALL 2007

#define IDM_SEARCH_FIND 3001
#define IDM_SEARCH_FINDNEXT 3002
#define IDM_SEARCH_REPLACE 3003
#define IDM_SEARCH_GOTOLINE 3004
#define IDM_SEARCH_FINDINFILES 3005

#define IDM_VIEW_FULLSCREEN 4001
#define IDM_VIEW_ALWAYSONTOP 4002
#define IDM_VIEW_WORDWRAP 4003
#define IDM_VIEW_ZOOMIN 4004
#define IDM_VIEW_ZOOMOUT 4005
#define IDM_VIEW_ZOOMRESTORE 4006

#define IDM_RUN_RUN 5001
#define IDM_RUN_GOOGLE_SEARCH 5002
#define IDM_RUN_OPEN_DEFAULT 5003
#define IDM_RUN_OPEN_CHROME 5004
#define IDM_RUN_OPEN_FIREFOX 5005

struct TabData {
  GtkWidget *sci;
  std::string filename;
  bool modified;
  int encoding;
  int eolFormat; // 0=Windows, 1=Unix, 2=Mac
  std::string language;
  time_t fileModTime; // Last modification time of the file on disk
};

struct AppState {
  GtkWidget *window;
  GtkWidget *notebook;
  GtkWidget *statusbar;
  GtkWidget *toolbar;
  GtkWidget *menubar;
  guint status_context;
  std::vector<std::string> recent_files;
  bool word_wrap;
  bool alwaysOnTop;
  guint file_watch_timer_id;
  GtkAccelGroup *accel_group;
  GtkWidget *recent_menu;
  bool show_whitespace;
  bool show_eol;
  bool show_line_numbers;
  bool is_split;
  GtkWidget *notebook2;
  GtkWidget *paned;
  bool is_horizontal_split;
  bool is_fullscreen;
  bool is_distraction_free;
  std::string last_search;
  bool find_case_sensitive;
  int last_find_pos;
  bool is_recording_macro;
  std::vector<std::string> current_macro;
  std::vector<std::vector<std::string>> saved_macros;
  GtkWidget *incremental_search_bar;
  GtkWidget *incremental_search_entry;
  bool incremental_search_active;
  guint auto_save_timer_id;
};

// Function declarations
void init_app_state(AppState *app);
GtkWidget *create_main_window();
GtkWidget *create_menu_bar(AppState *app);
GtkWidget *create_toolbar(AppState *app);
GtkWidget *create_tab(AppState *app, const std::string &filename);
TabData *get_current_tabdata(AppState *app);
void update_statusbar_for_scintilla(GtkWidget *sci, AppState *app);
void apply_lexer_for_file(GtkWidget *sci, const std::string &filename);

// Dialog functions
void show_find_dialog(AppState *app);
void show_replace_dialog(AppState *app);
void show_goto_line_dialog(AppState *app);
void show_about_dialog(AppState *app);

// Command handlers
void cmd_file_new(AppState *app);
void cmd_file_open(AppState *app);
void cmd_file_save(AppState *app);
void cmd_file_saveas(AppState *app);
void cmd_file_saveall(AppState *app);
void cmd_file_close(AppState *app);
void cmd_file_exit(AppState *app);

void cmd_edit_undo(AppState *app);
void cmd_edit_redo(AppState *app);
void cmd_edit_cut(AppState *app);
void cmd_edit_copy(AppState *app);
void cmd_edit_paste(AppState *app);
void cmd_edit_delete(AppState *app);
void cmd_edit_selectall(AppState *app);

void cmd_search_find(AppState *app);
void cmd_search_replace(AppState *app);
void cmd_search_gotoline(AppState *app);

void cmd_view_wordwrap(AppState *app);
void cmd_view_zoomin(AppState *app);

// File watching functions
time_t get_file_modification_time(const std::string &filename);
void check_file_changes(AppState *app);
gboolean file_watch_timer(gpointer data);
void cmd_view_zoomout(AppState *app);
