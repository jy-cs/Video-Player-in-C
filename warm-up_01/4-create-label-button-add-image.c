#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *quiz_screen, *score_screen, *grid;
  GtkWidget *ltitle, *lque_no, *lque, *lopta, *loptb, *loptc, *loptd;
  GtkWidget *bopta, *boptb, *boptc, *boptd, *bque_img;
  GtkWidget *que_image;
  
  // Source - https://docs.gtk.org/gtk3/method.Widget.override_background_color.html
  // Source - https://stackoverflow.com/questions/26504651/gtk-3-0-change-fontcolor-of-statusbar
  GdkRGBA red_color, blue_color, green_color,white_color;
  red_color = (GdkRGBA){1, 0, 0, 1};
  green_color = (GdkRGBA){0, 1, 0, 1};
  blue_color = (GdkRGBA){0, 0, 1, 0.8};
  white_color = (GdkRGBA){1, 1, 1, 1};

  quiz_screen = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(quiz_screen), "Quiz Time");
  gtk_window_set_default_size(GTK_WINDOW(quiz_screen), 1000, 600);
  gtk_window_move(GTK_WINDOW(quiz_screen), 200, 300);

  // Declarations
  //  Source - https://docs.gtk.org/gtk4/class.Grid.html
  //  Source - https://stackoverflow.com/questions/26429173/getting-a-gtk-label-to-dislay-an-int-in-c
  //  Source - https://docs.gtk.org/gtk3/class.image.html
  ltitle = gtk_label_new("Quiz Time");
  lque_no = gtk_label_new("1.");
  lque = gtk_label_new("Sample Question?");
  lopta = gtk_label_new("A.");
  loptb = gtk_label_new("B.");
  loptc = gtk_label_new("C.");
  loptd = gtk_label_new("D.");
  grid = gtk_grid_new();
  bopta = gtk_button_new_with_label("Option A");
  boptb = gtk_button_new_with_label("Option B");
  boptc = gtk_button_new_with_label("Option C");
  boptd = gtk_button_new_with_label("Option D");
  bque_img = gtk_button_new();
  // que_image = gtk_image_new_from_file("icon-2.png");
  que_image = gtk_image_new_from_file("test.jpeg");
  // Source - https://docs.gtk.org/gtk3/method.Button.set_image.html
  gtk_button_set_image(GTK_BUTTON(bque_img),que_image);

  // Coloring the Elements
  gtk_widget_override_background_color(quiz_screen, GTK_STATE_FLAG_NORMAL, &red_color);
  gtk_widget_override_background_color(bopta, GTK_STATE_FLAG_NORMAL, &blue_color);
  gtk_widget_override_background_color(boptb, GTK_STATE_FLAG_NORMAL, &blue_color);
  gtk_widget_override_background_color(boptc, GTK_STATE_FLAG_NORMAL, &blue_color);
  gtk_widget_override_background_color(boptd, GTK_STATE_FLAG_NORMAL, &blue_color);

  // Set Properties
  gtk_container_set_border_width(GTK_CONTAINER(quiz_screen), 20);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 112);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 160);
  gtk_container_add(GTK_CONTAINER(quiz_screen), grid);

  // Fill the grid with shit             (x, y, v, h)
  gtk_grid_attach(GTK_GRID(grid), ltitle, 0, 0, 7, 1);
  gtk_grid_attach(GTK_GRID(grid), lque_no, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), lque, 1, 1, 2, 2);
  gtk_grid_attach(GTK_GRID(grid), bque_img, 3, 1, 4, 2);
  gtk_grid_attach(GTK_GRID(grid), lopta, 0, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), loptb, 4, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), loptc, 0, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), loptd, 4, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), bopta, 1, 3, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), boptb, 5, 3, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), boptc, 1, 4, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), boptd, 5, 4, 2, 1);

  gtk_widget_show_all(quiz_screen);

  score_screen = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(score_screen), "Score Board");
  gtk_window_set_default_size(GTK_WINDOW(score_screen), 300, 600);
  gtk_window_move(GTK_WINDOW(score_screen), 1250, 300);
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