#include <gtk/gtk.h>
#include "../scintilla/include/ScintillaWidget.h"
#include "../scintilla/include/ScintillaTypes.h"

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(win), 900, 700);
    gtk_window_set_title(GTK_WINDOW(win), "Notepad++ - GTK Prototype");

    GtkWidget *sci = scintilla_object_new();
    gtk_container_add(GTK_CONTAINER(win), sci);

    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}
