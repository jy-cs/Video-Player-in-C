#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>


void updateLabel(GtkLabel *sum, int num)
{
    gchar *display;
    display = g_strdup_printf("%d", num);         //convert num to str
    gtk_label_set_text (GTK_LABEL(sum), display); //set label to "display"
    g_free(display);                              //free display
}

// bopta_check for now only add 1 on both sb_num_corr and sb_num_fail labels
void bopta_check(GtkButton *bopta, GtkLabel **score_arr)
{
  g_print("Enter: bopta_check\n"); // testing for debug <------------
  GtkLabel *corr = *score_arr;
  GtkLabel *fail = *(score_arr + 1);
  g_print("GTK_IS_LABEL (corr) = %d\n", GTK_IS_LABEL (corr));// testing for debug <---
  g_print("GTK_IS_LABEL (fail) = %d\n", GTK_IS_LABEL (fail));// testing for debug <---
  GtkLabel corr_test = *corr;// testing for debug <---
  GtkLabel fail_test = *fail;// testing for debug <---
  //GTK_IS_LABEL (corr_test); // testing for dubug <--
  int num_corr = atoi(gtk_label_get_text(corr));
  int num_fail = atoi(gtk_label_get_text(fail));
  num_corr += 1;
  num_fail += 1;
  g_print("Button Option A is clicked: Correct + Fail\n");// prompt for progress
  updateLabel(GTK_LABEL(corr),num_corr);
  updateLabel(GTK_LABEL(fail),num_fail);
  g_print("Enter: bopta_check\n");// testing for debug <---
}

static void activate(GtkApplication *app, gpointer user_data)
{
  // >>>>>>>>>>>>> Quiz Screen starts >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  
  // GtkWidget *quiz_screen, *score_screen, *grid; !!!!!!! we need to delete score_screen
  GtkWidget *quiz_screen, *grid;
  GtkWidget *ltitle, *lque_no, *lque, *lopta, *loptb, *loptc, *loptd;
  GtkWidget *bopta, *boptb, *boptc, *boptd; // GtkWidget button option A here <-----------
  GtkWidget *que_image;
  GtkWidget *que_image2;
  
  GdkRGBA red_color, blue_color, green_color,white_color;
  red_color = (GdkRGBA){1, 0, 0, 1};
  green_color = (GdkRGBA){0, 1, 0, 1};
  blue_color = (GdkRGBA){0, 0, 1, 1};
  white_color = (GdkRGBA){1, 1, 1, 1};

  quiz_screen = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(quiz_screen), "Quiz Time");
  gtk_window_set_default_size(GTK_WINDOW(quiz_screen), 1000, 600);
  gtk_window_move(GTK_WINDOW(quiz_screen), 200, 300);

  // Declarations
  ltitle = gtk_label_new("Quiz Time");
  lque_no = gtk_label_new("1.");
  lque = gtk_label_new("Sample Question?");
  lopta = gtk_label_new("A.");
  loptb = gtk_label_new("B.");
  loptc = gtk_label_new("C.");
  loptd = gtk_label_new("D.");
  grid = gtk_grid_new();
  bopta = gtk_button_new_with_label("Option A"); // button option A here <-----------
  boptb = gtk_button_new_with_label("Option A");
  boptc = gtk_button_new_with_label("Option A");
  boptd = gtk_button_new_with_label("Option A");
  que_image = gtk_button_new_with_label("Image");
  que_image2 = gtk_image_new_from_file("icon.png");

  gtk_widget_override_background_color(quiz_screen, GTK_STATE_FLAG_NORMAL, &red_color);

  // Set Properties
  gtk_container_set_border_width(GTK_CONTAINER(quiz_screen), 20);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 112);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 160);
  gtk_container_add(GTK_CONTAINER(quiz_screen), grid);

  // Fill the grid with shit             (x, y, v, h)
  gtk_grid_attach(GTK_GRID(grid), ltitle, 0, 0, 7, 1);
  gtk_grid_attach(GTK_GRID(grid), lque_no, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), lque, 1, 1, 2, 2);
  gtk_grid_attach(GTK_GRID(grid), que_image, 3, 1, 4, 2);
  gtk_grid_attach(GTK_GRID(grid), lopta, 0, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), loptb, 4, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), loptc, 0, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), loptd, 4, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), bopta, 1, 3, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), boptb, 5, 3, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), boptc, 1, 4, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), boptd, 5, 4, 2, 1);
  

  gtk_widget_show_all(quiz_screen);

  // <<<<<<<<<<<<< Quiz Screen ends <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

  // >>>>>>>>>>>>> Score Screen starts >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

  // Score Screen(Time: Sat.)
  GtkWidget *score_screen, *sb_grid;

  score_screen = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(score_screen), "Score Board");
  gtk_window_set_default_size(GTK_WINDOW(score_screen), 300, 600);
  gtk_window_move(GTK_WINDOW(score_screen),1250,300);

  // Perc represents a percentage of (num / den)
  struct Perc{
    int num;
    int den;
  };

  // Variables for scores
  const int total_num = -120;
  int correct_num, fail_num;
  struct Perc res = {correct_num, total_num};
  
  // Create score board(sb) labels(lab)
  GtkWidget *sb_title;
  GtkWidget *sb_lab_total, *sb_lab_corr, *sb_lab_fail, *sb_lab_perc;
  GtkWidget *sb_num_tot, *sb_num_corr, *sb_num_fail, *sb_num_perc;

  // Integer percentage to Strings
  char total[5], corr[5], fail[5], perc[5];
  sprintf(total, "%d", total_num);

  // Add widgets on scoure board(sb for abbr.)
  grid = gtk_grid_new ();
  sb_title = gtk_label_new ("Your Current Score");
  sb_lab_total = gtk_label_new ("Totol Number \n of questions  :");
  sb_lab_corr = gtk_label_new ("Correctness    :");
  sb_lab_fail = gtk_label_new ("Failures             :");
  sb_lab_perc = gtk_label_new ("Result               :");
  sb_num_tot = gtk_label_new (total);
  sb_num_corr = gtk_label_new ("123");
  sb_num_fail = gtk_label_new ("124");
  sb_num_perc = gtk_label_new ("testing#125");
  
  sb_grid = gtk_grid_new();

  // Set properties
  gtk_container_set_border_width (GTK_CONTAINER(score_screen), 45);
  gtk_grid_set_row_spacing       (GTK_GRID(sb_grid), 88);
  gtk_grid_set_column_spacing    (GTK_GRID(sb_grid), 50);
  gtk_container_add              (GTK_CONTAINER(score_screen), sb_grid);

  // Fill the grid with coordinates               (x, y, h, v)
  gtk_grid_attach(GTK_GRID(sb_grid), sb_title,     0, 0, 3, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_lab_total, 0, 1, 2, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_num_tot,   2, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_lab_corr,  0, 2, 2, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_num_corr,  2, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_lab_fail,  0, 3, 2, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_num_fail,  2, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_lab_perc,  0, 4, 2, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_num_perc,  2, 4, 1, 1);
  
  // Sep 19 Mon - Cross button event handling
  //g_signal_connect(score_screen, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  
  gtk_widget_show_all(score_screen);
  
  // Sep 19 Mon - Add clicked actions on Button Option A to control score board
  GtkLabel *score_arr[2]; // for now, score_arr = {NUM_CORRECTNESS, NUM_FAIL} 
  score_arr[0] = sb_num_corr;
  score_arr[1] = sb_num_fail;
  g_signal_connect(G_OBJECT(bopta), "clicked", G_CALLBACK(bopta_check), score_arr);

  // <<<<<<<<<<<<< Score Screen ends <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  
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
