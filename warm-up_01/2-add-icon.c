#include <gtk/gtk.h>

// Not Working in MacOS
GdkPixbuf *create_pixbuf(const gchar *filename)
{

    GdkPixbuf *pixbuf;
    GError *error = NULL;
    pixbuf = gdk_pixbuf_new_from_file(filename, &error);

    if (!pixbuf)
    {
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
    }

    return pixbuf;
}

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *quiz_screen;
    GdkPixbuf *icon;

    quiz_screen = gtk_application_window_new(app);

    // Naming the window of the Application
    gtk_window_set_title(GTK_WINDOW(quiz_screen), "Quiz Time");

    // Setting the window application size
    gtk_window_set_default_size(GTK_WINDOW(quiz_screen), 800, 400);

    // Set window position to center
    // Center Position
    // Source - https://docs.gtk.org/gtk3/enum.WindowPosition.html
    gtk_window_set_position(GTK_WINDOW(quiz_screen), GTK_WIN_POS_CENTER);

    // Icon Added to the window - Not Working in MacOS
    // Source - https://zetcode.com/gui/gtk2/firstprograms/
    icon = create_pixbuf("icon-2.png");
    gtk_window_set_icon(GTK_WINDOW(quiz_screen), icon);
    printf("%s", gtk_window_get_icon_name(GTK_WINDOW(quiz_screen)));

    // Show the Window
    gtk_widget_show_all(quiz_screen);
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}