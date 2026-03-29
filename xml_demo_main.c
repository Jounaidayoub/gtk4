#include "demo_app.h"
#include "widgets/xml_parser.h"
#include "widgets/theme.h"

#include "src/handlers.c"

#include <gtk/gtk.h>
#include <string.h>

static void xml_demo_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    static bool once = false;
    if (!once) {
        apply_demo_theme();
        once = true;
    }

    showcase_state *state = g_new0(showcase_state, 1);

    /* Map XML callback names to the actual functions defined in handlers.c */
    const xml_callback_entry callbacks[] = {
        {"on_entry_changed",      (void*)on_entry_changed},
        {"on_role_selected",      (void*)on_role_selected},
        {"on_spin_value_changed", (void*)on_spin_value_changed},
        {"on_button_clicked",     (void*)on_button_clicked},
        {"on_checkbox_toggled",   (void*)on_checkbox_toggled},
        {"on_switch_changed",     (void*)on_switch_changed},
        {"on_day_selected",       (void*)on_day_selected},
        {"on_show_dialog",        (void*)on_show_dialog},
        {"on_inc_progress",       (void*)on_inc_progress},
        {"on_toggle_spinner",     (void*)on_toggle_spinner},
        {"on_icon_btn_clicked",   (void*)on_icon_btn_clicked},
        {"on_star_clicked",       (void*)on_star_clicked},
        {"on_copy_clicked",       (void*)on_copy_clicked},
        {"on_search_clicked",     (void*)on_search_clicked},
        {"on_home_clicked",       (void*)on_home_clicked},
        {"on_zoom_in",            (void*)on_zoom_in},
        {"on_zoom_out",           (void*)on_zoom_out},
        {"on_icon_selected",      (void*)on_icon_selected},
    };

    xml_parse_context ctx = {
        .app            = app,
        .user_data      = state,
        .callbacks      = callbacks,
        .callback_count = G_N_ELEMENTS(callbacks),
    };

    GtkWidget *window = xml_parse_file("demo_app.xml", &ctx);
    if (!window) {
        g_warning("Failed to parse demo_app.xml");
        return;
    }

    /* Grab all the widgets the handlers expect to exist */
    state->status_label    = xml_get_widget_by_id("status_label");
    state->progress        = xml_get_widget_by_id("demo_progress");
    state->spinner         = xml_get_widget_by_id("demo_spinner");
    state->switch_widget   = NULL; // The parent handles switch changes, we don't need manual ref here for demo
    state->icon_size_label = xml_get_widget_by_id("icon_size_label");
    state->icon_preview    = xml_get_widget_by_id("icon_preview");
    
    state->spinner_running = true;

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app =
        gtk_application_new("org.gtk.xmldemo", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(xml_demo_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
