#include "widgets.h"

GtkWidget *create_window(Window *win) {
  GtkWidget *window = gtk_application_window_new(win->app);
  gtk_window_set_title(GTK_WINDOW(window), win->title);
  gtk_window_set_default_size(GTK_WINDOW(window), win->width, win->height);
  if (win->maximized) {
    gtk_window_maximize(GTK_WINDOW(window));
  }
  return window;
}
