#ifndef WIDGETS_H
#define WIDGETS_H

#include <gtk/gtk.h>
#include <stdbool.h>

typedef struct window {
  GtkApplication *app;
  char *title;
  int width;
  int height;
  bool maximized;
} Window;

GtkWidget *create_window(Window *win);

#endif
