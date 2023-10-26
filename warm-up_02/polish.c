#include <cairo.h>
#include <gtk/gtk.h>
#include <math.h>

// Constant for max computations allowed on a image
#define MAX_COMPUTATION 100
// Current Modes for computing specific functions
#define NO_MODE 0
#define SELECT_COLOR 1
#define PAINT 2
#define LINE 3
#define RHOMBUS 4
#define CIRCLE 5

// Structure for storing the image information
struct Info
{
    cairo_surface_t *surface;
    int width;
    int height;
    int stride;
} image_info;

// Stucture for storing mouse click information
struct mouse_details
{
    int mode;
    int x;
    int y;
    float r;
    float g;
    float b;
    float a;
} mouse_details[MAX_COMPUTATION];

// Current Selected Color Values Save
static float selected_color[4] = {0, 0, 0, 0};
// Mode Values to compare with the current mode
static char mode1 = 's', mode2 = 'p', mode3 = 'l', mode4 = 'r', mode5 = 'c', mode6 = 'u';
static int count = -1, current_mode;
// GTK Window (Two Window Objects)
GtkWidget *screen, *tool_screen;

// Draw a Slant Line
static void draw_line(cairo_t *cr, int x, int y, float r, float g, float b)
{
    cairo_set_source_rgb(cr, r, g, b);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x + 50, y + 50);
    cairo_stroke(cr);
}

// Draw a Rhombus
static void draw_rhombus(cairo_t *cr, int x, int y, float r, float g, float b)
{
    cairo_set_source_rgb(cr, r, g, b);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, x, y, 50, 30);
    cairo_stroke(cr);
}

// Draw a Circle
static void draw_circle(cairo_t *cr, int x, int y, float r, float g, float b)
{
    cairo_set_source_rgb(cr, r, g, b);
    cairo_arc(cr, x, y, 50, 0, (2.0) * M_PI);
    cairo_stroke(cr);
}

// Modify/Polish a Pixel Value
static void modify_pixel(cairo_t *cr, int x, int y, float r, float g, float b)
{
    cairo_set_source_rgb(cr, r, g, b);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x + 1, y + 1);
    cairo_stroke(cr);
}

// Event method that is called when user wants to make some changes on the image
static void on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    int i;
    cairo_set_source_surface(cr, image_info.surface, 0, 0);
    cairo_paint(cr);
    for (i = 0; i <= count; i++)
    {
        cr = cairo_create(image_info.surface);
        switch (mouse_details[i].mode)
        {
        case PAINT:
            modify_pixel(cr, mouse_details[i].x, mouse_details[i].y, mouse_details[i].r, mouse_details[i].g, mouse_details[i].b);
            break;

        case LINE:
            draw_line(cr, mouse_details[i].x, mouse_details[i].y, mouse_details[i].r, mouse_details[i].g, mouse_details[i].b);
            break;

        case RHOMBUS:
            draw_rhombus(cr, mouse_details[i].x, mouse_details[i].y, mouse_details[i].r, mouse_details[i].g, mouse_details[i].b);
            break;

        case CIRCLE:
            draw_circle(cr, mouse_details[i].x, mouse_details[i].y, mouse_details[i].r, mouse_details[i].g, mouse_details[i].b);
            break;
        }
        gtk_widget_queue_draw(screen);
        cairo_destroy(cr);
    }
}

// Change the current mode for the mouse click
static void change_mode(GtkApplication *app, gpointer user_data)
{
    switch (*(char *)user_data)
    {
    case 's':
        current_mode = SELECT_COLOR;
        break;

    case 'p':
        current_mode = PAINT;
        break;

    case 'l':
        current_mode = LINE;
        break;

    case 'r':
        current_mode = RHOMBUS;
        break;

    case 'c':
        current_mode = CIRCLE;
        break;

    case 'u':
        if (count >= 0)
        {
            count--;
        }
        current_mode = NO_MODE;
        gtk_widget_queue_draw(screen);
        break;
    }
}

// Channge the current selected color value
static void on_draw_selected_colour(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    cairo_set_source_rgb(cr, selected_color[0], selected_color[1], selected_color[2]);
    cairo_rectangle(cr, 0, 0, 200, 30);
    cairo_fill(cr);
}

// Select Predefined Color from a color pallete
static void on_draw_predefined_colour(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    cairo_set_source_rgb(cr, 0.5, 0.5, 1);
    cairo_rectangle(cr, 40, 0, 30, 30);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
    cairo_rectangle(cr, 40, 40, 30, 30);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 0.3, 0);
    cairo_rectangle(cr, 80, 0, 30, 30);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1, 0, 0.5);
    cairo_rectangle(cr, 80, 40, 30, 30);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1, 10, 0.5);
    cairo_rectangle(cr, 120, 0, 30, 30);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, 120, 40, 30, 30);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 128, 0, 0);
    cairo_rectangle(cr, 160, 0, 30, 30);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 0, 225);
    cairo_rectangle(cr, 160, 40, 30, 30);
    cairo_fill(cr);
}

// Button event to do specific tasks like selecting colors and other image renderig operations
static void on_button_press(GtkWidget *widget, GdkEventButton *event, GdkWindowEdge edge)
{
    int x = (int)event->x - 20, y = (int)event->y - 20;
    if (event->button == 1)
    {
        if (current_mode == SELECT_COLOR)
        {
            unsigned char *image_data = cairo_image_surface_get_data(image_info.surface);
            selected_color[2] = (float)image_data[x * image_info.stride + y * 4 + 0] / 255;
            selected_color[1] = (float)image_data[x * image_info.stride + y * 4 + 1] / 255;
            selected_color[0] = (float)image_data[x * image_info.stride + y * 4 + 2] / 255;
            selected_color[3] = (float)image_data[x * image_info.stride + y * 4 + 3] / 255;
            gtk_widget_queue_draw(tool_screen);
        }
        else
        {
            if ((count + 1) == MAX_COMPUTATION)
                count = -1;
            count++;
            // Modifying Pixel Contents
            mouse_details[count].x = x;
            mouse_details[count].y = y;
            mouse_details[count].r = selected_color[0];
            mouse_details[count].g = selected_color[1];
            mouse_details[count].b = selected_color[2];
            mouse_details[count].a = selected_color[3];
            mouse_details[count].mode = current_mode;
            gtk_widget_queue_draw(screen);
        }
    }
}

// Set the current color from the color pallete
static void on_button_press_2(GtkWidget *widget, GdkEventButton *event, GdkWindowEdge edge)
{
    int x = (int)event->x - 20, y = (int)event->y - 20;
    if (event->button == 1)
    {
        // g_print("%d - %d \n", x, y);
        if (x >= 40 && x <= 70 && y >= 125 && y <= 155)
        {
            selected_color[0] = 0.5;
            selected_color[1] = 0.5;
            selected_color[2] = 1.0;
            selected_color[3] = 1.0;
            current_mode = SELECT_COLOR;
        }
        if (x >= 40 && x <= 70 && y >= 165 && y <= 195)
        {
            selected_color[0] = 0.6;
            selected_color[1] = 0.6;
            selected_color[2] = 0.6;
            selected_color[3] = 1.0;
            current_mode = SELECT_COLOR;
        }
        if (x >= 80 && x <= 110 && y >= 125 && y <= 155)
        {
            selected_color[0] = 0;
            selected_color[1] = 0.3;
            selected_color[2] = 0;
            selected_color[3] = 1.0;
            current_mode = SELECT_COLOR;
        }
        if (x >= 80 && x <= 110 && y >= 165 && y <= 195)
        {
            selected_color[0] = 1;
            selected_color[1] = 0;
            selected_color[2] = 0.5;
            selected_color[3] = 1.0;
            current_mode = SELECT_COLOR;
        }
        if (x >= 120 && x <= 150 && y >= 125 && y <= 155)
        {
            selected_color[0] = 1;
            selected_color[1] = 1;
            selected_color[2] = 0.5;
            selected_color[3] = 1.0;
            current_mode = SELECT_COLOR;
        }
        if (x >= 120 && x <= 150 && y >= 165 && y <= 195)
        {
            selected_color[0] = 0;
            selected_color[1] = 0;
            selected_color[2] = 0;
            selected_color[3] = 1.0;
            current_mode = SELECT_COLOR;
        }
        if (x >= 150 && x <= 190 && y >= 125 && y <= 155)
        {
            selected_color[0] = 1;
            selected_color[1] = 0;
            selected_color[2] = 0;
            selected_color[3] = 1.0;
            current_mode = SELECT_COLOR;
        }
        if (x >= 150 && x <= 190 && y >= 165 && y <= 1955)
        {
            selected_color[0] = 0;
            selected_color[1] = 0;
            selected_color[2] = 1;
            selected_color[3] = 1.0;
            current_mode = SELECT_COLOR;
        }
        gtk_widget_queue_draw(tool_screen);
    }
}

// Save the final image file in the system
static void save_and_output(GtkWidget *widget, gpointer user_data)
{
    cairo_t *cr;
    cairo_surface_t *save_image_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, image_info.width, image_info.height); // create new surface to store image
    cr = cairo_create(save_image_surface);
    cairo_set_source_surface(cr, image_info.surface, 0, 0);
    cairo_paint(cr);
    cairo_surface_write_to_png(save_image_surface, "save.png"); // save new image to file
}

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *tb_grid;
    GtkWidget *darea, *tb_darea, *tb_dpredefine_color;
    GtkWidget *tb_bselect_color, *tb_bpaint, *tb_bline, *tb_brhombus, *tb_bcircle, *tb_bundo, *tb_bsave;
    GtkWidget *tb_ltitle, *tb_lselected;

    screen = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(screen), "Get Pixel Value");
    gtk_container_set_border_width(GTK_CONTAINER(screen), 20);

    tool_screen = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(tool_screen), "Tool Box");
    gtk_container_set_border_width(GTK_CONTAINER(tool_screen), 20);

    // Set window size same as the image size
    image_info.surface = cairo_image_surface_create_from_png("icon.png");
    image_info.width = cairo_image_surface_get_width(image_info.surface);
    image_info.height = cairo_image_surface_get_height(image_info.surface);
    image_info.stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, image_info.width);
    gtk_window_set_default_size(GTK_WINDOW(screen), image_info.width, image_info.height);
    gtk_window_set_default_size(GTK_WINDOW(tool_screen), 200, image_info.height);
    gtk_window_move(GTK_WINDOW(tool_screen), 1600, 0);

    darea = gtk_drawing_area_new();
    tb_ltitle = gtk_label_new("T O O L  B O X");
    tb_lselected = gtk_label_new("Selected Colour     >>> ");
    tb_darea = gtk_drawing_area_new();
    tb_dpredefine_color = gtk_drawing_area_new();
    tb_bselect_color = gtk_button_new_with_label("Fetch Color");
    tb_bpaint = gtk_button_new_with_label("Brush");
    tb_bline = gtk_button_new_with_label("Draw Line");
    tb_brhombus = gtk_button_new_with_label("Draw Rectengle");
    tb_bcircle = gtk_button_new_with_label("Draw Circle");
    tb_bundo = gtk_button_new_with_label("Undo");
    tb_bsave = gtk_button_new_with_label("Save");

    // Setting Grid
    tb_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(tb_grid), 50);
    gtk_grid_set_row_spacing(GTK_GRID(tb_grid), 50);
    gtk_container_add(GTK_CONTAINER(tool_screen), tb_grid);

    gtk_grid_attach(GTK_GRID(tb_grid), tb_ltitle, 0, 0, 4, 1);
    gtk_grid_attach(GTK_GRID(tb_grid), tb_lselected, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(tb_grid), tb_darea, 2, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(tb_grid), tb_dpredefine_color, 0, 2, 4, 3);
    gtk_grid_attach(GTK_GRID(tb_grid), tb_bselect_color, 0, 5, 4, 1);
    gtk_grid_attach(GTK_GRID(tb_grid), tb_bpaint, 0, 6, 4, 1);
    gtk_grid_attach(GTK_GRID(tb_grid), tb_bline, 0, 7, 4, 1);
    gtk_grid_attach(GTK_GRID(tb_grid), tb_brhombus, 0, 8, 4, 1);
    gtk_grid_attach(GTK_GRID(tb_grid), tb_bcircle, 0, 9, 4, 1);
    gtk_grid_attach(GTK_GRID(tb_grid), tb_bundo, 0, 10, 4, 1);
    gtk_grid_attach(GTK_GRID(tb_grid), tb_bsave, 0, 11, 4, 1);

    // Set Properties
    gtk_container_add(GTK_CONTAINER(screen), darea);

    // Show screen
    gtk_widget_show_all(screen);
    gtk_widget_show_all(tool_screen);

    // Setting Current Mode
    mouse_details[count].mode = -1;

    // Callback function
    g_signal_connect(G_OBJECT(tb_darea), "draw", G_CALLBACK(on_draw_selected_colour), NULL);
    g_signal_connect(G_OBJECT(tb_dpredefine_color), "draw", G_CALLBACK(on_draw_predefined_colour), NULL);
    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(G_OBJECT(tb_bselect_color), "clicked", G_CALLBACK(change_mode), &mode1);
    g_signal_connect(G_OBJECT(tb_bpaint), "clicked", G_CALLBACK(change_mode), &mode2);
    g_signal_connect(G_OBJECT(tb_bline), "clicked", G_CALLBACK(change_mode), &mode3);
    g_signal_connect(G_OBJECT(tb_brhombus), "clicked", G_CALLBACK(change_mode), &mode4);
    g_signal_connect(G_OBJECT(tb_bcircle), "clicked", G_CALLBACK(change_mode), &mode5);
    g_signal_connect(G_OBJECT(tb_bundo), "clicked", G_CALLBACK(change_mode), &mode6);
    g_signal_connect(G_OBJECT(screen), "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(G_OBJECT(tool_screen), "button-press-event", G_CALLBACK(on_button_press_2), NULL);
    g_signal_connect(G_OBJECT(tb_bsave), "clicked", G_CALLBACK(save_and_output), NULL);
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    // Destroy surface
    cairo_surface_destroy(image_info.surface);
    return status;
}
