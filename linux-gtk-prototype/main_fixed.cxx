#include <gtk/gtk.h>
#include <fstream>
#include <string>
#include "../scintilla/include/Scintilla.h"
#include "../scintilla/include/ScintillaWidget.h"

static void open_file_cb(GtkMenuItem *item, gpointer user_data) {
    GtkWidget *sci_w = GTK_WIDGET(user_data);
    GtkWidget *toplevel = gtk_widget_get_toplevel(sci_w);
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open File",
        GTK_WINDOW(toplevel), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::ifstream in(filename, std::ios::binary);
        std::string contents;
        if (in) {
            in.seekg(0, std::ios::end);
            contents.resize((size_t)in.tellg());
            in.seekg(0);
            in.read(&contents[0], contents.size());
        }
        if (!contents.empty()) {
            scintilla_send_message((ScintillaObject*)sci_w, SCI_SETTEXT, 0, (sptr_t)contents.c_str());
        } else {
            scintilla_send_message((ScintillaObject*)sci_w, SCI_SETTEXT, 0, (sptr_t)"");
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void save_file_cb(GtkMenuItem *item, gpointer user_data) {
    GtkWidget *sci_w = GTK_WIDGET(user_data);
    GtkWidget *toplevel = gtk_widget_get_toplevel(sci_w);
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File",
        GTK_WINDOW(toplevel), GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        int length = (int)scintilla_send_message((ScintillaObject*)sci_w, SCI_GETTEXTLENGTH, 0, 0);
        std::string buf;
        buf.resize(length + 1);
        scintilla_send_message((ScintillaObject*)sci_w, SCI_GETTEXT, (uptr_t)(length + 1), (sptr_t)buf.data());
        std::ofstream out(filename, std::ios::binary);
        if (out) out.write(buf.data(), length);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(win), 900, 700);
    gtk_window_set_title(GTK_WINDOW(win), "Notepad++ - GTK Prototype");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *filemenu = gtk_menu_new();
    GtkWidget *file = gtk_menu_item_new_with_label("File");
    GtkWidget *open = gtk_menu_item_new_with_label("Open");
    GtkWidget *save = gtk_menu_item_new_with_label("Save");
    GtkWidget *quit = gtk_menu_item_new_with_label("Quit");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), save);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    GtkWidget *sci = scintilla_object_new();
    gtk_box_pack_start(GTK_BOX(vbox), sci, TRUE, TRUE, 0);

    g_signal_connect(open, "activate", G_CALLBACK(open_file_cb), sci);
    g_signal_connect(save, "activate", G_CALLBACK(save_file_cb), sci);
    g_signal_connect_swapped(quit, "activate", G_CALLBACK(gtk_widget_destroy), win);

    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}
