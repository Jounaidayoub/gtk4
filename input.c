#include "input.h"

GtkWidget *create_input(input_config *config) {
    GtkWidget *entry = gtk_entry_new();
    if (config == NULL) return entry;

    if (config->placeholder) gtk_entry_set_placeholder_text(GTK_ENTRY(entry), config->placeholder);
    if (config->default_text) gtk_editable_set_text(GTK_EDITABLE(entry), config->default_text);
    if (config->is_password) gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    if (config->max_length > 0) gtk_entry_set_max_length(GTK_ENTRY(entry), config->max_length);
    if (config->css_class && config->css_class[0] != '\0') gtk_widget_add_css_class(entry, config->css_class);

    return entry;
}

GtkWidget *create_select(select_config *config) {
    if (config == NULL || config->options == NULL) return gtk_drop_down_new(NULL, NULL);

    GtkWidget *dropdown = gtk_drop_down_new_from_strings(config->options);
    if (config->selected_index > 0) gtk_drop_down_set_selected(GTK_DROP_DOWN(dropdown), config->selected_index);
    if (config->css_class && config->css_class[0] != '\0') gtk_widget_add_css_class(dropdown, config->css_class);

    return dropdown;
}