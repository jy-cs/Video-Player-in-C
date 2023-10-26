#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

// Variables for Score Screen
const int itotal_num_que = 6;
static int icorrect_num = 0, ifail_num = 0, ique_num = 0;
char total[5], corr[5], fail[5], perc[10], cque_num[5];
static char copta = 'a', coptb = 3, coptc = 'c', coptd = 'd';

struct Ques
{
  char que[100];
  char que_img[100];
  char opta[50];
  char optb[50];
  char optc[50];
  char optd[50];
  char ans;
  char diff;
} q[itotal_num_que] = {
    {"Identify the Image.", "que-1.png", "Vehical", "Animal", "Bird", "Number", 'd', 'e'},
    {"If x=27, Select the correct output.", "que-2.png", "52", "56", "54", "58", 'c', 'e'},
    {"Compute the Image.", "que-3.jpeg", "0", "2", "3", "1", 3, 'e'},
    {"Compute and Select the correct option.", "que-4.png", "Next", "Matched", "ERROR", "NULL", 'a', 'm'},
    {"Compute and Select the correct option.", "que-5.png", "ERROR", "1", "2", "NULL", 'c', 'm'},
    {"Compute and Select the correct option.", "que-6.jpeg", "3.356", "3.368", "3.388", "3.386", 'd', 'h'},
};

// Window
GtkWidget *quiz_screen, *score_screen, *final_screen;
// Grid
GtkWidget *qz_grid, *sb_grid;
// Labels
GtkWidget *qz_ltitle, *qz_lque_no, *qz_lque, *qz_lopta, *qz_loptb, *qz_loptc, *qz_loptd, *sb_ltitle, *sb_llab_total, *sb_llab_corr, *sb_llab_fail, *sb_llab_perc, *sb_lnum_tot, *sb_lnum_corr, *sb_lnum_fail, *sb_lnum_perc;
// Buttons
GtkWidget *qz_bopta, *qz_boptb, *qz_boptc, *qz_boptd, *qz_bque_img, *sb_bexit;
// Image
GtkWidget *qz_ique_image;
// Icon Variabel
GdkPixbuf *icon;

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

static void changeColor(GtkWidget *widget, int color)
{
  // Color Variables
  GdkRGBA red_color = (GdkRGBA){1, 0, 0, 1}, blue_color = (GdkRGBA){0, 0, 1, 1}, green_color = (GdkRGBA){0, 1, 0, 0.8}, white_color = (GdkRGBA){1, 1, 1, 1}, yellow_color = (GdkRGBA){1, 1, 0, 1};
  switch (color)
  {
  case 1:
    gtk_widget_override_background_color(widget, GTK_STATE_FLAG_NORMAL, &red_color);
    break;
  case 2:
    gtk_widget_override_background_color(widget, GTK_STATE_FLAG_NORMAL, &green_color);
    break;
  case 3:
    gtk_widget_override_background_color(widget, GTK_STATE_FLAG_NORMAL, &blue_color);
    break;
  case 4:
    gtk_widget_override_background_color(widget, GTK_STATE_FLAG_NORMAL, &yellow_color);
    break;
  case 5:
    gtk_widget_override_background_color(widget, GTK_STATE_FLAG_NORMAL, &white_color);
    break;
  }
}

static void updateScreen()
{
  // Quiz Screen Modification Start
  sprintf(cque_num, "%d", ique_num + 1);
  gtk_label_set_label(GTK_LABEL(qz_lque_no), cque_num);
  gtk_label_set_label(GTK_LABEL(qz_lque), q[ique_num].que);
  gtk_button_set_label(GTK_BUTTON(qz_bopta), q[ique_num].opta);
  gtk_button_set_label(GTK_BUTTON(qz_boptb), q[ique_num].optb);
  gtk_button_set_label(GTK_BUTTON(qz_boptc), q[ique_num].optc);
  gtk_button_set_label(GTK_BUTTON(qz_boptd), q[ique_num].optd);
  qz_ique_image = gtk_image_new_from_file(q[ique_num].que_img);
  gtk_button_set_image(GTK_BUTTON(qz_bque_img), qz_ique_image);
  switch (q[ique_num].diff)
  {
  case 'e':
    changeColor(quiz_screen, 2);
    break;

  case 'm':
    changeColor(quiz_screen, 3);
    break;

  case 'h':
    changeColor(quiz_screen, 1);
    break;
  }
  changeColor(qz_bque_img, 5);
  changeColor(qz_bopta, 5);
  changeColor(qz_boptb, 5);
  changeColor(qz_boptc, 5);
  changeColor(qz_boptd, 5);
  // Quiz Screen Modification Ends

  // Score Screen Modification Start
  sprintf(corr, "%d", icorrect_num);
  sprintf(fail, "%d", ifail_num);
  sprintf(perc, "%.2f", (float)icorrect_num * 100 / (ique_num));
  strcat(perc, " \%");
  gtk_label_set_label(GTK_LABEL(sb_lnum_corr), corr);
  gtk_label_set_label(GTK_LABEL(sb_lnum_fail), fail);
  gtk_label_set_label(GTK_LABEL(sb_lnum_perc), perc);
  // Score Screen Modification Ends
}

static void close_program()
{
  gtk_window_close(GTK_WINDOW(score_screen));
}

static void button_press(GtkWidget *widget, gpointer data)
{
  if (*(char *)data == q[ique_num++].ans)
  {
    icorrect_num++;
    changeColor(score_screen, 2);
  }
  else
  {
    ifail_num++;
    changeColor(score_screen, 1);
  }
  if (ique_num == itotal_num_que)
  {
    gtk_window_close(GTK_WINDOW(quiz_screen));
    sprintf(corr, "%d", icorrect_num);
    sprintf(fail, "%d", ifail_num);
    sprintf(perc, "%.2f", (float)icorrect_num * 100 / (ique_num));
    strcat(perc, " \%");
    gtk_label_set_label(GTK_LABEL(sb_lnum_corr), corr);
    gtk_label_set_label(GTK_LABEL(sb_lnum_fail), fail);
    gtk_label_set_label(GTK_LABEL(sb_lnum_perc), perc);
    if (icorrect_num * 100 / (ique_num) >= 60)
    {
      gtk_label_set_label(GTK_LABEL(sb_ltitle), "Congratulations!! You passed the Quiz.");
    }
    else
    {
      gtk_label_set_label(GTK_LABEL(sb_ltitle), "Sorry!! Fail - Please try again.");
    }
    gtk_grid_insert_row(GTK_GRID(sb_grid), 5);
    sb_bexit = gtk_button_new_with_label("Exit");
    gtk_grid_attach(GTK_GRID(sb_grid), sb_bexit, 0, 5, 4, 1);
    changeColor(sb_bexit, 4);
    gtk_window_move(GTK_WINDOW(score_screen), 720, 250);
    gtk_widget_show_all(score_screen);
    g_signal_connect(G_OBJECT(sb_bexit), "clicked", G_CALLBACK(close_program), NULL);
  }
  else
  {
    updateScreen();
  }
}

static void activate(GtkApplication *app, gpointer user_data)
{
  icon = create_pixbuf("icon.png");
  //   ---------------- Quiz Screen Start -----------------
  quiz_screen = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(quiz_screen), "Quiz Time");
  gtk_window_set_default_size(GTK_WINDOW(quiz_screen), 1000, 600);
  gtk_window_move(GTK_WINDOW(quiz_screen), 200, 200);

  qz_grid = gtk_grid_new();
  gtk_container_set_border_width(GTK_CONTAINER(quiz_screen), 20);
  gtk_grid_set_row_spacing(GTK_GRID(qz_grid), 112);
  gtk_grid_set_column_spacing(GTK_GRID(qz_grid), 160);
  gtk_container_add(GTK_CONTAINER(quiz_screen), qz_grid);

  qz_ltitle = gtk_label_new("Quiz Time");
  qz_lque_no = gtk_label_new("");
  qz_lque = gtk_label_new("");
  qz_lopta = gtk_label_new("A.");
  qz_loptb = gtk_label_new("B.");
  qz_loptc = gtk_label_new("C.");
  qz_loptd = gtk_label_new("D.");
  qz_bopta = gtk_button_new();
  qz_boptb = gtk_button_new();
  qz_boptc = gtk_button_new();
  qz_boptd = gtk_button_new();
  qz_bque_img = gtk_button_new();

  gtk_grid_attach(GTK_GRID(qz_grid), qz_ltitle, 0, 0, 7, 1);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_lque_no, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_lque, 1, 1, 2, 2);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_bque_img, 3, 1, 4, 2);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_lopta, 0, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_loptb, 4, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_loptc, 0, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_loptd, 4, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_bopta, 1, 3, 2, 1);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_boptb, 5, 3, 2, 1);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_boptc, 1, 4, 2, 1);
  gtk_grid_attach(GTK_GRID(qz_grid), qz_boptd, 5, 4, 2, 1);
  //   ---------------- Quiz Screen End -----------------

  //   ---------------- Score Screen Start -----------------
  score_screen = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(score_screen), "Score Board");
  gtk_window_set_default_size(GTK_WINDOW(score_screen), 300, 600);
  gtk_window_move(GTK_WINDOW(score_screen), 1300, 200);
  changeColor(score_screen, 4);

  sb_grid = gtk_grid_new();
  gtk_container_set_border_width(GTK_CONTAINER(score_screen), 45);
  gtk_grid_set_row_spacing(GTK_GRID(sb_grid), 88);
  gtk_grid_set_column_spacing(GTK_GRID(sb_grid), 50);
  gtk_container_add(GTK_CONTAINER(score_screen), sb_grid);

  sb_ltitle = gtk_label_new("Screen Board");
  sb_llab_total = gtk_label_new("Totol Number \n of questions  :");
  sb_llab_corr = gtk_label_new("Correctness    :");
  sb_llab_fail = gtk_label_new("Failures             :");
  sb_llab_perc = gtk_label_new("Result               :");
  sprintf(total, "%d", itotal_num_que);
  sb_lnum_tot = gtk_label_new(total);
  sb_lnum_corr = gtk_label_new("");
  sb_lnum_fail = gtk_label_new("");
  sb_lnum_perc = gtk_label_new("");

  gtk_grid_attach(GTK_GRID(sb_grid), sb_ltitle, 0, 0, 3, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_llab_total, 0, 1, 2, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_lnum_tot, 2, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_llab_corr, 0, 2, 2, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_lnum_corr, 2, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_llab_fail, 0, 3, 2, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_lnum_fail, 2, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_llab_perc, 0, 4, 2, 1);
  gtk_grid_attach(GTK_GRID(sb_grid), sb_lnum_perc, 2, 4, 1, 1);
  //   ---------------- Score Screen End -----------------

  updateScreen();
  gtk_window_set_icon(GTK_WINDOW(quiz_screen), icon);
  gtk_window_set_icon(GTK_WINDOW(score_screen), icon);
  gtk_widget_show_all(score_screen);
  gtk_widget_show_all(quiz_screen);

  //   ---------------- Callback Function Start -----------------
  g_signal_connect(G_OBJECT(qz_bopta), "clicked", G_CALLBACK(button_press), &copta);
  g_signal_connect(G_OBJECT(qz_boptb), "clicked", G_CALLBACK(button_press), &coptb);
  g_signal_connect(G_OBJECT(qz_boptc), "clicked", G_CALLBACK(button_press), &coptc);
  g_signal_connect(G_OBJECT(qz_boptd), "clicked", G_CALLBACK(button_press), &coptd);
  //   ---------------- Callback Function End -----------------
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