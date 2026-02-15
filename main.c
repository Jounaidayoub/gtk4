#include "widgets.h"

static void launchApp(GtkApplication *app, gpointer user_data) {

  GtkWidget *window = create_window(&(Window){.app = app,
                                              .title = "📙 GTK4 Widget Demo",
                                              .width = 600,
                                              .height = 400,
                                              .maximized = false});

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  GtkApplication *app =
      gtk_application_new("org.gtk.demo", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(launchApp), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
