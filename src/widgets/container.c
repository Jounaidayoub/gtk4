#include "widgets/container.h"


GtkWidget *container_add(GtkWidget *container, GtkWidget *child, int a, int b,
                         int c, int d)
{
    if (container == NULL || child == NULL) {
        return NULL;
    }

    if (GTK_IS_BOX(container)) {
        gtk_box_append(GTK_BOX(container), child);
        return child;
    }

    if (GTK_IS_GRID(container)) {
        gtk_grid_attach(GTK_GRID(container), child, a, b, c, d);
        return child;
    }

    return NULL;
}

GtkWidget *grid_add(GtkWidget *grid, GtkWidget *child, int a, int b, int c, int d)
{
    return container_add(grid, child, a, b, c, d);
}

GtkWidget *create_grid(const grid_config *config) {
    GtkWidget *grid = gtk_grid_new();
    if (config == NULL) {
        return grid;
    }

    gtk_grid_set_column_spacing(GTK_GRID(grid), config->column_spacing);
    gtk_grid_set_row_spacing(GTK_GRID(grid), config->row_spacing);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), config->homogeneous_columns);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), config->homogeneous_rows);
    apply_widget_style(grid, &config->style);
    return grid;
}

GtkWidget *create_box(const box_config *config) {
    GtkWidget *box = gtk_box_new(
        config ? config->orientation : GTK_ORIENTATION_VERTICAL,
        config ? config->spacing : 0
    );
    if (config == NULL) {
        return box;
    }

    gtk_box_set_homogeneous(GTK_BOX(box), config->homogeneous);
    apply_widget_style(box, &config->style);
    return box;
}

GtkWidget *create_stack(const stack_config *config) {
    GtkWidget *stack = gtk_stack_new();
    if (config == NULL) {
        return stack;
    }

    gtk_stack_set_transition_type(GTK_STACK(stack), config->transition);
    gtk_stack_set_transition_duration(GTK_STACK(stack), config->duration_ms);
    gtk_stack_set_vhomogeneous(GTK_STACK(stack), config->vhomogeneous);
    gtk_stack_set_hhomogeneous(GTK_STACK(stack), config->hhomogeneous);
    apply_widget_style(stack, &config->style);
    return stack;
}

GtkWidget *create_stack_sidebar(GtkWidget *stack, const widget_style_config *style) {
    GtkWidget *sidebar = gtk_stack_sidebar_new();
    if (stack != NULL) {
        gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(sidebar), GTK_STACK(stack));
    }
    apply_widget_style(sidebar, style);
    return sidebar;
}

void stack_add_page(GtkWidget *stack, GtkWidget *child, const char *name, const char *title) {
    if (stack == NULL || child == NULL || name == NULL || title == NULL) {
        return;
    }

    gtk_stack_add_named(GTK_STACK(stack), child, name);
    g_object_set(gtk_stack_get_page(GTK_STACK(stack), child), "title", title, NULL);
}
