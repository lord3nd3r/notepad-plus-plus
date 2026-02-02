#include <gtk/gtk.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <vector>
#include <memory>
#include <cstring>
#include "Scintilla.h"
#include "ScintillaWidget.h"
#include "ILexer.h"
#include "Lexilla.h"
#include "access/LexillaAccess.h"

using std::string;

// Forward declarations
struct AppState;
static void update_statusbar_for_scintilla(GtkWidget *sci, AppState *app);
static TabData* get_current_tabdata(AppState *app);

struct AppState {
    GtkWidget *window;
    GtkWidget *notebook;
    GtkWidget *statusbar;
    guint status_context;
    GtkWidget *find_dialog;
    GtkWidget *replace_dialog;
    GtkWidget *goto_dialog;
    GtkAccelGroup *accel_group;
    string last_search;
    bool word_wrap;
};

struct TabData {
    GtkWidget *sci;
    string filename;
    bool modified;
};

static GtkWidget *statusbar = nullptr;
static guint status_context = 0;

static void update_statusbar_for_scintilla(GtkWidget *sci) {
    int len = scintilla_send_message((ScintillaObject*)sci, SCI_GETTEXTLENGTH, 0, 0);
    int pos = scintilla_send_message((ScintillaObject*)sci, SCI_GETCURRENTPOS, 0, 0);
    int line = scintilla_send_message((ScintillaObject*)sci, SCI_LINEFROMPOSITION, pos, 0) + 1;
    int lineCount = scintilla_send_message((ScintillaObject*)sci, SCI_GETLINECOUNT, 0, 0);
    int col = pos - scintilla_send_message((ScintillaObject*)sci, SCI_POSITIONFROMLINE, line-1, 0) + 1;
    char buf[512];
    snprintf(buf, sizeof(buf), "length: %d   lines: %d          Ln: %d   Col: %d   Sel: 0          Dos\\Windows          UTF-8          INS", 
             len, lineCount, line, col);
    gtk_statusbar_pop(GTK_STATUSBAR(statusbar), status_context);
    gtk_statusbar_push(GTK_STATUSBAR(statusbar), status_context, buf);
}

static void sci_notify_cb(GtkWidget *widget, gint code, gpointer notification, gpointer userdata) {
    (void)widget;
    if (!userdata) return;
    GtkNotebook *notebook = GTK_NOTEBOOK(userdata);
    gint page = gtk_notebook_get_current_page(notebook);
    if (page < 0) return;
    GtkWidget *tab = gtk_notebook_get_nth_page(notebook, page);
    TabData *td = (TabData*)g_object_get_data(G_OBJECT(tab), "tabdata");
    if (!td) return;

    if (code == SCN_UPDATEUI) {
        update_statusbar_for_scintilla(td->sci);
    } else if (code == SCN_MODIFIED) {
        td->modified = true;
        // Update tab label
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

static GtkWidget* create_scintilla_widget(GtkNotebook *notebook) {
    GtkWidget *sci = scintilla_object_new();
    gtk_widget_set_hexpand(sci, TRUE);
    gtk_widget_set_vexpand(sci, TRUE);
    // Style defaults
    scintilla_send_message((ScintillaObject*)sci, SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t)"Monospace");
    scintilla_send_message((ScintillaObject*)sci, SCI_STYLESETSIZE, STYLE_DEFAULT, 12);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    scintilla_send_message((ScintillaObject*)sci, SCI_SETMARGINWIDTHN, 0, 40);
    return sci;
}

static GtkWidget* create_tab(GtkNotebook *notebook, const string &filename="") {
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *sci = create_scintilla_widget(notebook);
    gtk_container_add(GTK_CONTAINER(scrolled), sci);

    GtkWidget *label = gtk_label_new(filename.empty() ? "Untitled" : filename.c_str());
    GtkWidget *tab = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(tab), label, FALSE, FALSE, 0);
    gtk_widget_show_all(tab);

    TabData *td = new TabData{sci, filename, false};
    g_object_set_data(G_OBJECT(scrolled), "tabdata", td);
    g_object_set_data(G_OBJECT(scrolled), "labelfwd", label);

    // connect notifications with notebook as userdata so callback can find current tab
    g_signal_connect(sci, SCINTILLA_NOTIFY, G_CALLBACK(sci_notify_cb), notebook);

    // If filename provided, attempt to set lexer for it
    if (!filename.empty()) {
        // Determine extension
        std::string::size_type pos = filename.rfind('.');
        if (pos != std::string::npos && pos + 1 < filename.size()) {
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
            else lang = ext; // try ext directly

            if (!lang.empty()) {
                // Try to make a lexer via LexillaAccess
                if (Lexilla::Load(".")) {
                    Scintilla::ILexer5 *pLexer = Lexilla::MakeLexer(lang);
                    if (pLexer) {
                        scintilla_send_message((ScintillaObject*)sci, SCI_SETILEXER, 0, (sptr_t)pLexer);
                    }
                }
            }
        }
    }

    gtk_notebook_append_page(notebook, scrolled, tab);
    gtk_widget_show_all(scrolled);
    return scrolled;
}

static TabData* get_current_tabdata(GtkNotebook *notebook) {
    gint page = gtk_notebook_get_current_page(notebook);
    if (page < 0) return nullptr;
    GtkWidget *tab = gtk_notebook_get_nth_page(notebook, page);
    return (TabData*)g_object_get_data(G_OBJECT(tab), "tabdata");
}

static void new_tab_trampoline(GtkWidget *w, gpointer ud) {
    (void)w;
    GtkNotebook *nb = GTK_NOTEBOOK(ud);
    create_tab(nb, "");
    gtk_notebook_set_current_page(nb, gtk_notebook_get_n_pages(nb)-1);
}

static void open_file_cb(GtkWidget *widget, gpointer userdata) {
    GtkNotebook *notebook = GTK_NOTEBOOK(userdata);
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open File",
                                                    GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(notebook))),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::ifstream t(filename, std::ios::binary);
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        GtkWidget *tab = create_tab(notebook, filename);
        TabData *td = (TabData*)g_object_get_data(G_OBJECT(tab), "tabdata");
        if (td) {
            scintilla_send_message((ScintillaObject*)td->sci, SCI_SETTEXT, 0, (sptr_t)str.c_str());
            td->filename = filename;
            td->modified = false;
            // update label
            GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
            gtk_label_set_text(GTK_LABEL(label), filename);
        }
        gtk_notebook_set_current_page(notebook, gtk_notebook_get_n_pages(notebook)-1);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void save_file_cb(GtkWidget *widget, gpointer userdata) {
    GtkNotebook *notebook = GTK_NOTEBOOK(userdata);
    TabData *td = get_current_tabdata(notebook);
    if (!td) return;

    if (td->filename.empty()) {
        GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File",
                                                        GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(notebook))),
                                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                                        "_Save", GTK_RESPONSE_ACCEPT,
                                                        NULL);
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            int len = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
            std::string buf(len+1, '\0');
            scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXT, len+1, (sptr_t)&buf[0]);
            std::ofstream out(filename, std::ios::binary);
            out << buf;
            td->filename = filename;
            td->modified = false;
            GtkWidget *tab = gtk_notebook_get_nth_page(notebook, gtk_notebook_get_current_page(notebook));
            GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
            gtk_label_set_text(GTK_LABEL(label), filename);
            g_free(filename);
        }
        gtk_widget_destroy(dialog);
    } else {
        int len = scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXTLENGTH, 0, 0);
        std::string buf(len+1, '\0');
        scintilla_send_message((ScintillaObject*)td->sci, SCI_GETTEXT, len+1, (sptr_t)&buf[0]);
        std::ofstream out(td->filename, std::ios::binary);
        out << buf;
        td->modified = false;
        GtkWidget *tab = gtk_notebook_get_nth_page(notebook, gtk_notebook_get_current_page(notebook));
        GtkWidget *label = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "labelfwd");
        gtk_label_set_text(GTK_LABEL(label), td->filename.c_str());
    }
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 700);
    gtk_window_set_title(GTK_WINDOW(window), "GTK Scintilla Prototype");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Menu bar
    GtkWidget *menubar = gtk_menu_bar_new();
    
    // File menu
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    GtkWidget *file_new = gtk_menu_item_new_with_label("New");
    GtkWidget *file_open = gtk_menu_item_new_with_label("Open");
    GtkWidget *file_save = gtk_menu_item_new_with_label("Save");
    GtkWidget *file_quit = gtk_menu_item_new_with_label("Quit");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_new);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_open);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_save);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_quit);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);
    
    // Edit menu
    GtkWidget *edit_menu = gtk_menu_new();
    GtkWidget *edit_item = gtk_menu_item_new_with_label("Edit");
    GtkWidget *edit_undo = gtk_menu_item_new_with_label("Undo");
    GtkWidget *edit_redo = gtk_menu_item_new_with_label("Redo");
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_undo);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_redo);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_item), edit_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit_item);
    
    // View menu
    GtkWidget *view_menu = gtk_menu_new();
    GtkWidget *view_item = gtk_menu_item_new_with_label("View");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_item), view_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), view_item);
    
    // Help menu
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_item = gtk_menu_item_new_with_label("Help");
    GtkWidget *help_about = gtk_menu_item_new_with_label("About");
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

    // Notebook for tabs
    GtkWidget *notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

    // Status bar
    statusbar = gtk_statusbar_new();
    status_context = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "status");
    gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);

    // Connect toolbar actions
    g_signal_connect(tb_open, "clicked", G_CALLBACK(open_file_cb), notebook);
    g_signal_connect(tb_save, "clicked", G_CALLBACK(save_file_cb), notebook);
    g_signal_connect(tb_new, "clicked", G_CALLBACK(new_tab_trampoline), notebook);
    
    // Connect menu actions
    g_signal_connect(file_new, "activate", G_CALLBACK(new_tab_trampoline), notebook);
    g_signal_connect(file_open, "activate", G_CALLBACK(open_file_cb), notebook);
    g_signal_connect(file_save, "activate", G_CALLBACK(save_file_cb), notebook);
    g_signal_connect(file_quit, "activate", G_CALLBACK(gtk_main_quit), NULL);

    // Create initial tab
    create_tab(GTK_NOTEBOOK(notebook), "");

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
