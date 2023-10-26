#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *quiz_screen,*score_screen;

  quiz_screen = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(quiz_screen), "Quiz Time");
  gtk_window_set_default_size(GTK_WINDOW(quiz_screen), 1000, 600);
//   gtk_window_set_position(GTK_WINDOW(quiz_screen), GTK_WIN_POS_CENTER);
  gtk_window_move(GTK_WINDOW(quiz_screen),200,300);
  gtk_widget_show_all(quiz_screen);

  score_screen = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(score_screen), "Score Board");
  gtk_window_set_default_size(GTK_WINDOW(score_screen), 300, 600);
//   gtk_window_set_gravity(GTK_WINDOW(score_screen), GDK_GRAVITY_NORTH_EAST);
//   Source - https://docs.gtk.org/gtk3/method.Window.move.html
// Source - https://stackoverflow.com/questions/17908584/how-to-set-the-gtk-window-to-the-center-of-screen
  gtk_window_move(GTK_WINDOW(score_screen),1250,300);
  gtk_widget_show_all(score_screen);

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