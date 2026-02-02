#include <gtk/gtk.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include "Scintilla.h"
#include "ScintillaWidget.h"
#include "ILexer.h"
#include "Lexilla.h"
#include "access/LexillaAccess.h"

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
};

// Forward declarations
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

    if (!lang.empty() && Lexilla::Load(".")) {
        Scintilla::ILexer5 *pLexer = Lexilla::MakeLexer(lang);
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

static void cmd_undo(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) scintilla_send_message((ScintillaObject*)td->sci, SCI_UNDO, 0, 0);
}

static void cmd_redo(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) scintilla_send_message((ScintillaObject*)td->sci, SCI_REDO, 0, 0);
}

static void cmd_cut(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) scintilla_send_message((ScintillaObject*)td->sci, SCI_CUT, 0, 0);
}

static void cmd_copy(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) scintilla_send_message((ScintillaObject*)td->sci, SCI_COPY, 0, 0);
}

static void cmd_paste(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) scintilla_send_message((ScintillaObject*)td->sci, SCI_PASTE, 0, 0);
}

static void cmd_delete(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) scintilla_send_message((ScintillaObject*)td->sci, SCI_CLEAR, 0, 0);
}

static void cmd_selectall(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (td) scintilla_send_message((ScintillaObject*)td->sci, SCI_SELECTALL, 0, 0);
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
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new("Find what:"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), entry, FALSE, FALSE, 5);
    gtk_widget_show_all(content);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
        app->last_search = text;
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
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new("Find what:"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), find_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new("Replace with:"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), replace_entry, FALSE, FALSE, 5);
    gtk_widget_show_all(content);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == 1 || response == 2) {
        const char *find_text = gtk_entry_get_text(GTK_ENTRY(find_entry));
        const char *replace_text = gtk_entry_get_text(GTK_ENTRY(replace_entry));
        
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

// Line operations
static void cmd_line_duplicate(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEDUPLICATE, 0, 0);
}

static void cmd_line_delete(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    if (!td) return;
    scintilla_send_message((ScintillaObject*)td->sci, SCI_LINEDELETE, 0, 0);
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

    app.accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(app.window), app.accel_group);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app.window), vbox);

    // Menu bar
    GtkWidget *menubar = gtk_menu_bar_new();
    
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
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_trim);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_item), edit_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit_item);
    
    // Search menu
    GtkWidget *search_menu = gtk_menu_new();
    GtkWidget *search_item = gtk_menu_item_new_with_mnemonic("_Search");
    GtkWidget *search_find = gtk_menu_item_new_with_mnemonic("_Find...");
    GtkWidget *search_find_next = gtk_menu_item_new_with_mnemonic("Find _Next");
    GtkWidget *search_find_prev = gtk_menu_item_new_with_mnemonic("Find _Previous");
    GtkWidget *search_replace = gtk_menu_item_new_with_mnemonic("_Replace...");
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
    
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(eol_unix), TRUE);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), eol_windows);
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), eol_unix);
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), eol_mac);
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(eol_submenu), eol_convert);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(eol_item), eol_submenu);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(encoding_menu), eol_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(encoding_item), encoding_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), encoding_item);
    
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
    
    g_signal_connect(search_find, "activate", G_CALLBACK(cmd_find), &app);
    g_signal_connect(search_find_next, "activate", G_CALLBACK(cmd_find_next), &app);
    g_signal_connect(search_find_prev, "activate", G_CALLBACK(cmd_find_previous), &app);
    g_signal_connect(search_replace, "activate", G_CALLBACK(cmd_replace), &app);
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
    
    g_signal_connect(eol_windows, "activate", G_CALLBACK(cmd_set_eol_windows), &app);
    g_signal_connect(eol_unix, "activate", G_CALLBACK(cmd_set_eol_unix), &app);
    g_signal_connect(eol_mac, "activate", G_CALLBACK(cmd_set_eol_mac), &app);
    g_signal_connect(eol_convert, "activate", G_CALLBACK(cmd_convert_eol), &app);
    
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
    
    // Restore session if available
    session_restore(&app);

    gtk_widget_show_all(app.window);
    gtk_main();
    return 0;
}
