#ifndef INPUT_H
#define INPUT_H
#include <gtk/gtk.h>
#include <stdbool.h>

typedef struct {
    const char *placeholder;
    const char *default_text;
    const char *css_class;
    bool is_password;
    int max_length;
} input_config;

typedef struct {
    const char **options;
    int selected_index;
    const char *css_class;
} select_config;

GtkWidget *create_input(input_config *config);
GtkWidget *create_select(select_config *config);

#endif