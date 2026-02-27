#ifndef BUTTON_H
#define BUTTON_H
#include <gtk/gtk.h>
#include <stdbool.h>

typedef struct button {
    const char *label;
    const char *icon;
    const char *css_class;
    bool has_frame;
    bool use_underline;
    int width;
    int height;
} button;

typedef struct{
    const char *label;
    GtkWidget *group_with;
    bool is_active;
} Radiobutton;

GtkWidget *create_button(button *btnConfig);
GtkWidget *create_radio_button(Radiobutton *btnConfig);
void on_aide_clicked(GtkButton *btn, gpointer parent_window);

#endif