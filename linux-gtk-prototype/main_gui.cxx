#include <gtk/gtk.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <regex>
#include <set>
#include <cctype>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include "Scintilla.h"
#include "ScintillaWidget.h"
#include "ILexer.h"
#include "Lexilla.h"
#include "LexillaAccess.h"

using std::string;

struct TabData {
    GtkWidget *sci;
    string filename;
    bool modified;
};

struct AppState {
    GtkWidget *window;
    GtkWidget *notebook;
    GtkWidget *notebook2;  // Second notebook for split view
    GtkWidget *paned;      // Paned widget for split view
    GtkWidget *statusbar;
    guint status_context;
    GtkAccelGroup *accel_group;
    GtkWidget *find_entry;
    GtkWidget *replace_entry;
    string last_search;
    bool word_wrap = false;
    bool show_whitespace = false;
    bool show_eol = false;
    bool show_line_numbers = true;
    bool find_case_sensitive = false;
    int last_find_pos = -1;
    std::vector<string> recent_files;
    GtkWidget *recent_menu = nullptr;
    bool is_split = false;
    bool is_horizontal_split = true;
    bool is_fullscreen = false;
    bool is_distraction_free = false;
    GtkWidget *menubar = nullptr;
    GtkWidget *toolbar = nullptr;
    bool is_recording_macro = false;
    std::vector<string> current_macro;
    std::vector<std::vector<string>> saved_macros;
    GtkWidget *incremental_search_bar = nullptr;
    GtkWidget *incremental_search_entry = nullptr;
    bool incremental_search_active = false;
    guint auto_save_timer_id = 0;
};

struct Preferences {
    int tab_width = 4;
    int font_size = 10;
    bool use_tabs = false;  // false = use spaces
    bool auto_indent = true;
    bool show_indent_guides = false;
    bool highlight_current_line = true;
    string font_name = "Monospace";
    int edge_column = 80;
    bool show_edge_line = false;
    bool auto_save_enabled = true;
    int auto_save_interval = 300;  // seconds (5 minutes default)
    
    void load();
    void save();
    void apply_to_scintilla(GtkWidget *sci);
};

// Forward declarations
static void record_macro_action(AppState *app, const string &action);
static void update_recent_menu(AppState *app);
static void add_recent_file(AppState *app, const string &filename);
static void session_save(AppState *app);
static void session_restore(AppState *app);

static void update_statusbar(AppState *app, GtkWidget *sci) {
    int len = scintilla_send_message((ScintillaObject*)sci, SCI_GETTEXTLENGTH, 0, 0);
    int pos = scintilla_send_message((ScintillaObject*)sci, SCI_GETCURRENTPOS, 0, 0);
    int line = scintilla_send_message((ScintillaObject*)sci, SCI_LINEFROMPOSITION, pos, 0) + 1;
    int lineCount = scintilla_send_message((ScintillaObject*)sci, SCI_GETLINECOUNT, 0, 0);
    int col = pos - scintilla_send_message((ScintillaObject*)sci, SCI_POSITIONFROMLINE, line-1, 0) + 1;
    char buf[512];
    snprintf(buf, sizeof(buf), "length: %d   lines: %d          Ln: %d   Col: %d          Dos\\\\Windows          UTF-8          INS", 
             len, lineCount, line, col);
    gtk_statusbar_pop(GTK_STATUSBAR(app->statusbar), app->status_context);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, buf);
}

static TabData* get_current_tabdata(AppState *app) {
    gint page = gtk_notebook_get_current_page(GTK_NOTEBOOK(app->notebook));
    if (page < 0) return nullptr;
    GtkWidget *tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), page);
    return (TabData*)g_object_get_data(G_OBJECT(tab), "tabdata");
}

static void sci_notify_cb(GtkWidget *widget, gint code, gpointer notification, gpointer userdata) {
    AppState *app = (AppState*)userdata;
    TabData *td = get_current_tabdata(app);
    if (!td) return;

    if (code == SCN_UPDATEUI) {
        update_statusbar(app, td->sci);
    } else if (code == SCN_MODIFIED) {
        td->modified = true;
        gint page = gtk_notebook_get_current_page(GTK_NOTEBOOK(app->notebook));
        GtkWidget *tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), page);
        GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
        char title[512];
        const char *name = td->filename.empty() ? "Untitled" : td->filename.c_str();
        if (td->modified)
            snprintf(title, sizeof(title), "* %s", name);
        else
            snprintf(title, sizeof(title), "%s", name);
        gtk_label_set_text(GTK_LABEL(label), title);
    }
}

static void apply_lexer(GtkWidget *sci, const string &filename) {
    if (filename.empty()) return;
    std::string::size_type pos = filename.rfind('.');
    if (pos == std::string::npos) return;
    
    std::string ext = filename.substr(pos+1);
    for (auto &c : ext) c = tolower(c);
    std::string lang;
    
    if (ext == "c" || ext == "h") lang = "cpp";
    else if (ext == "cpp" || ext == "cc" || ext == "cxx" || ext == "hpp") lang = "cpp";
    else if (ext == "py") lang = "python";
    else if (ext == "js") lang = "javascript";
    else if (ext == "json") lang = "json";
    else if (ext == "html" || ext == "htm") lang = "html";
    else if (ext == "xml") lang = "xml";
    else if (ext == "java") lang = "java";
    else if (ext == "rs") lang = "rust";
    else if (ext == "go") lang = "go";
    else if (ext == "sh" || ext == "bash") lang = "bash";
    else if (ext == "lua") lang = "lua";
    else if (ext == "tex") lang = "tex";
    else if (ext == "md" || ext == "markdown") lang = "markdown";
    else if (ext == "cs") lang = "csharp";
    else if (ext == "rb") lang = "ruby";
    else if (ext == "php") lang = "php";
    else if (ext == "pl" || ext == "pm") lang = "perl";
    else if (ext == "sql") lang = "sql";
    else if (ext == "css") lang = "css";
    else if (ext == "scss" || ext == "sass") lang = "css";
    else if (ext == "yml" || ext == "yaml") lang = "yaml";
    else if (ext == "toml") lang = "toml";
    else if (ext == "ini" || ext == "cfg" || ext == "conf") lang = "props";
    else lang = ext;

    if (!lang.empty()) {
        Scintilla::ILexer5 *pLexer = CreateLexer(lang.c_str());
        if (pLexer) {
            scintilla_send_message((ScintillaObject*)sci, SCI_SETILEXER, 0, (sptr_t)pLexer);
        }
    }
}

static GtkWidget* create_scintilla_widget(AppState *app) {
    GtkWidget *sci = scintilla_object_new();
    gtk_widget_set_hexpand(sci, TRUE);
    gtk_widget_set_vexpand(sci, TRUE);
    scintilla_send_message((ScintillaObject*)sci, SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t)"Monospace");
    scintilla_send_message((ScintillaObject*)sci, SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINWIDTHN, 0, 40);
    
    // Setup bookmark margin (margin 1)
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINWIDTHN, 1, 16);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINMASKN, 1, 1 << 1);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERDEFINE, 1, SC_MARK_CIRCLE);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERSETFORE, 1, 0x0000FF); // Red bookmark
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERSETBACK, 1, 0x0000FF);
    
    // Setup folding margin (margin 2)
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINTYPEN, 2, SC_MARGIN_SYMBOL);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINWIDTHN, 2, 16);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINSENSITIVEN, 2, 1);
    
    // Define fold markers
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
    
    // Set fold marker colors
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, 0xFFFFFF);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, 0x808080);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, 0xFFFFFF);
    scintilla_send_message((ScintillaObject*)sci, SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, 0x808080);
    
    // Enable automatic folding
    scintilla_send_message((ScintillaObject*)sci, SCI_SETPROPERTY, (uptr_t)"fold", (sptr_t)"1");
    scintilla_send_message((ScintillaObject*)sci, SCI_SETPROPERTY, (uptr_t)"fold.compact", (sptr_t)"0");
    scintilla_send_message((ScintillaObject*)sci, SCI_SETPROPERTY, (uptr_t)"fold.comment", (sptr_t)"1");
    scintilla_send_message((ScintillaObject*)sci, SCI_SETPROPERTY, (uptr_t)"fold.preprocessor", (sptr_t)"1");
    scintilla_send_message((ScintillaObject*)sci, SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CLICK, 0);
    
    // Enable rectangular selection with Alt modifier
    scintilla_send_message((ScintillaObject*)sci, SCI_SETRECTANGULARSELECTIONMODIFIER, SCMOD_ALT, 0);
    
    // Enable multiple selection and multi-paste (for column mode)
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMULTIPLESELECTION, 1, 0);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETADDITIONALSELECTIONTYPING, 1, 0);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMULTIPASTE, 1, 0);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETVIRTUALSPACEOPTIONS, SCVS_RECTANGULARSELECTION, 0);
    
    scintilla_send_message((ScintillaObject*)sci, SCI_SETTABWIDTH, 4, 0);
    g_signal_connect(sci, SCINTILLA_NOTIFY, G_CALLBACK(sci_notify_cb), app);
    return sci;
}

static GtkWidget* create_tab(AppState *app, const string &filename="") {
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *sci = create_scintilla_widget(app);
    gtk_container_add(GTK_CONTAINER(scrolled), sci);

    GtkWidget *label = gtk_label_new(filename.empty() ? "new 1" : filename.c_str());
    GtkWidget *tab = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(tab), label, FALSE, FALSE, 0);
    gtk_widget_show_all(tab);

    TabData *td = new TabData{sci, filename, false};
    g_object_set_data(G_OBJECT(scrolled), "tabdata", td);
    g_object_set_data(G_OBJECT(scrolled), "labelfwd", label);

    if (!filename.empty()) {
        apply_lexer(sci, filename);
    }
    
    // Apply current view settings
    scintilla_send_message((ScintillaObject*)sci, SCI_SETWRAPMODE, 
                          app->word_wrap ? SC_WRAP_WORD : SC_WRAP_NONE, 0);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETVIEWWS, 
                          app->show_whitespace ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE, 0);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETVIEWEOL, app->show_eol, 0);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINWIDTHN, 0, 
                          app->show_line_numbers ? 40 : 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), scrolled, tab);
    gtk_widget_show_all(scrolled);
    return scrolled;
}

// Command handlers
static void cmd_new(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    create_tab(app, "");
    gtk_notebook_set_current_page(GTK_NOTEBOOK(app->notebook), 
                                  gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook))-1);
}

static void cmd_open(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(app->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::ifstream t(filename, std::ios::binary);
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        GtkWidget *tab = create_tab(app, filename);
        TabData *td = (TabData*)g_object_get_data(G_OBJECT(tab), "tabdata");
        if (td) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTEXT, 0, (sptr_t)str.c_str());
            td->modified = false;
            GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
            gtk_label_set_text(GTK_LABEL(label), filename);
            add_recent_file(app, filename);
        }
        gtk_notebook_set_current_page(GTK_NOTEBOOK(app->notebook), 
                                      gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook))-1);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void cmd_save(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;

    if (td->filename.empty()) {
        GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(app->window),
                                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                                        "_Save", GTK_RESPONSE_ACCEPT, NULL);
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            td->filename = filename;
            g_free(filename);
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);
    }

    int len = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
    std::string buf(len+1, '\0');
    scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXT, len+1, (sptr_t)&buf[0]);
    std::ofstream out(td->filename, std::ios::binary);
    out << buf;
    td->modified = false;
    gint page = gtk_notebook_get_current_page(GTK_NOTEBOOK(app->notebook));
    GtkWidget *tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), page);
    GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
    gtk_label_set_text(GTK_LABEL(label), td->filename.c_str());
}

static void cmd_saveas(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save As", GTK_WINDOW(app->window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    if (!td->filename.empty()) {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), td->filename.c_str());
    }
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        int len = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
        std::string buf(len+1, '\0');
        scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXT, len+1, (sptr_t)&buf[0]);
        std::ofstream out(filename, std::ios::binary);
        out << buf;
        td->filename = filename;
        td->modified = false;
        gint page = gtk_notebook_get_current_page(GTK_NOTEBOOK(app->notebook));
        GtkWidget *tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), page);
        GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
        gtk_label_set_text(GTK_LABEL(label), filename);
        apply_lexer(td->sci, filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// Auto-save functionality
static gboolean auto_save_timer_callback(gpointer data) {
    AppState *app = (AppState*)data;
    
    // Save all modified files with filenames
    int num_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
    int saved_count = 0;
    
    for (int i = 0; i < num_pages; i++) {
        GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), i);
        TabData *td = (TabData*)g_object_get_data(G_OBJECT(page), "tabdata");
        
        if (td && td->modified && !td->filename.empty()) {
            int len = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
            std::string buf(len+1, '\0');
            scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXT, len+1, (sptr_t)&buf[0]);
            std::ofstream out(td->filename, std::ios::binary);
            out << buf;
            td->modified = false;
            
            GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(page), "labelfwd");
            gtk_label_set_text(GTK_LABEL(label), td->filename.c_str());
            saved_count++;
        }
    }
    
    if (saved_count > 0) {
        string msg = "Auto-saved " + std::to_string(saved_count) + " file(s)";
        gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, msg.c_str());
    }
    
    return TRUE;  // Continue timer
}

static void start_auto_save_timer(AppState *app, Preferences *prefs) {
    if (prefs->auto_save_enabled && prefs->auto_save_interval > 0) {
        if (app->auto_save_timer_id > 0) {
            g_source_remove(app->auto_save_timer_id);
        }
        app->auto_save_timer_id = g_timeout_add_seconds(prefs->auto_save_interval, 
                                                         auto_save_timer_callback, app);
    }
}

static void stop_auto_save_timer(AppState *app) {
    if (app->auto_save_timer_id > 0) {
        g_source_remove(app->auto_save_timer_id);
        app->auto_save_timer_id = 0;
    }
}

static void cmd_undo(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_UNDO, 0, 0);
        record_macro_action(app, "UNDO");
    }
}

static void cmd_redo(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_REDO, 0, 0);
        record_macro_action(app, "REDO");
    }
}

static void cmd_cut(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_CUT, 0, 0);
        record_macro_action(app, "CUT");
    }
}

static void cmd_copy(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_COPY, 0, 0);
        record_macro_action(app, "COPY");
    }
}

static void cmd_paste(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_PASTE, 0, 0);
        record_macro_action(app, "PASTE");
    }
}

static void cmd_delete(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_CLEAR, 0, 0);
        record_macro_action(app, "DELETE");
    }
}

static void cmd_selectall(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SELECTALL, 0, 0);
        record_macro_action(app, "SELECTALL");
    }
}

static void cmd_find(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Find", GTK_WINDOW(app->window),
                                                     GTK_DIALOG_MODAL,
                                                     "_Find", GTK_RESPONSE_OK,
                                                     "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), app->last_search.c_str());
    GtkWidget *regex_check = gtk_check_button_new_with_label("Regular expression");
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new("Find what:"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), regex_check, FALSE, FALSE, 5);
    gtk_widget_show_all(content);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
        bool use_regex = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(regex_check));
        app->last_search = text;
        
        if (use_regex) {
            // Use regex search
            try {
                std::regex pattern(text);
                int doc_length = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
                int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0);
                
                // Get document text from current position to end
                char *doc_text = new char[doc_length + 1];
                scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXT, doc_length + 1, (sptr_t)doc_text);
                
                std::string search_text(doc_text + pos);
                std::smatch match;
                if (std::regex_search(search_text, match, pattern)) {
                    int found_pos = pos + match.position();
                    int found_len = match.length();
                    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEL, found_pos, found_pos + found_len);
                }
                delete[] doc_text;
            } catch (const std::regex_error &e) {
                GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                    "Invalid regular expression: %s", e.what());
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
            }
        } else {
            // Use literal search
            int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0);
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, pos, 0);
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, 
                                  scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0), 0);
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEARCHFLAGS, 0, 0);
            int found = scintilla_send_message((ScintillaObject*)td->sci, SCI_SEARCHINTARGET, strlen(text), (sptr_t)text);
            if (found >= 0) {
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEL, found, found + strlen(text));
            }
        }
    }
    gtk_widget_destroy(dialog);
}

static void cmd_replace(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Replace", GTK_WINDOW(app->window),
                                                     GTK_DIALOG_MODAL,
                                                     "_Replace", 1,
                                                     "Replace _All", 2,
                                                     "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *find_entry = gtk_entry_new();
    GtkWidget *replace_entry = gtk_entry_new();
    GtkWidget *regex_check = gtk_check_button_new_with_label("Regular expression");
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new("Find what:"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), find_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new("Replace with:"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), replace_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), regex_check, FALSE, FALSE, 5);
    gtk_widget_show_all(content);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == 1 || response == 2) {
        const char *find_text = gtk_entry_get_text(GTK_ENTRY(find_entry));
        const char *replace_text = gtk_entry_get_text(GTK_ENTRY(replace_entry));
        bool use_regex = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(regex_check));
        
        if (use_regex) {
            // Use regex replace
            try {
                std::regex pattern(find_text);
                int doc_length = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
                char *doc_text = new char[doc_length + 1];
                scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXT, doc_length + 1, (sptr_t)doc_text);
                
                std::string text(doc_text);
                std::string result;
                
                if (response == 2) { // Replace All
                    result = std::regex_replace(text, pattern, replace_text);
                } else { // Replace first occurrence from current position
                    int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0);
                    std::string before = text.substr(0, pos);
                    std::string after = text.substr(pos);
                    result = before + std::regex_replace(after, pattern, replace_text, std::regex_constants::format_first_only);
                }
                
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTEXT, 0, (sptr_t)result.c_str());
                delete[] doc_text;
            } catch (const std::regex_error &e) {
                GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                    "Invalid regular expression: %s", e.what());
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
            }
        } else {
            // Use literal replace
            if (response == 2) { // Replace All
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, 0, 0);
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, 
                                      scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0), 0);
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEARCHFLAGS, 0, 0);
                
                while (true) {
                    int found = scintilla_send_message((ScintillaObject*)td->sci, SCI_SEARCHINTARGET, 
                                                      strlen(find_text), (sptr_t)find_text);
                    if (found < 0) break;
                    scintilla_send_message((ScintillaObject*)td->sci, SCI_REPLACETARGET, 
                                          strlen(replace_text), (sptr_t)replace_text);
                    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, found + strlen(replace_text), 0);
                    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, 
                                          scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0), 0);
                }
            }
        }
    }
    gtk_widget_destroy(dialog);
}

static void cmd_goto(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Go To Line", GTK_WINDOW(app->window),
                                                     GTK_DIALOG_MODAL,
                                                     "_Go", GTK_RESPONSE_OK,
                                                     "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *spin = gtk_spin_button_new_with_range(1, 999999, 1);
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new("Line number:"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), spin, FALSE, FALSE, 5);
    gtk_widget_show_all(content);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        int line = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin)) - 1;
        int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_POSITIONFROMLINE, line, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_GOTOPOS, pos, 0);
    }
    gtk_widget_destroy(dialog);
}

// Find in Files structures and functions
struct FindInFilesResult {
    string filename;
    int line_number;
    string line_text;
};

static std::vector<FindInFilesResult> find_in_files_results;

static void search_in_file(const string &filepath, const string &search_text, bool case_sensitive) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    
    string line;
    int line_num = 1;
    while (std::getline(file, line)) {
        string search_in = line;
        string search_for = search_text;
        
        if (!case_sensitive) {
            std::transform(search_in.begin(), search_in.end(), search_in.begin(), ::tolower);
            std::transform(search_for.begin(), search_for.end(), search_for.begin(), ::tolower);
        }
        
        if (search_in.find(search_for) != string::npos) {
            FindInFilesResult result;
            result.filename = filepath;
            result.line_number = line_num;
            result.line_text = line;
            find_in_files_results.push_back(result);
        }
        line_num++;
    }
    file.close();
}

static void search_directory(const string &dir_path, const string &pattern, 
                             const string &search_text, bool case_sensitive, bool recursive) {
    DIR *dir = opendir(dir_path.c_str());
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name == "." || name == "..") continue;
        
        string full_path = dir_path + "/" + name;
        struct stat st;
        if (stat(full_path.c_str(), &st) != 0) continue;
        
        if (S_ISDIR(st.st_mode)) {
            if (recursive) {
                search_directory(full_path, pattern, search_text, case_sensitive, recursive);
            }
        } else if (S_ISREG(st.st_mode)) {
            // Check if filename matches pattern
            if (fnmatch(pattern.c_str(), name.c_str(), 0) == 0) {
                search_in_file(full_path, search_text, case_sensitive);
            }
        }
    }
    closedir(dir);
}

static void show_find_in_files_results(AppState *app) {
    GtkWidget *results_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(results_window), "Find in Files Results");
    gtk_window_set_default_size(GTK_WINDOW(results_window), 800, 400);
    gtk_window_set_transient_for(GTK_WINDOW(results_window), GTK_WINDOW(app->window));
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(results_window), scrolled);
    
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), text_view);
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    string results_text = "Find in Files Results\n";
    results_text += "=====================\n\n";
    results_text += "Found " + std::to_string(find_in_files_results.size()) + " matches\n\n";
    
    for (const auto &result : find_in_files_results) {
        results_text += result.filename + ":" + std::to_string(result.line_number) + ": " + result.line_text + "\n";
    }
    
    gtk_text_buffer_set_text(buffer, results_text.c_str(), -1);
    gtk_widget_show_all(results_window);
}

static void cmd_find_in_files(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Find in Files",
        GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL,
        "_Find", GTK_RESPONSE_OK,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_set_spacing(GTK_BOX(content), 5);
    gtk_container_set_border_width(GTK_CONTAINER(content), 10);
    
    // Search text
    GtkWidget *search_label = gtk_label_new("Find what:");
    GtkWidget *search_entry = gtk_entry_new();
    gtk_widget_set_size_request(search_entry, 400, -1);
    
    // Directory
    GtkWidget *dir_label = gtk_label_new("Directory:");
    GtkWidget *dir_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(dir_entry), ".");
    
    // File pattern
    GtkWidget *pattern_label = gtk_label_new("File pattern:");
    GtkWidget *pattern_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pattern_entry), "*.*");
    
    // Options
    GtkWidget *case_check = gtk_check_button_new_with_label("Case sensitive");
    GtkWidget *recursive_check = gtk_check_button_new_with_label("Search in subdirectories");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recursive_check), TRUE);
    
    gtk_box_pack_start(GTK_BOX(content), search_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), search_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), dir_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), dir_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), pattern_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), pattern_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), case_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), recursive_check, FALSE, FALSE, 0);
    
    gtk_widget_show_all(content);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *search_text = gtk_entry_get_text(GTK_ENTRY(search_entry));
        const char *dir_path = gtk_entry_get_text(GTK_ENTRY(dir_entry));
        const char *pattern = gtk_entry_get_text(GTK_ENTRY(pattern_entry));
        bool case_sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(case_check));
        bool recursive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(recursive_check));
        
        if (strlen(search_text) > 0) {
            find_in_files_results.clear();
            search_directory(dir_path, pattern, search_text, case_sensitive, recursive);
            show_find_in_files_results(app);
        }
    }
    
    gtk_widget_destroy(dialog);
}

static void cmd_about(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Notepad++ GTK");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "0.1");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "A GTK port of Notepad++");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Preferences implementation
void Preferences::load() {
    string config_dir = string(g_get_home_dir()) + "/.config/notepad-plus-plus-gtk";
    string pref_file = config_dir + "/preferences.ini";
    
    std::ifstream file(pref_file);
    if (!file.is_open()) return;
    
    string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == string::npos) continue;
        
        string key = line.substr(0, eq);
        string value = line.substr(eq + 1);
        
        if (key == "tab_width") tab_width = std::stoi(value);
        else if (key == "font_size") font_size = std::stoi(value);
        else if (key == "use_tabs") use_tabs = (value == "true" || value == "1");
        else if (key == "auto_indent") auto_indent = (value == "true" || value == "1");
        else if (key == "show_indent_guides") show_indent_guides = (value == "true" || value == "1");
        else if (key == "highlight_current_line") highlight_current_line = (value == "true" || value == "1");
        else if (key == "font_name") font_name = value;
        else if (key == "edge_column") edge_column = std::stoi(value);
        else if (key == "show_edge_line") show_edge_line = (value == "true" || value == "1");
        else if (key == "auto_save_enabled") auto_save_enabled = (value == "true" || value == "1");
        else if (key == "auto_save_interval") auto_save_interval = std::stoi(value);
    }
}

void Preferences::save() {
    string config_dir = string(g_get_home_dir()) + "/.config/notepad-plus-plus-gtk";
    g_mkdir_with_parents(config_dir.c_str(), 0755);
    
    string pref_file = config_dir + "/preferences.ini";
    std::ofstream file(pref_file);
    if (!file.is_open()) return;
    
    file << "# Notepad++ GTK Preferences\n";
    file << "tab_width=" << tab_width << "\n";
    file << "font_size=" << font_size << "\n";
    file << "use_tabs=" << (use_tabs ? "true" : "false") << "\n";
    file << "auto_indent=" << (auto_indent ? "true" : "false") << "\n";
    file << "show_indent_guides=" << (show_indent_guides ? "true" : "false") << "\n";
    file << "highlight_current_line=" << (highlight_current_line ? "true" : "false") << "\n";
    file << "font_name=" << font_name << "\n";
    file << "edge_column=" << edge_column << "\n";
    file << "show_edge_line=" << (show_edge_line ? "true" : "false") << "\n";
    file << "auto_save_enabled=" << (auto_save_enabled ? "true" : "false") << "\n";
    file << "auto_save_interval=" << auto_save_interval << "\n";
}

void Preferences::apply_to_scintilla(GtkWidget *sci) {
    // Tab settings
    scintilla_send_message((ScintillaObject*)sci, SCI_SETTABWIDTH, tab_width, 0);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETUSETABS, use_tabs ? 1 : 0, 0);
    
    // Indentation
    scintilla_send_message((ScintillaObject*)sci, SCI_SETINDENT, tab_width, 0);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETINDENTATIONGUIDES, 
                          show_indent_guides ? SC_IV_LOOKBOTH : SC_IV_NONE, 0);
    
    // Current line highlighting
    scintilla_send_message((ScintillaObject*)sci, SCI_SETCARETLINEVISIBLE, highlight_current_line ? 1 : 0, 0);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETCARETLINEBACK, 0xE8E8FF, 0);
    
    // Edge column
    scintilla_send_message((ScintillaObject*)sci, SCI_SETEDGEMODE, 
                          show_edge_line ? EDGE_LINE : EDGE_NONE, 0);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETEDGECOLUMN, edge_column, 0);
    
    // Font
    string font_spec = font_name + " " + std::to_string(font_size);
    PangoFontDescription *font_desc = pango_font_description_from_string(font_spec.c_str());
    if (font_desc) {
        scintilla_send_message((ScintillaObject*)sci, SCI_STYLESETFONT, STYLE_DEFAULT, 
                              (sptr_t)pango_font_description_get_family(font_desc));
        scintilla_send_message((ScintillaObject*)sci, SCI_STYLESETSIZE, STYLE_DEFAULT, 
                              pango_font_description_get_size(font_desc) / PANGO_SCALE);
        scintilla_send_message((ScintillaObject*)sci, SCI_STYLECLEARALL, 0, 0);
        pango_font_description_free(font_desc);
    }
}

static void cmd_preferences(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    static Preferences prefs;
    prefs.load();
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Preferences", GTK_WINDOW(app->window),
                                                     GTK_DIALOG_MODAL,
                                                     "_OK", GTK_RESPONSE_OK,
                                                     "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(content), notebook, TRUE, TRUE, 0);
    
    // Editor Settings Tab
    GtkWidget *editor_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(editor_box), 10);
    
    GtkWidget *tab_width_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(tab_width_box), gtk_label_new("Tab Width:"), FALSE, FALSE, 0);
    GtkWidget *tab_width_spin = gtk_spin_button_new_with_range(1, 16, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(tab_width_spin), prefs.tab_width);
    gtk_box_pack_start(GTK_BOX(tab_width_box), tab_width_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_box), tab_width_box, FALSE, FALSE, 0);
    
    GtkWidget *use_tabs_check = gtk_check_button_new_with_label("Use tabs (instead of spaces)");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_tabs_check), prefs.use_tabs);
    gtk_box_pack_start(GTK_BOX(editor_box), use_tabs_check, FALSE, FALSE, 0);
    
    GtkWidget *auto_indent_check = gtk_check_button_new_with_label("Auto-indent");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(auto_indent_check), prefs.auto_indent);
    gtk_box_pack_start(GTK_BOX(editor_box), auto_indent_check, FALSE, FALSE, 0);
    
    GtkWidget *indent_guides_check = gtk_check_button_new_with_label("Show indent guides");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(indent_guides_check), prefs.show_indent_guides);
    gtk_box_pack_start(GTK_BOX(editor_box), indent_guides_check, FALSE, FALSE, 0);
    
    GtkWidget *edge_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(edge_box), gtk_label_new("Edge Column:"), FALSE, FALSE, 0);
    GtkWidget *edge_spin = gtk_spin_button_new_with_range(40, 200, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(edge_spin), prefs.edge_column);
    gtk_box_pack_start(GTK_BOX(edge_box), edge_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_box), edge_box, FALSE, FALSE, 0);
    
    GtkWidget *show_edge_check = gtk_check_button_new_with_label("Show edge line");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_edge_check), prefs.show_edge_line);
    gtk_box_pack_start(GTK_BOX(editor_box), show_edge_check, FALSE, FALSE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), editor_box, gtk_label_new("Editor"));
    
    // Display Settings Tab
    GtkWidget *display_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(display_box), 10);
    
    GtkWidget *font_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(font_box), gtk_label_new("Font:"), FALSE, FALSE, 0);
    GtkWidget *font_button = gtk_font_button_new();
    string font_spec = prefs.font_name + " " + std::to_string(prefs.font_size);
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(font_button), font_spec.c_str());
    gtk_box_pack_start(GTK_BOX(font_box), font_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(display_box), font_box, FALSE, FALSE, 0);
    
    GtkWidget *highlight_line_check = gtk_check_button_new_with_label("Highlight current line");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(highlight_line_check), prefs.highlight_current_line);
    gtk_box_pack_start(GTK_BOX(display_box), highlight_line_check, FALSE, FALSE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), display_box, gtk_label_new("Display"));
    
    // Auto-Save Tab
    GtkWidget *autosave_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(autosave_box), 10);
    
    GtkWidget *auto_save_check = gtk_check_button_new_with_label("Enable auto-save");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(auto_save_check), prefs.auto_save_enabled);
    gtk_box_pack_start(GTK_BOX(autosave_box), auto_save_check, FALSE, FALSE, 0);
    
    GtkWidget *interval_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(interval_box), gtk_label_new("Auto-save interval (seconds):"), FALSE, FALSE, 0);
    GtkWidget *interval_spin = gtk_spin_button_new_with_range(60, 3600, 30);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(interval_spin), prefs.auto_save_interval);
    gtk_box_pack_start(GTK_BOX(interval_box), interval_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(autosave_box), interval_box, FALSE, FALSE, 0);
    
    GtkWidget *info_label = gtk_label_new("Auto-save will save all modified files with filenames.\nUnsaved new files are not auto-saved.");
    gtk_label_set_line_wrap(GTK_LABEL(info_label), TRUE);
    gtk_box_pack_start(GTK_BOX(autosave_box), info_label, FALSE, FALSE, 10);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), autosave_box, gtk_label_new("Auto-Save"));
    
    gtk_widget_show_all(content);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        // Save preferences
        prefs.tab_width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(tab_width_spin));
        prefs.use_tabs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_tabs_check));
        prefs.auto_indent = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(auto_indent_check));
        prefs.show_indent_guides = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(indent_guides_check));
        prefs.edge_column = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(edge_spin));
        prefs.show_edge_line = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_edge_check));
        prefs.highlight_current_line = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(highlight_line_check));
        
        prefs.auto_save_enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(auto_save_check));
        prefs.auto_save_interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(interval_spin));
        
        const char *font_name_full = gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_button));
        PangoFontDescription *font_desc = pango_font_description_from_string(font_name_full);
        if (font_desc) {
            prefs.font_name = pango_font_description_get_family(font_desc);
            prefs.font_size = pango_font_description_get_size(font_desc) / PANGO_SCALE;
            pango_font_description_free(font_desc);
        }
        
        prefs.save();
        
        // Apply to all open tabs
        int n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
        for (int i = 0; i < n_pages; i++) {
            GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), i);
            TabData *td = (TabData*)g_object_get_data(G_OBJECT(page), "tabdata");
            if (td) {
                prefs.apply_to_scintilla(td->sci);
            }
        }
        
        // Apply to second notebook if split
        if (app->is_split && app->notebook2) {
            n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook2));
            for (int i = 0; i < n_pages; i++) {
                GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook2), i);
                TabData *td = (TabData*)g_object_get_data(G_OBJECT(page), "tabdata");
                if (td) {
                    prefs.apply_to_scintilla(td->sci);
                }
            }
        }
        
        // Restart auto-save timer with new settings
        stop_auto_save_timer(app);
        start_auto_save_timer(app, &prefs);
    }
    
    gtk_widget_destroy(dialog);
}

// Macro functionality
static void record_macro_action(AppState *app, const string &action) {
    if (app->is_recording_macro) {
        app->current_macro.push_back(action);
    }
}

static void cmd_start_macro_recording(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    if (app->is_recording_macro) {
        // Already recording, show message
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            "Macro recording is already in progress.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    app->is_recording_macro = true;
    app->current_macro.clear();
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, "Recording macro...");
}

static void cmd_stop_macro_recording(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    if (!app->is_recording_macro) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            "No macro recording in progress.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    app->is_recording_macro = false;
    if (!app->current_macro.empty()) {
        app->saved_macros.push_back(app->current_macro);
        
        // Save to file
        string config_dir = string(g_get_home_dir()) + "/.config/notepad-plus-plus-gtk/macros";
        g_mkdir_with_parents(config_dir.c_str(), 0755);
        string macro_file = config_dir + "/macro_" + std::to_string(app->saved_macros.size()) + ".txt";
        
        std::ofstream file(macro_file);
        if (file.is_open()) {
            for (const auto &action : app->current_macro) {
                file << action << "\n";
            }
        }
    }
    
    string status_msg = "Macro recorded: " + std::to_string(app->current_macro.size()) + " actions";
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, status_msg.c_str());
}

static void cmd_playback_macro(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    if (app->current_macro.empty()) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            "No macro to playback. Record a macro first.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Playback recorded actions
    for (const auto &action : app->current_macro) {
        if (action.substr(0, 6) == "TYPE:") {
            string text = action.substr(6);
            scintilla_send_message((ScintillaObject*)td->sci, SCI_ADDTEXT, text.length(), (sptr_t)text.c_str());
        } else if (action == "BACKSPACE") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_DELETEBACK, 0, 0);
        } else if (action == "DELETE") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_CLEAR, 0, 0);
        } else if (action == "NEWLINE") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_NEWLINE, 0, 0);
        } else if (action == "TAB") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_TAB, 0, 0);
        } else if (action == "UNDO") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_UNDO, 0, 0);
        } else if (action == "REDO") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_REDO, 0, 0);
        } else if (action == "CUT") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_CUT, 0, 0);
        } else if (action == "COPY") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_COPY, 0, 0);
        } else if (action == "PASTE") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_PASTE, 0, 0);
        } else if (action == "SELECTALL") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SELECTALL, 0, 0);
        } else if (action == "DUPLICATE_LINE") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEDUPLICATE, 0, 0);
        } else if (action == "DELETE_LINE") {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEDELETE, 0, 0);
        }
    }
    
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, "Macro played back");
}

static void cmd_save_macro(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    
    if (app->current_macro.empty()) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            "No macro to save. Record a macro first.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Macro", GTK_WINDOW(app->window),
                                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                                     "_Cancel", GTK_RESPONSE_CANCEL,
                                                     "_Save", GTK_RESPONSE_ACCEPT, NULL);
    
    string config_dir = string(g_get_home_dir()) + "/.config/notepad-plus-plus-gtk/macros";
    g_mkdir_with_parents(config_dir.c_str(), 0755);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), config_dir.c_str());
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "macro.txt");
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::ofstream file(filename);
        if (file.is_open()) {
            for (const auto &action : app->current_macro) {
                file << action << "\n";
            }
            string status_msg = string("Macro saved to: ") + filename;
            gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, status_msg.c_str());
        }
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

static void cmd_load_macro(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Load Macro", GTK_WINDOW(app->window),
                                                     GTK_FILE_CHOOSER_ACTION_OPEN,
                                                     "_Cancel", GTK_RESPONSE_CANCEL,
                                                     "_Open", GTK_RESPONSE_ACCEPT, NULL);
    
    string config_dir = string(g_get_home_dir()) + "/.config/notepad-plus-plus-gtk/macros";
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), config_dir.c_str());
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::ifstream file(filename);
        if (file.is_open()) {
            app->current_macro.clear();
            string line;
            while (std::getline(file, line)) {
                if (!line.empty()) {
                    app->current_macro.push_back(line);
                }
            }
            string status_msg = "Macro loaded: " + std::to_string(app->current_macro.size()) + " actions";
            gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, status_msg.c_str());
        }
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// Incremental search functionality
static void on_incremental_search_changed(GtkEntry *entry, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    const char *search_text = gtk_entry_get_text(entry);
    if (strlen(search_text) == 0) {
        // Clear highlighting
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETINDICATORCURRENT, 0, 0);
        int doc_length = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_INDICATORCLEARRANGE, 0, doc_length);
        return;
    }
    
    // Clear previous highlighting
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETINDICATORCURRENT, 0, 0);
    int doc_length = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_INDICATORCLEARRANGE, 0, doc_length);
    
    // Setup indicator for highlighting
    scintilla_send_message((ScintillaObject*)td->sci, SCI_INDICSETSTYLE, 0, INDIC_ROUNDBOX);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_INDICSETFORE, 0, 0x00FF00);  // Green highlight
    scintilla_send_message((ScintillaObject*)td->sci, SCI_INDICSETALPHA, 0, 100);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETINDICATORCURRENT, 0, 0);
    
    // Search and highlight all matches
    int search_flags = 0;  // Case insensitive
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEARCHFLAGS, search_flags, 0);
    
    int pos = 0;
    int first_match = -1;
    while (pos < doc_length) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, pos, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, doc_length, 0);
        int found = scintilla_send_message((ScintillaObject*)td->sci, SCI_SEARCHINTARGET, 
                                          strlen(search_text), (sptr_t)search_text);
        if (found < 0) break;
        
        if (first_match < 0) first_match = found;
        
        int match_end = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTARGETEND, 0, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_INDICATORFILLRANGE, found, match_end - found);
        pos = match_end;
    }
    
    // Jump to first match
    if (first_match >= 0) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEL, first_match, 
                              first_match + strlen(search_text));
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SCROLLCARET, 0, 0);
    }
}

static void on_incremental_search_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    AppState *app = (AppState*)data;
    
    if (event->keyval == GDK_KEY_Escape) {
        // Close search bar
        if (app->incremental_search_bar) {
            gtk_widget_hide(app->incremental_search_bar);
            app->incremental_search_active = false;
            
            // Clear highlighting
            TabData *td = get_current_tabdata(app);
            if (td) {
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETINDICATORCURRENT, 0, 0);
                int doc_length = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
                scintilla_send_message((ScintillaObject*)td->sci, SCI_INDICATORCLEARRANGE, 0, doc_length);
            }
        }
    } else if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
        // Find next
        TabData *td = get_current_tabdata(app);
        if (!td) return;
        
        const char *search_text = gtk_entry_get_text(GTK_ENTRY(app->incremental_search_entry));
        if (strlen(search_text) == 0) return;
        
        int current_pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0);
        int doc_length = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
        
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, current_pos, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, doc_length, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEARCHFLAGS, 0, 0);
        
        int found = scintilla_send_message((ScintillaObject*)td->sci, SCI_SEARCHINTARGET, 
                                          strlen(search_text), (sptr_t)search_text);
        if (found >= 0) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEL, found, 
                                  found + strlen(search_text));
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SCROLLCARET, 0, 0);
        } else {
            // Wrap to beginning
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, 0, 0);
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, current_pos, 0);
            found = scintilla_send_message((ScintillaObject*)td->sci, SCI_SEARCHINTARGET, 
                                          strlen(search_text), (sptr_t)search_text);
            if (found >= 0) {
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEL, found, 
                                      found + strlen(search_text));
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SCROLLCARET, 0, 0);
            }
        }
    }
}

static void cmd_incremental_search(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    
    if (!app->incremental_search_bar) {
        // Create search bar (will be added to the main window later in init)
        app->incremental_search_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_container_set_border_width(GTK_CONTAINER(app->incremental_search_bar), 5);
        
        GtkWidget *label = gtk_label_new("Incremental Search:");
        gtk_box_pack_start(GTK_BOX(app->incremental_search_bar), label, FALSE, FALSE, 0);
        
        app->incremental_search_entry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(app->incremental_search_bar), app->incremental_search_entry, TRUE, TRUE, 0);
        
        GtkWidget *close_button = gtk_button_new_with_label("Close");
        gtk_box_pack_start(GTK_BOX(app->incremental_search_bar), close_button, FALSE, FALSE, 0);
        
        g_signal_connect(app->incremental_search_entry, "changed", 
                        G_CALLBACK(on_incremental_search_changed), app);
        g_signal_connect(app->incremental_search_entry, "key-press-event", 
                        G_CALLBACK(on_incremental_search_key_press), app);
        g_signal_connect_swapped(close_button, "clicked", 
                                G_CALLBACK(gtk_widget_hide), app->incremental_search_bar);
    }
    
    if (!app->incremental_search_active) {
        gtk_widget_show_all(app->incremental_search_bar);
        app->incremental_search_active = true;
        gtk_widget_grab_focus(app->incremental_search_entry);
        gtk_entry_set_text(GTK_ENTRY(app->incremental_search_entry), "");
    } else {
        gtk_widget_grab_focus(app->incremental_search_entry);
    }
}

// View menu commands
static void apply_view_settings(AppState *app) {
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETWRAPMODE, 
                          app->word_wrap ? SC_WRAP_WORD : SC_WRAP_NONE, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETVIEWWS, 
                          app->show_whitespace ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETVIEWEOL, app->show_eol, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETMARGINWIDTHN, 0, 
                          app->show_line_numbers ? 40 : 0);
}

static void cmd_toggle_word_wrap(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    app->word_wrap = !app->word_wrap;
    
    // Apply to all tabs
    int n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
    for (int i = 0; i < n_pages; i++) {
        GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), i);
        TabData *td = (TabData*)g_object_get_data(G_OBJECT(page), "tabdata");
        if (td) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETWRAPMODE, 
                                  app->word_wrap ? SC_WRAP_WORD : SC_WRAP_NONE, 0);
        }
    }
}

static void cmd_toggle_whitespace(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    app->show_whitespace = !app->show_whitespace;
    
    // Apply to all tabs
    int n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
    for (int i = 0; i < n_pages; i++) {
        GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), i);
        TabData *td = (TabData*)g_object_get_data(G_OBJECT(page), "tabdata");
        if (td) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETVIEWWS, 
                                  app->show_whitespace ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE, 0);
        }
    }
}

static void cmd_toggle_eol(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    app->show_eol = !app->show_eol;
    
    // Apply to all tabs
    int n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
    for (int i = 0; i < n_pages; i++) {
        GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), i);
        TabData *td = (TabData*)g_object_get_data(G_OBJECT(page), "tabdata");
        if (td) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETVIEWEOL, app->show_eol, 0);
        }
    }
}

static void cmd_toggle_line_numbers(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    app->show_line_numbers = !app->show_line_numbers;
    
    // Apply to all tabs
    int n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
    for (int i = 0; i < n_pages; i++) {
        GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), i);
        TabData *td = (TabData*)g_object_get_data(G_OBJECT(page), "tabdata");
        if (td) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETMARGINWIDTHN, 0, 
                                  app->show_line_numbers ? 40 : 0);
        }
    }
}

static void cmd_zoom_in(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_ZOOMIN, 0, 0);
}

static void cmd_zoom_out(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_ZOOMOUT, 0, 0);
}

static void cmd_zoom_restore(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETZOOM, 0, 0);
}

// Split view operations
static void cmd_split_horizontal(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    if (app->is_split) return;  // Already split
    
    // Create second notebook
    app->notebook2 = gtk_notebook_new();
    gtk_widget_show(app->notebook2);
    
    // Remove notebook from its current parent
    GtkWidget *parent = gtk_widget_get_parent(app->notebook);
    g_object_ref(app->notebook);
    gtk_container_remove(GTK_CONTAINER(parent), app->notebook);
    
    // Create horizontal paned widget
    app->paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_pack1(GTK_PANED(app->paned), app->notebook, TRUE, TRUE);
    gtk_paned_pack2(GTK_PANED(app->paned), app->notebook2, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(app->paned), 350);  // Split in middle
    
    // Add paned to parent
    gtk_box_pack_start(GTK_BOX(parent), app->paned, TRUE, TRUE, 0);
    gtk_widget_show(app->paned);
    g_object_unref(app->notebook);
    
    app->is_split = true;
    app->is_horizontal_split = true;
}

static void cmd_split_vertical(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    if (app->is_split) return;  // Already split
    
    // Create second notebook
    app->notebook2 = gtk_notebook_new();
    gtk_widget_show(app->notebook2);
    
    // Remove notebook from its current parent
    GtkWidget *parent = gtk_widget_get_parent(app->notebook);
    g_object_ref(app->notebook);
    gtk_container_remove(GTK_CONTAINER(parent), app->notebook);
    
    // Create vertical paned widget
    app->paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(app->paned), app->notebook, TRUE, TRUE);
    gtk_paned_pack2(GTK_PANED(app->paned), app->notebook2, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(app->paned), 500);  // Split in middle
    
    // Add paned to parent
    gtk_box_pack_start(GTK_BOX(parent), app->paned, TRUE, TRUE, 0);
    gtk_widget_show(app->paned);
    g_object_unref(app->notebook);
    
    app->is_split = true;
    app->is_horizontal_split = false;
}

static void cmd_unsplit(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    if (!app->is_split) return;  // Not split
    
    // Get parent of paned
    GtkWidget *parent = gtk_widget_get_parent(app->paned);
    
    // Remove paned and destroy second notebook
    g_object_ref(app->notebook);
    gtk_container_remove(GTK_CONTAINER(app->paned), app->notebook);
    gtk_widget_destroy(app->paned);
    
    // Add notebook back to parent
    gtk_box_pack_start(GTK_BOX(parent), app->notebook, TRUE, TRUE, 0);
    g_object_unref(app->notebook);
    
    app->is_split = false;
    app->notebook2 = nullptr;
    app->paned = nullptr;
}

// Folding commands
static void cmd_fold_all(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int maxLine = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINECOUNT, 0, 0);
    for (int line = 0; line < maxLine; line++) {
        int level = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETFOLDLEVEL, line, 0);
        if (level & SC_FOLDLEVELHEADERFLAG) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETFOLDEXPANDED, line, 0);
        }
    }
    // Refresh display
    scintilla_send_message((ScintillaObject*)td->sci, SCI_COLOURISE, 0, -1);
}

static void cmd_unfold_all(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int maxLine = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINECOUNT, 0, 0);
    for (int line = 0; line < maxLine; line++) {
        int level = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETFOLDLEVEL, line, 0);
        if (level & SC_FOLDLEVELHEADERFLAG) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETFOLDEXPANDED, line, 1);
        }
    }
    // Refresh display
    scintilla_send_message((ScintillaObject*)td->sci, SCI_COLOURISE, 0, -1);
}

static void cmd_toggle_fold(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int line = scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEFROMPOSITION,
                                      scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0), 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_TOGGLEFOLD, line, 0);
}

static void cmd_toggle_fullscreen(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    if (app->is_fullscreen) {
        gtk_window_unfullscreen(GTK_WINDOW(app->window));
        app->is_fullscreen = false;
    } else {
        gtk_window_fullscreen(GTK_WINDOW(app->window));
        app->is_fullscreen = true;
    }
}

static void cmd_toggle_distraction_free(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    if (app->is_distraction_free) {
        // Exit distraction-free mode
        if (app->menubar) gtk_widget_show(app->menubar);
        if (app->toolbar) gtk_widget_show(app->toolbar);
        gtk_widget_show(app->statusbar);
        app->is_distraction_free = false;
        gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, "Distraction-free mode: OFF");
    } else {
        // Enter distraction-free mode
        if (app->menubar) gtk_widget_hide(app->menubar);
        if (app->toolbar) gtk_widget_hide(app->toolbar);
        gtk_widget_hide(app->statusbar);
        app->is_distraction_free = true;
    }
}

// Line operations
static void cmd_line_duplicate(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEDUPLICATE, 0, 0);
    record_macro_action(app, "DUPLICATE_LINE");
}

static void cmd_line_delete(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEDELETE, 0, 0);
    record_macro_action(app, "DELETE_LINE");
}

static void cmd_line_move_up(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_MOVESELECTEDLINESUP, 0, 0);
}

static void cmd_line_move_down(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_MOVESELECTEDLINESDOWN, 0, 0);
}

static void cmd_line_transpose(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_LINETRANSPOSE, 0, 0);
}

static void cmd_line_cut(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_LINECUT, 0, 0);
}

static void cmd_line_copy(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_LINECOPY, 0, 0);
}

// Text transformation operations
static void cmd_uppercase(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_UPPERCASE, 0, 0);
}

static void cmd_lowercase(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_LOWERCASE, 0, 0);
}

static void cmd_trim_trailing(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int lines = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINECOUNT, 0, 0);
    for (int i = 0; i < lines; i++) {
        int lineStart = scintilla_send_message((ScintillaObject*)td->sci, SCI_POSITIONFROMLINE, i, 0);
        int lineEnd = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINEENDPOSITION, i, 0);
        int length = lineEnd - lineStart;
        
        if (length > 0) {
            char *line = new char[length + 1];
            
            // Use SCI_GETTEXTRANGEFULL with proper struct
            Sci_TextRangeFull tr;
            tr.chrg.cpMin = lineStart;
            tr.chrg.cpMax = lineEnd;
            tr.lpstrText = line;
            scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTRANGEFULL, 0, (sptr_t)&tr);
            
            // Find trailing whitespace
            int j = length - 1;
            while (j >= 0 && (line[j] == ' ' || line[j] == '\t')) {
                j--;
            }
            
            if (j < length - 1) {
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, lineStart + j + 1, 0);
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, lineEnd, 0);
                scintilla_send_message((ScintillaObject*)td->sci, SCI_REPLACETARGET, 0, (sptr_t)"");
            }
            delete[] line;
        }
    }
}

static void cmd_tabs_to_spaces(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int tab_width = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTABWIDTH, 0, 0);
    int doc_length = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
    
    // Get entire document text
    char *doc_text = new char[doc_length + 1];
    scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXT, doc_length + 1, (sptr_t)doc_text);
    
    // Build new text with tabs converted to spaces
    string new_text;
    new_text.reserve(doc_length * 2);  // Reserve enough space
    
    for (int i = 0; i < doc_length; i++) {
        if (doc_text[i] == '\t') {
            // Convert tab to spaces
            for (int j = 0; j < tab_width; j++) {
                new_text += ' ';
            }
        } else {
            new_text += doc_text[i];
        }
    }
    
    // Replace document text
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTEXT, 0, (sptr_t)new_text.c_str());
    delete[] doc_text;
    
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, "Converted tabs to spaces");
}

static void cmd_spaces_to_tabs(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int tab_width = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTABWIDTH, 0, 0);
    int lines = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINECOUNT, 0, 0);
    
    // Process line by line (easier for leading spaces)
    for (int i = 0; i < lines; i++) {
        int lineStart = scintilla_send_message((ScintillaObject*)td->sci, SCI_POSITIONFROMLINE, i, 0);
        int lineEnd = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINEENDPOSITION, i, 0);
        int length = lineEnd - lineStart;
        
        if (length > 0) {
            char *line = new char[length + 1];
            Sci_TextRangeFull tr;
            tr.chrg.cpMin = lineStart;
            tr.chrg.cpMax = lineEnd;
            tr.lpstrText = line;
            scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTRANGEFULL, 0, (sptr_t)&tr);
            
            // Count leading spaces
            int space_count = 0;
            int j = 0;
            while (j < length && line[j] == ' ') {
                space_count++;
                j++;
            }
            
            // Convert leading spaces to tabs
            if (space_count >= tab_width) {
                int num_tabs = space_count / tab_width;
                int remaining_spaces = space_count % tab_width;
                
                string new_line;
                for (int t = 0; t < num_tabs; t++) {
                    new_line += '\t';
                }
                for (int s = 0; s < remaining_spaces; s++) {
                    new_line += ' ';
                }
                new_line += string(line + space_count);
                
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, lineStart, 0);
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, lineEnd, 0);
                scintilla_send_message((ScintillaObject*)td->sci, SCI_REPLACETARGET, new_line.length(), (sptr_t)new_line.c_str());
            }
            delete[] line;
        }
    }
    
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, "Converted leading spaces to tabs");
}

static void cmd_sort_lines_ascending(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int lines = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINECOUNT, 0, 0);
    
    // Collect all lines
    std::vector<string> line_list;
    for (int i = 0; i < lines; i++) {
        int lineStart = scintilla_send_message((ScintillaObject*)td->sci, SCI_POSITIONFROMLINE, i, 0);
        int lineEnd = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINEENDPOSITION, i, 0);
        int length = lineEnd - lineStart;
        
        if (length > 0) {
            char *line = new char[length + 1];
            Sci_TextRangeFull tr;
            tr.chrg.cpMin = lineStart;
            tr.chrg.cpMax = lineEnd;
            tr.lpstrText = line;
            scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTRANGEFULL, 0, (sptr_t)&tr);
            line_list.push_back(string(line));
            delete[] line;
        } else {
            line_list.push_back("");
        }
    }
    
    // Sort lines
    std::sort(line_list.begin(), line_list.end());
    
    // Rebuild document
    string new_text;
    for (size_t i = 0; i < line_list.size(); i++) {
        new_text += line_list[i];
        if (i < line_list.size() - 1) {
            new_text += '\n';
        }
    }
    
    // Replace document
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTEXT, 0, (sptr_t)new_text.c_str());
    
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, "Sorted lines (ascending)");
}

static void cmd_sort_lines_descending(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int lines = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINECOUNT, 0, 0);
    
    // Collect all lines
    std::vector<string> line_list;
    for (int i = 0; i < lines; i++) {
        int lineStart = scintilla_send_message((ScintillaObject*)td->sci, SCI_POSITIONFROMLINE, i, 0);
        int lineEnd = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINEENDPOSITION, i, 0);
        int length = lineEnd - lineStart;
        
        if (length > 0) {
            char *line = new char[length + 1];
            Sci_TextRangeFull tr;
            tr.chrg.cpMin = lineStart;
            tr.chrg.cpMax = lineEnd;
            tr.lpstrText = line;
            scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTRANGEFULL, 0, (sptr_t)&tr);
            line_list.push_back(string(line));
            delete[] line;
        } else {
            line_list.push_back("");
        }
    }
    
    // Sort lines in descending order
    std::sort(line_list.begin(), line_list.end(), std::greater<string>());
    
    // Rebuild document
    string new_text;
    for (size_t i = 0; i < line_list.size(); i++) {
        new_text += line_list[i];
        if (i < line_list.size() - 1) {
            new_text += '\n';
        }
    }
    
    // Replace document
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTEXT, 0, (sptr_t)new_text.c_str());
    
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, "Sorted lines (descending)");
}

static void cmd_indent(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_TAB, 0, 0);
}

static void cmd_unindent(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_BACKTAB, 0, 0);
}

// Language selection
static void cmd_set_language(GtkWidget *w, gpointer data) {
    struct LangData { AppState *app; const char *lang; };
    LangData *ld = (LangData*)data;
    TabData *td = get_current_tabdata(ld->app);
    if (!td) return;
    
    // Map language names to Lexilla lexer names
    const char *lexer_name = nullptr;
    if (strcmp(ld->lang, "C++") == 0) lexer_name = "cpp";
    else if (strcmp(ld->lang, "C") == 0) lexer_name = "cpp";
    else if (strcmp(ld->lang, "Python") == 0) lexer_name = "python";
    else if (strcmp(ld->lang, "JavaScript") == 0) lexer_name = "cpp";
    else if (strcmp(ld->lang, "HTML") == 0) lexer_name = "hypertext";
    else if (strcmp(ld->lang, "CSS") == 0) lexer_name = "css";
    else if (strcmp(ld->lang, "PHP") == 0) lexer_name = "phpscript";
    else if (strcmp(ld->lang, "Perl") == 0) lexer_name = "perl";
    else if (strcmp(ld->lang, "Ruby") == 0) lexer_name = "ruby";
    else if (strcmp(ld->lang, "Rust") == 0) lexer_name = "rust";
    else if (strcmp(ld->lang, "Go") == 0) lexer_name = "go";
    else if (strcmp(ld->lang, "Java") == 0) lexer_name = "cpp";
    else if (strcmp(ld->lang, "Shell") == 0) lexer_name = "bash";
    else if (strcmp(ld->lang, "SQL") == 0) lexer_name = "sql";
    else if (strcmp(ld->lang, "XML") == 0) lexer_name = "xml";
    else if (strcmp(ld->lang, "JSON") == 0) lexer_name = "json";
    else if (strcmp(ld->lang, "Markdown") == 0) lexer_name = "markdown";
    else if (strcmp(ld->lang, "YAML") == 0) lexer_name = "yaml";
    else if (strcmp(ld->lang, "Plain Text") == 0) lexer_name = "null";
    
    if (lexer_name) {
        Scintilla::ILexer5 *lexer = CreateLexer(lexer_name);
        if (lexer) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETILEXER, 0, (sptr_t)lexer);
        }
    }
}

// EOL format commands
static void cmd_set_eol_windows(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETEOLMODE, SC_EOL_CRLF, 0);
}

static void cmd_set_eol_unix(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETEOLMODE, SC_EOL_LF, 0);
}

static void cmd_set_eol_mac(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETEOLMODE, SC_EOL_CR, 0);
}

static void cmd_convert_eol(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    int mode = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETEOLMODE, 0, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_CONVERTEOLS, mode, 0);
}

// EOL conversion commands (set mode and convert)
static void cmd_convert_to_windows(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETEOLMODE, SC_EOL_CRLF, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_CONVERTEOLS, SC_EOL_CRLF, 0);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, "Converted to Windows format (CRLF)");
}

static void cmd_convert_to_unix(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETEOLMODE, SC_EOL_LF, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_CONVERTEOLS, SC_EOL_LF, 0);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, "Converted to Unix format (LF)");
}

static void cmd_convert_to_mac(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETEOLMODE, SC_EOL_CR, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_CONVERTEOLS, SC_EOL_CR, 0);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, "Converted to Mac format (CR)");
}

// Find Next/Previous
static void cmd_find_next(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td || app->last_search.empty()) return;
    
    int flags = app->find_case_sensitive ? SCFIND_MATCHCASE : 0;
    int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0);
    
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, pos, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, 
                          scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0), 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEARCHFLAGS, flags, 0);
    
    int found = scintilla_send_message((ScintillaObject*)td->sci, SCI_SEARCHINTARGET, 
                                      app->last_search.length(), (sptr_t)app->last_search.c_str());
    if (found >= 0) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEL, found, found + app->last_search.length());
        app->last_find_pos = found;
    } else {
        // Wrap around
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, 0, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, pos, 0);
        found = scintilla_send_message((ScintillaObject*)td->sci, SCI_SEARCHINTARGET, 
                                      app->last_search.length(), (sptr_t)app->last_search.c_str());
        if (found >= 0) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEL, found, found + app->last_search.length());
            app->last_find_pos = found;
        }
    }
}

static void cmd_find_previous(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td || app->last_search.empty()) return;
    
    int flags = app->find_case_sensitive ? SCFIND_MATCHCASE : 0;
    int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0);
    
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, 0, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, pos, 0);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEARCHFLAGS, flags, 0);
    
    // Search backwards
    int found = -1;
    int last_found = -1;
    int search_pos = 0;
    while (search_pos < pos) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, search_pos, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, pos, 0);
        found = scintilla_send_message((ScintillaObject*)td->sci, SCI_SEARCHINTARGET, 
                                      app->last_search.length(), (sptr_t)app->last_search.c_str());
        if (found < 0) break;
        last_found = found;
        search_pos = found + 1;
    }
    
    if (last_found >= 0) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEL, last_found, last_found + app->last_search.length());
        app->last_find_pos = last_found;
    } else {
        // Wrap around to end
        int doc_len = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
        search_pos = pos;
        last_found = -1;
        while (search_pos < doc_len) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, search_pos, 0);
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, doc_len, 0);
            found = scintilla_send_message((ScintillaObject*)td->sci, SCI_SEARCHINTARGET, 
                                          app->last_search.length(), (sptr_t)app->last_search.c_str());
            if (found < 0) break;
            last_found = found;
            search_pos = found + 1;
        }
        if (last_found >= 0) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEL, last_found, last_found + app->last_search.length());
            app->last_find_pos = last_found;
        }
    }
}

// Bookmarks
static void cmd_toggle_bookmark(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int line = scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEFROMPOSITION,
                                     scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0), 0);
    int marker = scintilla_send_message((ScintillaObject*)td->sci, SCI_MARKERGET, line, 0);
    
    if (marker & (1 << 1)) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_MARKERDELETE, line, 1);
    } else {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_MARKERADD, line, 1);
    }
}

static void cmd_next_bookmark(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int line = scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEFROMPOSITION,
                                     scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0), 0);
    int next_line = scintilla_send_message((ScintillaObject*)td->sci, SCI_MARKERNEXT, line + 1, 1 << 1);
    
    if (next_line < 0) {
        next_line = scintilla_send_message((ScintillaObject*)td->sci, SCI_MARKERNEXT, 0, 1 << 1);
    }
    
    if (next_line >= 0) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_GOTOLINE, next_line, 0);
    }
}

static void cmd_previous_bookmark(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int line = scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEFROMPOSITION,
                                     scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0), 0);
    int prev_line = scintilla_send_message((ScintillaObject*)td->sci, SCI_MARKERPREVIOUS, line - 1, 1 << 1);
    
    if (prev_line < 0) {
        int line_count = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINECOUNT, 0, 0);
        prev_line = scintilla_send_message((ScintillaObject*)td->sci, SCI_MARKERPREVIOUS, line_count - 1, 1 << 1);
    }
    
    if (prev_line >= 0) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_GOTOLINE, prev_line, 0);
    }
}

static void cmd_clear_bookmarks(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_MARKERDELETEALL, 1, 0);
}

// Comment operations
static void cmd_block_comment(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int start = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETSELECTIONSTART, 0, 0);
    int end = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETSELECTIONEND, 0, 0);
    int start_line = scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEFROMPOSITION, start, 0);
    int end_line = scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEFROMPOSITION, end, 0);
    
    scintilla_send_message((ScintillaObject*)td->sci, SCI_BEGINUNDOACTION, 0, 0);
    for (int i = start_line; i <= end_line; i++) {
        int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_POSITIONFROMLINE, i, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_INSERTTEXT, pos, (sptr_t)"// ");
    }
    scintilla_send_message((ScintillaObject*)td->sci, SCI_ENDUNDOACTION, 0, 0);
}

static void cmd_block_uncomment(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int start = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETSELECTIONSTART, 0, 0);
    int end = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETSELECTIONEND, 0, 0);
    int start_line = scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEFROMPOSITION, start, 0);
    int end_line = scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEFROMPOSITION, end, 0);
    
    scintilla_send_message((ScintillaObject*)td->sci, SCI_BEGINUNDOACTION, 0, 0);
    for (int i = start_line; i <= end_line; i++) {
        int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_POSITIONFROMLINE, i, 0);
        int line_end = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINEENDPOSITION, i, 0);
        int len = line_end - pos;
        
        if (len >= 3) {
            char buf[4];
            Sci_TextRangeFull tr;
            tr.chrg.cpMin = pos;
            tr.chrg.cpMax = pos + 3;
            tr.lpstrText = buf;
            scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTRANGEFULL, 0, (sptr_t)&tr);
            
            if (strncmp(buf, "// ", 3) == 0) {
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, pos, 0);
                scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, pos + 3, 0);
                scintilla_send_message((ScintillaObject*)td->sci, SCI_REPLACETARGET, 0, (sptr_t)"");
            } else if (len >= 2) {
                tr.chrg.cpMax = pos + 2;
                scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTRANGEFULL, 0, (sptr_t)&tr);
                if (strncmp(buf, "//", 2) == 0) {
                    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, pos, 0);
                    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, pos + 2, 0);
                    scintilla_send_message((ScintillaObject*)td->sci, SCI_REPLACETARGET, 0, (sptr_t)"");
                }
            }
        }
    }
    scintilla_send_message((ScintillaObject*)td->sci, SCI_ENDUNDOACTION, 0, 0);
}

// Tab management
static void cmd_close_all(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    int n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
    
    for (int i = n_pages - 1; i >= 0; i--) {
        GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), i);
        TabData *td = (TabData*)g_object_get_data(G_OBJECT(page), "tabdata");
        if (td) {
            delete td;
        }
        gtk_notebook_remove_page(GTK_NOTEBOOK(app->notebook), i);
    }
    
    // Create a new empty tab
    create_tab(app, "");
}

static void cmd_next_tab(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    int current = gtk_notebook_get_current_page(GTK_NOTEBOOK(app->notebook));
    int n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(app->notebook), (current + 1) % n_pages);
}

static void cmd_prev_tab(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    int current = gtk_notebook_get_current_page(GTK_NOTEBOOK(app->notebook));
    int n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(app->notebook), (current - 1 + n_pages) % n_pages);
}

// Additional line operations
static void cmd_join_lines(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0);
    int line = scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEFROMPOSITION, pos, 0);
    int line_count = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINECOUNT, 0, 0);
    
    if (line < line_count - 1) {
        int line_end = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLINEENDPOSITION, line, 0);
        int next_line_start = scintilla_send_message((ScintillaObject*)td->sci, SCI_POSITIONFROMLINE, line + 1, 0);
        
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, line_end, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, next_line_start, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_REPLACETARGET, 1, (sptr_t)" ");
    }
}

static void cmd_split_lines(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int start = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETSELECTIONSTART, 0, 0);
    int end = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETSELECTIONEND, 0, 0);
    
    if (start == end) {
        // No selection, split at edge distance
        int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0);
        int line = scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEFROMPOSITION, pos, 0);
        int line_start = scintilla_send_message((ScintillaObject*)td->sci, SCI_POSITIONFROMLINE, line, 0);
        int edge = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETEDGECOLUMN, 0, 0);
        if (edge <= 0) edge = 80;
        
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEARCHFLAGS, 0, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETSTART, line_start, 0);
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTARGETEND, line_start + edge, 0);
        
        // Find last space before edge
        for (int i = line_start + edge; i > line_start; i--) {
            char c = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCHARAT, i, 0);
            if (c == ' ' || c == '\t') {
                scintilla_send_message((ScintillaObject*)td->sci, SCI_INSERTTEXT, i, (sptr_t)"\n");
                break;
            }
        }
    }
}

// Recent files management
static void update_recent_menu(AppState *app);

static void cmd_open_recent(GtkWidget *w, gpointer data) {
    struct RecentData { AppState *app; string filename; };
    RecentData *rd = (RecentData*)data;
    
    std::ifstream t(rd->filename, std::ios::binary);
    if (!t.good()) return;
    
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    GtkWidget *tab = create_tab(rd->app, rd->filename);
    TabData *td = (TabData*)g_object_get_data(G_OBJECT(tab), "tabdata");
    if (td) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTEXT, 0, (sptr_t)str.c_str());
        td->modified = false;
        GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
        if (label) gtk_label_set_text(GTK_LABEL(label), rd->filename.c_str());
        update_statusbar(rd->app, td->sci);
    }
    gtk_notebook_set_current_page(GTK_NOTEBOOK(rd->app->notebook), 
                                  gtk_notebook_get_n_pages(GTK_NOTEBOOK(rd->app->notebook))-1);
}

static void add_recent_file(AppState *app, const string &filename) {
    // Remove if already exists
    auto it = std::find(app->recent_files.begin(), app->recent_files.end(), filename);
    if (it != app->recent_files.end()) {
        app->recent_files.erase(it);
    }
    
    // Add to front
    app->recent_files.insert(app->recent_files.begin(), filename);
    
    // Keep only 10 most recent
    if (app->recent_files.size() > 10) {
        app->recent_files.resize(10);
    }
    
    update_recent_menu(app);
}

static void cmd_select_word(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0);
    int start = scintilla_send_message((ScintillaObject*)td->sci, SCI_WORDSTARTPOSITION, pos, true);
    int end = scintilla_send_message((ScintillaObject*)td->sci, SCI_WORDENDPOSITION, pos, true);
    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETSEL, start, end);
}

static void cmd_autocomplete(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    // Get current word at cursor
    int pos = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETCURRENTPOS, 0, 0);
    int wordStart = scintilla_send_message((ScintillaObject*)td->sci, SCI_WORDSTARTPOSITION, pos, true);
    int wordEnd = scintilla_send_message((ScintillaObject*)td->sci, SCI_WORDENDPOSITION, pos, true);
    
    // Get the word prefix
    char wordBuf[256];
    int wordLen = wordEnd - wordStart;
    if (wordLen >= sizeof(wordBuf)) wordLen = sizeof(wordBuf) - 1;
    
    if (wordLen > 0) {
        Sci_TextRangeFull tr;
        tr.chrg.cpMin = wordStart;
        tr.chrg.cpMax = wordEnd;
        tr.lpstrText = wordBuf;
        scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTRANGEFULL, 0, (sptr_t)&tr);
        wordBuf[wordLen] = '\0';
    } else {
        wordBuf[0] = '\0';
    }
    
    // Collect all words from the document
    std::set<std::string> words;
    int docLength = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETLENGTH, 0, 0);
    
    int currentPos = 0;
    while (currentPos < docLength) {
        int wordStartPos = scintilla_send_message((ScintillaObject*)td->sci, SCI_WORDSTARTPOSITION, currentPos, true);
        int wordEndPos = scintilla_send_message((ScintillaObject*)td->sci, SCI_WORDENDPOSITION, currentPos, true);
        
        if (wordEndPos > wordStartPos) {
            char buf[256];
            int len = wordEndPos - wordStartPos;
            if (len >= sizeof(buf)) len = sizeof(buf) - 1;
            
            Sci_TextRangeFull tr;
            tr.chrg.cpMin = wordStartPos;
            tr.chrg.cpMax = wordEndPos;
            tr.lpstrText = buf;
            scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTRANGEFULL, 0, (sptr_t)&tr);
            buf[len] = '\0';
            
            // Only add words that are alphanumeric/underscore and at least 3 chars
            bool valid = true;
            for (int i = 0; buf[i]; i++) {
                if (!isalnum(buf[i]) && buf[i] != '_') {
                    valid = false;
                    break;
                }
            }
            if (valid && strlen(buf) >= 3) {
                words.insert(buf);
            }
        }
        
        currentPos = wordEndPos + 1;
    }
    
    // Build completion list
    std::string completionList;
    for (const auto& word : words) {
        if (wordBuf[0] == '\0' || word.find(wordBuf) == 0) {
            if (!completionList.empty()) completionList += " ";
            completionList += word;
        }
    }
    
    if (!completionList.empty()) {
        scintilla_send_message((ScintillaObject*)td->sci, SCI_AUTOCSHOW, strlen(wordBuf), (sptr_t)completionList.c_str());
    }
}

// Multi-cursor editing
static void cmd_add_next_occurrence(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    ScintillaObject *sci = (ScintillaObject*)td->sci;
    
    // Enable multiple selection if not already enabled
    scintilla_send_message(sci, SCI_SETMULTIPLESELECTION, 1, 0);
    scintilla_send_message(sci, SCI_SETADDITIONALSELECTIONTYPING, 1, 0);
    scintilla_send_message(sci, SCI_SETMULTIPASTE, 1, 0);
    scintilla_send_message(sci, SCI_SETVIRTUALSPACEOPTIONS, SCVS_RECTANGULARSELECTION, 0);
    
    int num_selections = scintilla_send_message(sci, SCI_GETSELECTIONS, 0, 0);
    
    // Get the last selection (most recent)
    int main_sel = scintilla_send_message(sci, SCI_GETMAINSELECTION, 0, 0);
    int sel_start = scintilla_send_message(sci, SCI_GETSELECTIONNSTART, main_sel, 0);
    int sel_end = scintilla_send_message(sci, SCI_GETSELECTIONNEND, main_sel, 0);
    
    // If no selection, select the word under cursor
    if (sel_start == sel_end) {
        int pos = scintilla_send_message(sci, SCI_GETCURRENTPOS, 0, 0);
        sel_start = scintilla_send_message(sci, SCI_WORDSTARTPOSITION, pos, true);
        sel_end = scintilla_send_message(sci, SCI_WORDENDPOSITION, pos, true);
        scintilla_send_message(sci, SCI_SETSELECTION, sel_start, sel_end);
        return;
    }
    
    // Get the selected text
    int len = sel_end - sel_start;
    char *text = new char[len + 1];
    Sci_TextRangeFull tr;
    tr.chrg.cpMin = sel_start;
    tr.chrg.cpMax = sel_end;
    tr.lpstrText = text;
    scintilla_send_message(sci, SCI_GETTEXTRANGEFULL, 0, (sptr_t)&tr);
    
    // Search for next occurrence after the current selection
    scintilla_send_message(sci, SCI_SETTARGETSTART, sel_end, 0);
    scintilla_send_message(sci, SCI_SETTARGETEND, scintilla_send_message(sci, SCI_GETLENGTH, 0, 0), 0);
    scintilla_send_message(sci, SCI_SETSEARCHFLAGS, SCFIND_MATCHCASE, 0);
    
    int found_pos = scintilla_send_message(sci, SCI_SEARCHINTARGET, len, (sptr_t)text);
    
    if (found_pos >= 0) {
        int found_end = scintilla_send_message(sci, SCI_GETTARGETEND, 0, 0);
        // Add additional selection
        scintilla_send_message(sci, SCI_ADDSELECTION, found_end, found_pos);
        scintilla_send_message(sci, SCI_SETMAINSELECTION, num_selections, 0);
        // Scroll to show the new selection
        scintilla_send_message(sci, SCI_SCROLLRANGE, found_pos, found_end);
    }
    
    delete[] text;
}

static void cmd_select_all_occurrences(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    ScintillaObject *sci = (ScintillaObject*)td->sci;
    
    int sel_start = scintilla_send_message(sci, SCI_GETSELECTIONSTART, 0, 0);
    int sel_end = scintilla_send_message(sci, SCI_GETSELECTIONEND, 0, 0);
    
    // If no selection, select the word under cursor
    if (sel_start == sel_end) {
        int pos = scintilla_send_message(sci, SCI_GETCURRENTPOS, 0, 0);
        sel_start = scintilla_send_message(sci, SCI_WORDSTARTPOSITION, pos, true);
        sel_end = scintilla_send_message(sci, SCI_WORDENDPOSITION, pos, true);
        if (sel_start == sel_end) return;
    }
    
    // Enable multiple selection
    scintilla_send_message(sci, SCI_SETMULTIPLESELECTION, 1, 0);
    scintilla_send_message(sci, SCI_SETADDITIONALSELECTIONTYPING, 1, 0);
    scintilla_send_message(sci, SCI_SETMULTIPASTE, 1, 0);
    
    // Get the selected text
    int len = sel_end - sel_start;
    char *text = new char[len + 1];
    Sci_TextRangeFull tr;
    tr.chrg.cpMin = sel_start;
    tr.chrg.cpMax = sel_end;
    tr.lpstrText = text;
    scintilla_send_message(sci, SCI_GETTEXTRANGEFULL, 0, (sptr_t)&tr);
    
    // Clear existing selections and set the first one
    scintilla_send_message(sci, SCI_CLEARSELECTIONS, 0, 0);
    scintilla_send_message(sci, SCI_SETSELECTION, sel_start, sel_end);
    
    // Find all occurrences
    int doc_length = scintilla_send_message(sci, SCI_GETLENGTH, 0, 0);
    int search_pos = sel_end;
    int count = 1;
    
    while (search_pos < doc_length) {
        scintilla_send_message(sci, SCI_SETTARGETSTART, search_pos, 0);
        scintilla_send_message(sci, SCI_SETTARGETEND, doc_length, 0);
        scintilla_send_message(sci, SCI_SETSEARCHFLAGS, SCFIND_MATCHCASE, 0);
        
        int found_pos = scintilla_send_message(sci, SCI_SEARCHINTARGET, len, (sptr_t)text);
        
        if (found_pos < 0) break;
        
        int found_end = scintilla_send_message(sci, SCI_GETTARGETEND, 0, 0);
        scintilla_send_message(sci, SCI_ADDSELECTION, found_end, found_pos);
        count++;
        search_pos = found_end;
    }
    
    delete[] text;
    
    // Show message in status bar
    char msg[256];
    snprintf(msg, sizeof(msg), "Selected %d occurrence(s)", count);
    gtk_statusbar_pop(GTK_STATUSBAR(app->statusbar), app->status_context);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context, msg);
}

static void cmd_clear_multiple_selections(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    
    ScintillaObject *sci = (ScintillaObject*)td->sci;
    
    // Keep only main selection
    int main_sel = scintilla_send_message(sci, SCI_GETMAINSELECTION, 0, 0);
    int start = scintilla_send_message(sci, SCI_GETSELECTIONNSTART, main_sel, 0);
    int end = scintilla_send_message(sci, SCI_GETSELECTIONNEND, main_sel, 0);
    
    scintilla_send_message(sci, SCI_CLEARSELECTIONS, 0, 0);
    scintilla_send_message(sci, SCI_SETSELECTION, start, end);
    
    update_statusbar(app, td->sci);
}

static void update_recent_menu(AppState *app) {
    if (!app->recent_menu) return;
    
    // Clear existing items
    GList *children = gtk_container_get_children(GTK_CONTAINER(app->recent_menu));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // Add recent files
    if (app->recent_files.empty()) {
        GtkWidget *empty_item = gtk_menu_item_new_with_label("(No Recent Files)");
        gtk_widget_set_sensitive(empty_item, FALSE);
        gtk_menu_shell_append(GTK_MENU_SHELL(app->recent_menu), empty_item);
        gtk_widget_show(empty_item);
    } else {
        for (size_t i = 0; i < app->recent_files.size(); i++) {
            string label = std::to_string(i + 1) + ". " + app->recent_files[i];
            GtkWidget *item = gtk_menu_item_new_with_label(label.c_str());
            
            // We need to pass the filename, so we'll use g_object_set_data
            g_object_set_data_full(G_OBJECT(item), "filename", 
                                  g_strdup(app->recent_files[i].c_str()), g_free);
            g_object_set_data(G_OBJECT(item), "app", app);
            
            g_signal_connect(item, "activate", G_CALLBACK(+[](GtkWidget *w, gpointer data) {
                AppState *app = (AppState*)g_object_get_data(G_OBJECT(w), "app");
                const char *filename = (const char*)g_object_get_data(G_OBJECT(w), "filename");
                
                std::ifstream t(filename, std::ios::binary);
                if (!t.good()) return;
                
                std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
                GtkWidget *tab = create_tab(app, filename);
                TabData *td = (TabData*)g_object_get_data(G_OBJECT(tab), "tabdata");
                if (td) {
                    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTEXT, 0, (sptr_t)str.c_str());
                    td->modified = false;
                    GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
                    if (label) gtk_label_set_text(GTK_LABEL(label), filename);
                    update_statusbar(app, td->sci);
                }
                gtk_notebook_set_current_page(GTK_NOTEBOOK(app->notebook), 
                                              gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook))-1);
            }), NULL);
            
            gtk_menu_shell_append(GTK_MENU_SHELL(app->recent_menu), item);
            gtk_widget_show(item);
        }
    }
}

// Session management functions
static string get_config_dir() {
    const char *home = getenv("HOME");
    if (!home) home = getenv("USERPROFILE"); // Windows fallback
    if (!home) return "";
    
    string config_dir = string(home) + "/.config/notepad-plus-plus-gtk";
    return config_dir;
}

static void ensure_config_dir() {
    string config_dir = get_config_dir();
    if (config_dir.empty()) return;
    
    // Create parent directory first (~/.config)
    const char *home = getenv("HOME");
    if (home) {
        string parent = string(home) + "/.config";
        mkdir(parent.c_str(), 0755);
    }
    
    // Create directory if it doesn't exist
    mkdir(config_dir.c_str(), 0755);
}

static void session_save(AppState *app) {
    ensure_config_dir();
    string session_file = get_config_dir() + "/session.txt";
    
    std::ofstream ofs(session_file);
    if (!ofs.is_open()) return;
    
    // Save all open tabs from both notebooks (if split)
    auto save_notebook = [&](GtkWidget *notebook) {
        if (!notebook) return;
        int n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
        for (int i = 0; i < n_pages; i++) {
            GtkWidget *tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), i);
            TabData *td = (TabData*)g_object_get_data(G_OBJECT(tab), "tabdata");
            if (td && !td->filename.empty()) {
                ofs << td->filename << "\n";
            }
        }
    };
    
    save_notebook(app->notebook);
    if (app->is_split && app->notebook2) {
        save_notebook(app->notebook2);
    }
    
    ofs.close();
}

static void session_restore(AppState *app);  // Forward declare for use in session_restore

static GtkWidget* create_tab(AppState *app, const string &filename);  // Forward declare without default arg

static void session_restore(AppState *app) {
    string session_file = get_config_dir() + "/session.txt";
    std::ifstream ifs(session_file);
    if (!ifs.is_open()) return;
    
    string line;
    bool first = true;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        
        // Check if file exists
        std::ifstream test(line);
        if (test.good()) {
            test.close();
            if (first) {
                // Replace the initial empty tab
                first = false;
                GtkWidget *tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), 0);
                TabData *td = (TabData*)g_object_get_data(G_OBJECT(tab), "tabdata");
                if (td) {
                    // Load file into existing tab
                    std::ifstream file(line);
                    std::string content((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
                    scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTEXT, 0, (sptr_t)content.c_str());
                    td->filename = line;
                    td->modified = false;
                    
                    // Update tab label
                    size_t pos = line.find_last_of("/\\");
                    string basename = (pos != string::npos) ? line.substr(pos + 1) : line;
                    GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
                    if (label) gtk_label_set_text(GTK_LABEL(label), basename.c_str());
                    
                    update_statusbar(app, td->sci);
                    add_recent_file(app, line);
                }
            } else {
                // Create new tab
                create_tab(app, line);
                add_recent_file(app, line);
            }
        }
    }
    ifs.close();
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    AppState app;
    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(app.window), 1000, 700);
    gtk_window_set_title(GTK_WINDOW(app.window), "Notepad++");
    
    // Set Notepad++ icon - try multiple paths
    GError *error = nullptr;
    const char *icon_paths[] = {
        "npp.ico",           // Current directory
        "../npp.ico",        // Parent directory (when running from build/)
        nullptr
    };
    
    bool icon_loaded = false;
    for (int i = 0; icon_paths[i] != nullptr; i++) {
        if (gtk_window_set_icon_from_file(GTK_WINDOW(app.window), icon_paths[i], &error)) {
            icon_loaded = true;
            break;
        }
        if (error) {
            g_error_free(error);
            error = nullptr;
        }
    }
    
    if (!icon_loaded) {
        g_message("Icon not loaded (npp.ico not found in current or parent directory)");
    }

    app.accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(app.window), app.accel_group);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app.window), vbox);

    // Menu bar
    GtkWidget *menubar = gtk_menu_bar_new();
    app.menubar = menubar;  // Store for distraction-free mode
    
    // File menu
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_mnemonic("_File");
    GtkWidget *file_new = gtk_menu_item_new_with_mnemonic("_New");
    GtkWidget *file_open = gtk_menu_item_new_with_mnemonic("_Open...");
    GtkWidget *file_save = gtk_menu_item_new_with_mnemonic("_Save");
    GtkWidget *file_saveas = gtk_menu_item_new_with_mnemonic("Save _As...");
    GtkWidget *file_close = gtk_menu_item_new_with_mnemonic("_Close");
    GtkWidget *file_close_all = gtk_menu_item_new_with_mnemonic("Close A_ll");
    GtkWidget *file_save_session = gtk_menu_item_new_with_mnemonic("Save Se_ssion");
    GtkWidget *file_load_session = gtk_menu_item_new_with_mnemonic("Load S_ession");
    GtkWidget *file_quit = gtk_menu_item_new_with_mnemonic("_Quit");
    
    gtk_widget_add_accelerator(file_new, "activate", app.accel_group, GDK_KEY_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(file_open, "activate", app.accel_group, GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(file_save, "activate", app.accel_group, GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(file_saveas, "activate", app.accel_group, GDK_KEY_s, (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(file_close, "activate", app.accel_group, GDK_KEY_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(file_close_all, "activate", app.accel_group, GDK_KEY_w, (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(file_quit, "activate", app.accel_group, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_new);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_open);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_save);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_saveas);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    
    // Recent files submenu
    app.recent_menu = gtk_menu_new();
    GtkWidget *file_recent = gtk_menu_item_new_with_mnemonic("Recent _Files");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_recent), app.recent_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_recent);
    update_recent_menu(&app);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_save_session);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_load_session);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_close);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_close_all);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_quit);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);
    
    // Edit menu
    GtkWidget *edit_menu = gtk_menu_new();
    GtkWidget *edit_item = gtk_menu_item_new_with_mnemonic("_Edit");
    GtkWidget *edit_undo = gtk_menu_item_new_with_mnemonic("_Undo");
    GtkWidget *edit_redo = gtk_menu_item_new_with_mnemonic("_Redo");
    GtkWidget *edit_cut = gtk_menu_item_new_with_mnemonic("Cu_t");
    GtkWidget *edit_copy = gtk_menu_item_new_with_mnemonic("_Copy");
    GtkWidget *edit_paste = gtk_menu_item_new_with_mnemonic("_Paste");
    GtkWidget *edit_delete = gtk_menu_item_new_with_mnemonic("_Delete");
    GtkWidget *edit_selectall = gtk_menu_item_new_with_mnemonic("Select _All");
    
    // Line operations submenu
    GtkWidget *edit_line_menu = gtk_menu_new();
    GtkWidget *edit_line = gtk_menu_item_new_with_mnemonic("_Line");
    GtkWidget *edit_line_duplicate = gtk_menu_item_new_with_mnemonic("_Duplicate Line");
    GtkWidget *edit_line_delete = gtk_menu_item_new_with_mnemonic("D_elete Line");
    GtkWidget *edit_line_cut = gtk_menu_item_new_with_mnemonic("Cu_t Line");
    GtkWidget *edit_line_copy = gtk_menu_item_new_with_mnemonic("_Copy Line");
    GtkWidget *edit_line_move_up = gtk_menu_item_new_with_mnemonic("Move Line _Up");
    GtkWidget *edit_line_move_down = gtk_menu_item_new_with_mnemonic("Move Line Do_wn");
    GtkWidget *edit_line_transpose = gtk_menu_item_new_with_mnemonic("_Transpose Lines");
    GtkWidget *edit_line_join = gtk_menu_item_new_with_mnemonic("_Join Lines");
    GtkWidget *edit_line_split = gtk_menu_item_new_with_mnemonic("_Split Lines");
    
    gtk_widget_add_accelerator(edit_line_duplicate, "activate", app.accel_group, GDK_KEY_d, (GdkModifierType)(GDK_CONTROL_MASK|GDK_MOD1_MASK), GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_line_delete, "activate", app.accel_group, GDK_KEY_l, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_line_move_up, "activate", app.accel_group, GDK_KEY_Up, (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_line_move_down, "activate", app.accel_group, GDK_KEY_Down, (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_line_transpose, "activate", app.accel_group, GDK_KEY_t, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_line_join, "activate", app.accel_group, GDK_KEY_j, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), edit_line_duplicate);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), edit_line_delete);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), edit_line_cut);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), edit_line_copy);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), edit_line_move_up);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), edit_line_move_down);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), edit_line_transpose);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), edit_line_join);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_line_menu), edit_line_split);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_line), edit_line_menu);
    
    // Text case submenu
    GtkWidget *edit_case_menu = gtk_menu_new();
    GtkWidget *edit_case = gtk_menu_item_new_with_mnemonic("Con_vert Case");
    GtkWidget *edit_uppercase = gtk_menu_item_new_with_mnemonic("_UPPERCASE");
    GtkWidget *edit_lowercase = gtk_menu_item_new_with_mnemonic("_lowercase");
    
    gtk_widget_add_accelerator(edit_uppercase, "activate", app.accel_group, GDK_KEY_u, (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_lowercase, "activate", app.accel_group, GDK_KEY_u, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_case_menu), edit_uppercase);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_case_menu), edit_lowercase);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_case), edit_case_menu);
    
    // Other edit operations
    GtkWidget *edit_trim = gtk_menu_item_new_with_mnemonic("_Trim Trailing Space");
    GtkWidget *edit_tabs_to_spaces = gtk_menu_item_new_with_mnemonic("Convert _Tabs to Spaces");
    GtkWidget *edit_spaces_to_tabs = gtk_menu_item_new_with_mnemonic("Convert _Spaces to Tabs");
    
    // Sort submenu
    GtkWidget *edit_sort = gtk_menu_item_new_with_mnemonic("S_ort Lines");
    GtkWidget *edit_sort_menu = gtk_menu_new();
    GtkWidget *edit_sort_asc = gtk_menu_item_new_with_mnemonic("Sort Lines _Ascending");
    GtkWidget *edit_sort_desc = gtk_menu_item_new_with_mnemonic("Sort Lines _Descending");
    
    GtkWidget *edit_indent = gtk_menu_item_new_with_mnemonic("_Indent");
    GtkWidget *edit_unindent = gtk_menu_item_new_with_mnemonic("U_nindent");
    
    gtk_widget_add_accelerator(edit_indent, "activate", app.accel_group, GDK_KEY_Tab, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_unindent, "activate", app.accel_group, GDK_KEY_Tab, (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
    
    gtk_widget_add_accelerator(edit_undo, "activate", app.accel_group, GDK_KEY_z, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_redo, "activate", app.accel_group, GDK_KEY_y, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_cut, "activate", app.accel_group, GDK_KEY_x, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_copy, "activate", app.accel_group, GDK_KEY_c, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_paste, "activate", app.accel_group, GDK_KEY_v, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_selectall, "activate", app.accel_group, GDK_KEY_a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    // Select Word item
    GtkWidget *edit_selectword = gtk_menu_item_new_with_mnemonic("Select _Word");
    gtk_widget_add_accelerator(edit_selectword, "activate", app.accel_group, GDK_KEY_w, (GdkModifierType)(GDK_CONTROL_MASK|GDK_MOD1_MASK), GTK_ACCEL_VISIBLE);
    
    // Multi-cursor items
    GtkWidget *edit_add_next = gtk_menu_item_new_with_mnemonic("Add _Next Occurrence");
    GtkWidget *edit_select_all_occ = gtk_menu_item_new_with_mnemonic("Select All O_ccurrences");
    GtkWidget *edit_clear_selections = gtk_menu_item_new_with_mnemonic("C_lear Multiple Selections");
    
    gtk_widget_add_accelerator(edit_add_next, "activate", app.accel_group, GDK_KEY_d, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_select_all_occ, "activate", app.accel_group, GDK_KEY_l, (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_clear_selections, "activate", app.accel_group, GDK_KEY_Escape, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
    
    // Auto-completion item
    GtkWidget *edit_autocomplete = gtk_menu_item_new_with_mnemonic("_Word Completion");
    gtk_widget_add_accelerator(edit_autocomplete, "activate", app.accel_group, GDK_KEY_space, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_undo);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_redo);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_cut);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_copy);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_paste);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_delete);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_selectall);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_selectword);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_add_next);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_select_all_occ);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_clear_selections);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_autocomplete);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_line);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_case);
    
    // Comment submenu
    GtkWidget *edit_comment_menu = gtk_menu_new();
    GtkWidget *edit_comment = gtk_menu_item_new_with_mnemonic("C_omment");
    GtkWidget *edit_comment_block = gtk_menu_item_new_with_mnemonic("Block _Comment");
    GtkWidget *edit_uncomment_block = gtk_menu_item_new_with_mnemonic("Block _Uncomment");
    
    gtk_widget_add_accelerator(edit_comment_block, "activate", app.accel_group, GDK_KEY_slash, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(edit_uncomment_block, "activate", app.accel_group, GDK_KEY_slash, (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_comment_menu), edit_comment_block);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_comment_menu), edit_uncomment_block);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_comment), edit_comment_menu);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_comment);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_indent);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_unindent);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_trim);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_tabs_to_spaces);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_spaces_to_tabs);
    
    // Add sort submenu
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_sort_menu), edit_sort_asc);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_sort_menu), edit_sort_desc);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_sort), edit_sort_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_sort);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_item), edit_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit_item);
    
    // Search menu
    GtkWidget *search_menu = gtk_menu_new();
    GtkWidget *search_item = gtk_menu_item_new_with_mnemonic("_Search");
    GtkWidget *search_find = gtk_menu_item_new_with_mnemonic("_Find...");
    GtkWidget *search_find_next = gtk_menu_item_new_with_mnemonic("Find _Next");
    GtkWidget *search_find_prev = gtk_menu_item_new_with_mnemonic("Find _Previous");
    GtkWidget *search_replace = gtk_menu_item_new_with_mnemonic("_Replace...");
    GtkWidget *search_find_in_files = gtk_menu_item_new_with_mnemonic("Find in F_iles...");
    GtkWidget *search_incremental = gtk_menu_item_new_with_mnemonic("_Incremental Search");
    GtkWidget *search_goto = gtk_menu_item_new_with_mnemonic("_Go to Line...");
    
    // Bookmarks submenu
    GtkWidget *bookmark_menu = gtk_menu_new();
    GtkWidget *bookmark_item = gtk_menu_item_new_with_mnemonic("_Bookmarks");
    GtkWidget *bookmark_toggle = gtk_menu_item_new_with_mnemonic("_Toggle Bookmark");
    GtkWidget *bookmark_next = gtk_menu_item_new_with_mnemonic("_Next Bookmark");
    GtkWidget *bookmark_prev = gtk_menu_item_new_with_mnemonic("_Previous Bookmark");
    GtkWidget *bookmark_clear = gtk_menu_item_new_with_mnemonic("_Clear All Bookmarks");
    
    gtk_widget_add_accelerator(search_find, "activate", app.accel_group, GDK_KEY_f, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(search_find_next, "activate", app.accel_group, GDK_KEY_F3, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(search_find_prev, "activate", app.accel_group, GDK_KEY_F3, GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(search_replace, "activate", app.accel_group, GDK_KEY_h, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(search_find_in_files, "activate", app.accel_group, GDK_KEY_f, (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(search_incremental, "activate", app.accel_group, GDK_KEY_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(search_goto, "activate", app.accel_group, GDK_KEY_g, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(bookmark_toggle, "activate", app.accel_group, GDK_KEY_F2, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(bookmark_next, "activate", app.accel_group, GDK_KEY_F2, GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(bookmark_prev, "activate", app.accel_group, GDK_KEY_F2, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(bookmark_menu), bookmark_toggle);
    gtk_menu_shell_append(GTK_MENU_SHELL(bookmark_menu), bookmark_next);
    gtk_menu_shell_append(GTK_MENU_SHELL(bookmark_menu), bookmark_prev);
    gtk_menu_shell_append(GTK_MENU_SHELL(bookmark_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(bookmark_menu), bookmark_clear);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(bookmark_item), bookmark_menu);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), search_find);
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), search_find_next);
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), search_find_prev);
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), search_replace);
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), search_find_in_files);
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), search_incremental);
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), search_goto);
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(search_menu), bookmark_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(search_item), search_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), search_item);
    
    // View menu
    GtkWidget *view_menu = gtk_menu_new();
    GtkWidget *view_item = gtk_menu_item_new_with_mnemonic("_View");
    GtkWidget *view_word_wrap = gtk_check_menu_item_new_with_mnemonic("_Word Wrap");
    GtkWidget *view_line_numbers = gtk_check_menu_item_new_with_mnemonic("Show Line _Numbers");
    GtkWidget *view_whitespace = gtk_check_menu_item_new_with_mnemonic("Show _Whitespace");
    GtkWidget *view_eol = gtk_check_menu_item_new_with_mnemonic("Show _EOL");
    GtkWidget *view_zoom_in = gtk_menu_item_new_with_mnemonic("Zoom _In");
    GtkWidget *view_zoom_out = gtk_menu_item_new_with_mnemonic("Zoom _Out");
    GtkWidget *view_zoom_restore = gtk_menu_item_new_with_mnemonic("_Restore Default Zoom");
    
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(view_line_numbers), TRUE);
    
    gtk_widget_add_accelerator(view_zoom_in, "activate", app.accel_group, GDK_KEY_plus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(view_zoom_out, "activate", app.accel_group, GDK_KEY_minus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(view_zoom_restore, "activate", app.accel_group, GDK_KEY_0, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_word_wrap);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_line_numbers);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_whitespace);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_eol);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_in);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_out);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_restore);
    
    // Split view items
    GtkWidget *view_split_h = gtk_menu_item_new_with_mnemonic("Split _Horizontal");
    GtkWidget *view_split_v = gtk_menu_item_new_with_mnemonic("Split _Vertical");
    GtkWidget *view_unsplit = gtk_menu_item_new_with_mnemonic("_Unsplit");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_split_h);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_split_v);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_unsplit);
    
    // Folding items
    GtkWidget *view_fold_all = gtk_menu_item_new_with_mnemonic("_Fold All");
    GtkWidget *view_unfold_all = gtk_menu_item_new_with_mnemonic("U_nfold All");
    GtkWidget *view_toggle_fold = gtk_menu_item_new_with_mnemonic("_Toggle Current Fold");
    
    gtk_widget_add_accelerator(view_toggle_fold, "activate", app.accel_group, GDK_KEY_F, (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_fold_all);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_unfold_all);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_toggle_fold);
    
    // Full screen
    GtkWidget *view_fullscreen = gtk_menu_item_new_with_mnemonic("_Full Screen");
    gtk_widget_add_accelerator(view_fullscreen, "activate", app.accel_group, GDK_KEY_F11, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
    
    // Distraction-free mode
    GtkWidget *view_distraction_free = gtk_menu_item_new_with_mnemonic("_Distraction Free Mode");
    gtk_widget_add_accelerator(view_distraction_free, "activate", app.accel_group, GDK_KEY_F12, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_fullscreen);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_distraction_free);
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_item), view_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), view_item);
    
    // Language menu
    GtkWidget *lang_menu = gtk_menu_new();
    GtkWidget *lang_item = gtk_menu_item_new_with_mnemonic("_Language");
    const char *languages[] = {"Plain Text", "C", "C++", "Java", "JavaScript", "Python", "Ruby", 
                                "Perl", "PHP", "Shell", "Go", "Rust", "SQL", "HTML", "XML", 
                                "CSS", "JSON", "Markdown", "YAML", nullptr};
    for (int i = 0; languages[i]; i++) {
        GtkWidget *lang_choice = gtk_menu_item_new_with_label(languages[i]);
        gtk_menu_shell_append(GTK_MENU_SHELL(lang_menu), lang_choice);
        // Note: We'll connect signals after creating lang_menu to avoid scope issues
    }
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(lang_item), lang_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), lang_item);
    
    // Encoding menu
    GtkWidget *encoding_menu = gtk_menu_new();
    GtkWidget *encoding_item = gtk_menu_item_new_with_mnemonic("E_ncoding");
    
    GtkWidget *eol_submenu = gtk_menu_new();
    GtkWidget *eol_item = gtk_menu_item_new_with_mnemonic("_EOL Format");
    GtkWidget *eol_windows = gtk_radio_menu_item_new_with_label(nullptr, "Windows (CRLF)");
    GSList *eol_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(eol_windows));
    GtkWidget *eol_unix = gtk_radio_menu_item_new_with_label(eol_group, "Unix (LF)");
    eol_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(eol_unix));
    GtkWidget *eol_mac = gtk_radio_menu_item_new_with_label(eol_group, "Mac (CR)");
    GtkWidget *eol_convert = gtk_menu_item_new_with_label("Convert EOL");
    
    // EOL conversion submenu
    GtkWidget *eol_convert_submenu = gtk_menu_new();
    GtkWidget *eol_convert_item = gtk_menu_item_new_with_mnemonic("_Convert EOL to");
    GtkWidget *eol_convert_windows = gtk_menu_item_new_with_mnemonic("Convert to _Windows (CRLF)");
    GtkWidget *eol_convert_unix = gtk_menu_item_new_with_mnemonic("Convert to _Unix (LF)");
    GtkWidget *eol_convert_mac = gtk_menu_item_new_with_mnemonic("Convert to _Mac (CR)");
    
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(eol_unix), TRUE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), eol_windows);
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), eol_unix);
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), eol_mac);
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), eol_convert);
    
    // Add conversion submenu
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_convert_submenu), eol_convert_windows);
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_convert_submenu), eol_convert_unix);
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_convert_submenu), eol_convert_mac);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(eol_convert_item), eol_convert_submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), eol_convert_item);
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(eol_item), eol_submenu);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(encoding_menu), eol_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(encoding_item), encoding_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), encoding_item);
    
    // Settings menu
    GtkWidget *settings_menu = gtk_menu_new();
    GtkWidget *settings_item = gtk_menu_item_new_with_mnemonic("Se_ttings");
    GtkWidget *settings_preferences = gtk_menu_item_new_with_mnemonic("_Preferences");
    gtk_menu_shell_append(GTK_MENU_SHELL(settings_menu), settings_preferences);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(settings_item), settings_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), settings_item);
    
    // Macro menu
    GtkWidget *macro_menu = gtk_menu_new();
    GtkWidget *macro_item = gtk_menu_item_new_with_mnemonic("_Macro");
    GtkWidget *macro_start = gtk_menu_item_new_with_mnemonic("_Start Recording");
    GtkWidget *macro_stop = gtk_menu_item_new_with_mnemonic("St_op Recording");
    GtkWidget *macro_playback = gtk_menu_item_new_with_mnemonic("_Playback");
    GtkWidget *macro_save = gtk_menu_item_new_with_mnemonic("Sa_ve Macro...");
    GtkWidget *macro_load = gtk_menu_item_new_with_mnemonic("_Load Macro...");
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), macro_start);
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), macro_stop);
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), macro_playback);
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), macro_save);
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), macro_load);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(macro_item), macro_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), macro_item);
    
    // Help menu
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_item = gtk_menu_item_new_with_mnemonic("_Help");
    GtkWidget *help_about = gtk_menu_item_new_with_mnemonic("_About");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_about);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_item);
    
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    // Toolbar
    GtkWidget *toolbar = gtk_toolbar_new();
    app.toolbar = toolbar;  // Store for distraction-free mode
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
    GtkToolItem *tb_new = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
    GtkToolItem *tb_open = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
    GtkToolItem *tb_save = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_new, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_open, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_save, -1);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    // Notebook
    app.notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), app.notebook, TRUE, TRUE, 0);

    // Incremental search bar (initially hidden)
    cmd_incremental_search(nullptr, &app);  // Create the search bar
    gtk_box_pack_start(GTK_BOX(vbox), app.incremental_search_bar, FALSE, FALSE, 0);
    gtk_widget_hide(app.incremental_search_bar);
    app.incremental_search_active = false;

    // Status bar
    app.statusbar = gtk_statusbar_new();
    app.status_context = gtk_statusbar_get_context_id(GTK_STATUSBAR(app.statusbar), "status");
    gtk_box_pack_start(GTK_BOX(vbox), app.statusbar, FALSE, FALSE, 0);

    // Connect signals
    g_signal_connect(file_new, "activate", G_CALLBACK(cmd_new), &app);
    g_signal_connect(file_open, "activate", G_CALLBACK(cmd_open), &app);
    g_signal_connect(file_save, "activate", G_CALLBACK(cmd_save), &app);
    g_signal_connect(file_saveas, "activate", G_CALLBACK(cmd_saveas), &app);
    g_signal_connect(file_close_all, "activate", G_CALLBACK(cmd_close_all), &app);
    g_signal_connect(file_save_session, "activate", G_CALLBACK(+[](GtkWidget*, gpointer data) {
        session_save((AppState*)data);
    }), &app);
    g_signal_connect(file_load_session, "activate", G_CALLBACK(+[](GtkWidget*, gpointer data) {
        session_restore((AppState*)data);
    }), &app);
    g_signal_connect(file_quit, "activate", G_CALLBACK(gtk_main_quit), NULL);
    
    g_signal_connect(edit_undo, "activate", G_CALLBACK(cmd_undo), &app);
    g_signal_connect(edit_redo, "activate", G_CALLBACK(cmd_redo), &app);
    g_signal_connect(edit_cut, "activate", G_CALLBACK(cmd_cut), &app);
    g_signal_connect(edit_copy, "activate", G_CALLBACK(cmd_copy), &app);
    g_signal_connect(edit_paste, "activate", G_CALLBACK(cmd_paste), &app);
    g_signal_connect(edit_delete, "activate", G_CALLBACK(cmd_delete), &app);
    g_signal_connect(edit_selectall, "activate", G_CALLBACK(cmd_selectall), &app);
    g_signal_connect(edit_selectword, "activate", G_CALLBACK(cmd_select_word), &app);
    
    g_signal_connect(edit_add_next, "activate", G_CALLBACK(cmd_add_next_occurrence), &app);
    g_signal_connect(edit_select_all_occ, "activate", G_CALLBACK(cmd_select_all_occurrences), &app);
    g_signal_connect(edit_clear_selections, "activate", G_CALLBACK(cmd_clear_multiple_selections), &app);
    g_signal_connect(edit_autocomplete, "activate", G_CALLBACK(cmd_autocomplete), &app);
    
    g_signal_connect(edit_line_duplicate, "activate", G_CALLBACK(cmd_line_duplicate), &app);
    g_signal_connect(edit_line_delete, "activate", G_CALLBACK(cmd_line_delete), &app);
    g_signal_connect(edit_line_cut, "activate", G_CALLBACK(cmd_line_cut), &app);
    g_signal_connect(edit_line_copy, "activate", G_CALLBACK(cmd_line_copy), &app);
    g_signal_connect(edit_line_move_up, "activate", G_CALLBACK(cmd_line_move_up), &app);
    g_signal_connect(edit_line_move_down, "activate", G_CALLBACK(cmd_line_move_down), &app);
    g_signal_connect(edit_line_transpose, "activate", G_CALLBACK(cmd_line_transpose), &app);
    g_signal_connect(edit_line_join, "activate", G_CALLBACK(cmd_join_lines), &app);
    g_signal_connect(edit_line_split, "activate", G_CALLBACK(cmd_split_lines), &app);
    
    g_signal_connect(edit_uppercase, "activate", G_CALLBACK(cmd_uppercase), &app);
    g_signal_connect(edit_lowercase, "activate", G_CALLBACK(cmd_lowercase), &app);
    
    g_signal_connect(edit_comment_block, "activate", G_CALLBACK(cmd_block_comment), &app);
    g_signal_connect(edit_uncomment_block, "activate", G_CALLBACK(cmd_block_uncomment), &app);
    
    g_signal_connect(edit_indent, "activate", G_CALLBACK(cmd_indent), &app);
    g_signal_connect(edit_unindent, "activate", G_CALLBACK(cmd_unindent), &app);
    g_signal_connect(edit_trim, "activate", G_CALLBACK(cmd_trim_trailing), &app);
    g_signal_connect(edit_tabs_to_spaces, "activate", G_CALLBACK(cmd_tabs_to_spaces), &app);
    g_signal_connect(edit_spaces_to_tabs, "activate", G_CALLBACK(cmd_spaces_to_tabs), &app);
    g_signal_connect(edit_sort_asc, "activate", G_CALLBACK(cmd_sort_lines_ascending), &app);
    g_signal_connect(edit_sort_desc, "activate", G_CALLBACK(cmd_sort_lines_descending), &app);
    
    g_signal_connect(search_find, "activate", G_CALLBACK(cmd_find), &app);
    g_signal_connect(search_find_next, "activate", G_CALLBACK(cmd_find_next), &app);
    g_signal_connect(search_find_prev, "activate", G_CALLBACK(cmd_find_previous), &app);
    g_signal_connect(search_replace, "activate", G_CALLBACK(cmd_replace), &app);
    g_signal_connect(search_find_in_files, "activate", G_CALLBACK(cmd_find_in_files), &app);
    g_signal_connect(search_incremental, "activate", G_CALLBACK(cmd_incremental_search), &app);
    g_signal_connect(search_goto, "activate", G_CALLBACK(cmd_goto), &app);
    
    g_signal_connect(bookmark_toggle, "activate", G_CALLBACK(cmd_toggle_bookmark), &app);
    g_signal_connect(bookmark_next, "activate", G_CALLBACK(cmd_next_bookmark), &app);
    g_signal_connect(bookmark_prev, "activate", G_CALLBACK(cmd_previous_bookmark), &app);
    g_signal_connect(bookmark_clear, "activate", G_CALLBACK(cmd_clear_bookmarks), &app);
    
    g_signal_connect(view_word_wrap, "activate", G_CALLBACK(cmd_toggle_word_wrap), &app);
    g_signal_connect(view_line_numbers, "activate", G_CALLBACK(cmd_toggle_line_numbers), &app);
    g_signal_connect(view_whitespace, "activate", G_CALLBACK(cmd_toggle_whitespace), &app);
    g_signal_connect(view_eol, "activate", G_CALLBACK(cmd_toggle_eol), &app);
    g_signal_connect(view_zoom_in, "activate", G_CALLBACK(cmd_zoom_in), &app);
    g_signal_connect(view_zoom_out, "activate", G_CALLBACK(cmd_zoom_out), &app);
    g_signal_connect(view_zoom_restore, "activate", G_CALLBACK(cmd_zoom_restore), &app);
    
    g_signal_connect(view_split_h, "activate", G_CALLBACK(cmd_split_horizontal), &app);
    g_signal_connect(view_split_v, "activate", G_CALLBACK(cmd_split_vertical), &app);
    g_signal_connect(view_unsplit, "activate", G_CALLBACK(cmd_unsplit), &app);
    
    g_signal_connect(view_fold_all, "activate", G_CALLBACK(cmd_fold_all), &app);
    g_signal_connect(view_unfold_all, "activate", G_CALLBACK(cmd_unfold_all), &app);
    g_signal_connect(view_toggle_fold, "activate", G_CALLBACK(cmd_toggle_fold), &app);
    g_signal_connect(view_fullscreen, "activate", G_CALLBACK(cmd_toggle_fullscreen), &app);
    g_signal_connect(view_distraction_free, "activate", G_CALLBACK(cmd_toggle_distraction_free), &app);
    
    g_signal_connect(eol_windows, "activate", G_CALLBACK(cmd_set_eol_windows), &app);
    g_signal_connect(eol_unix, "activate", G_CALLBACK(cmd_set_eol_unix), &app);
    g_signal_connect(eol_mac, "activate", G_CALLBACK(cmd_set_eol_mac), &app);
    g_signal_connect(eol_convert, "activate", G_CALLBACK(cmd_convert_eol), &app);
    g_signal_connect(eol_convert_windows, "activate", G_CALLBACK(cmd_convert_to_windows), &app);
    g_signal_connect(eol_convert_unix, "activate", G_CALLBACK(cmd_convert_to_unix), &app);
    g_signal_connect(eol_convert_mac, "activate", G_CALLBACK(cmd_convert_to_mac), &app);
    
    g_signal_connect(settings_preferences, "activate", G_CALLBACK(cmd_preferences), &app);
    gtk_widget_add_accelerator(settings_preferences, "activate", app.accel_group, GDK_KEY_comma, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    g_signal_connect(macro_start, "activate", G_CALLBACK(cmd_start_macro_recording), &app);
    gtk_widget_add_accelerator(macro_start, "activate", app.accel_group, GDK_KEY_F9, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
    g_signal_connect(macro_stop, "activate", G_CALLBACK(cmd_stop_macro_recording), &app);
    gtk_widget_add_accelerator(macro_stop, "activate", app.accel_group, GDK_KEY_F9, GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(macro_playback, "activate", G_CALLBACK(cmd_playback_macro), &app);
    gtk_widget_add_accelerator(macro_playback, "activate", app.accel_group, GDK_KEY_F10, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
    g_signal_connect(macro_save, "activate", G_CALLBACK(cmd_save_macro), &app);
    g_signal_connect(macro_load, "activate", G_CALLBACK(cmd_load_macro), &app);
    
    g_signal_connect(help_about, "activate", G_CALLBACK(cmd_about), &app);
    
    g_signal_connect(tb_new, "clicked", G_CALLBACK(cmd_new), &app);
    g_signal_connect(tb_open, "clicked", G_CALLBACK(cmd_open), &app);
    g_signal_connect(tb_save, "clicked", G_CALLBACK(cmd_save), &app);

    // Save session on quit
    g_signal_connect(app.window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer data) {
        session_save((AppState*)data);
        gtk_main_quit();
    }), &app);

    // Create initial tab
    create_tab(&app, "");
    
    // Restore session if available (only if no command-line files)
    if (argc <= 1) {
        session_restore(&app);
    } else {
        // Open files from command line
        for (int i = 1; i < argc; i++) {
            string filepath = argv[i];
            // Check if file exists
            std::ifstream test(filepath);
            if (test.good()) {
                test.close();
                // For the first file, use the initial tab
                if (i == 1) {
                    GtkWidget *tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), 0);
                    TabData *td = (TabData*)g_object_get_data(G_OBJECT(tab), "tabdata");
                    if (td) {
                        // Load file into existing tab
                        std::ifstream file(filepath);
                        std::string content((std::istreambuf_iterator<char>(file)),
                                           std::istreambuf_iterator<char>());
                        scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTEXT, 0, (sptr_t)content.c_str());
                        td->filename = filepath;
                        td->modified = false;
                        
                        // Update tab label
                        size_t pos = filepath.find_last_of("/\\");
                        string basename = (pos != string::npos) ? filepath.substr(pos + 1) : filepath;
                        GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
                        if (label) gtk_label_set_text(GTK_LABEL(label), basename.c_str());
                        
                        update_statusbar(&app, td->sci);
                        add_recent_file(&app, filepath);
                    }
                } else {
                    // Create new tab for subsequent files
                    create_tab(&app, filepath);
                    add_recent_file(&app, filepath);
                }
            }
        }
    }

    gtk_widget_show_all(app.window);
    
    // Load preferences and start auto-save timer
    static Preferences prefs;
    prefs.load();
    start_auto_save_timer(&app, &prefs);
    
    gtk_main();
    
    // Cleanup: stop auto-save timer
    stop_auto_save_timer(&app);
    
    return 0;
}
