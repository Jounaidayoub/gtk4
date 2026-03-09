#ifndef GTK_WRAPPER_CONTAINER_H
#define GTK_WRAPPER_CONTAINER_H

#include <gtk/gtk.h>
#include <stdbool.h>

#include "common.h"

/* Orientation Aliases */
#define ORIENT_HORIZ GTK_ORIENTATION_HORIZONTAL
#define ORIENT_VERT GTK_ORIENTATION_VERTICAL

/* Box Shortcuts */
#define BOX_H(sp) \
    (box_config) { .orientation = ORIENT_HORIZ, .spacing = sp }
#define BOX_V(sp) \
    (box_config) { .orientation = ORIENT_VERT, .spacing = sp }

/* Stack Transitions */
#define TRANSITION_NONE GTK_STACK_TRANSITION_TYPE_NONE
#define TRANSITION_CROSSFADE GTK_STACK_TRANSITION_TYPE_CROSSFADE
#define TRANSITION_SLIDE_LEFT GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT
#define TRANSITION_SLIDE_RIGHT GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT
#define TRANSITION_SLIDE_UP GTK_STACK_TRANSITION_TYPE_SLIDE_UP
#define TRANSITION_SLIDE_DOWN GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN
#define TRANSITION_OVER_UP GTK_STACK_TRANSITION_TYPE_OVER_UP
#define TRANSITION_UNDER_DOWN GTK_STACK_TRANSITION_TYPE_UNDER_DOWN

/* Transition Speeds (ms) */
#define TRANSITION_FAST 250
#define TRANSITION_NORMAL 400
#define TRANSITION_SLOW 800

typedef struct
{
    int column_spacing;
    int row_spacing;
    bool homogeneous_columns;
    bool homogeneous_rows;
    widget_style_config style;
} grid_config;

typedef struct
{
    GtkOrientation orientation;
    int spacing;
    bool homogeneous;
    widget_style_config style;
} box_config;

typedef struct
{
    GtkStackTransitionType transition;
    guint duration_ms;
    bool vhomogeneous;
    bool hhomogeneous;
    widget_style_config style;
} stack_config;
GtkWidget *container_add(GtkWidget *container, GtkWidget *child, int a, int b,
                         int c, int d);
GtkWidget *grid_add(GtkWidget *grid, GtkWidget *child, int a, int b, int c,
                    int d);
GtkWidget *create_grid(const grid_config *config);
GtkWidget *create_box(const box_config *config);
GtkWidget *create_stack(const stack_config *config);
GtkWidget *create_stack_sidebar(GtkWidget *stack, const widget_style_config *style);
void stack_add_page(GtkWidget *stack, GtkWidget *child, const char *name, const char *title);

#endif
