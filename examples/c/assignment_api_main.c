#include "widgets/button.h"
#include "widgets/container.h"
#include "widgets/dialog.h"
#include "widgets/display.h"
#include "widgets/input.h"
#include "widgets/menu.h"
#include "widgets/theme.h"
#include "widgets/toggle.h"
#include "widgets/window.h"

typedef struct {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *status_label;
} assignment_state;

static void set_status(assignment_state *state, const char *text) {
    if (state != NULL && state->status_label != NULL) {
        gtk_label_set_text(GTK_LABEL(state->status_label), text);
    }
}

static void on_menu_choice(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action;
    (void)parameter;
    set_status(user_data, "Menu item selected");
}

static void on_dialog_response(int response_index, gpointer user_data) {
    static const char *responses[] = {"Oui", "Non", "Annuler"};
    assignment_state *state = user_data;

    if (response_index >= 0 && response_index < (int)G_N_ELEMENTS(responses)) {
        char message[64];
        g_snprintf(message, sizeof(message), "Dialogue: %s", responses[response_index]);
        set_status(state, message);
        return;
    }

    set_status(state, "Dialogue fermé");
}

static void on_show_image_dialog(GtkButton *button, gpointer user_data) {
    (void)button;
    assignment_state *state = user_data;
    const alert_button_config buttons[] = {
        {.label = "Oui", .appearance = "suggested"},
        {.label = "Non", .appearance = "flat"},
        {.label = "Annuler", .appearance = "cancel"},
    };

    show_alert_dialog(&(alert_dialog_config){
        .parent = GTK_WINDOW(state->window),
        .message = "Voulez vous afficher une image ?",
        .detail = NULL,
        .buttons = buttons,
        .button_count = G_N_ELEMENTS(buttons),
        .default_button = 0,
        .cancel_button = 2,
        .modal = true,
        .on_response = on_dialog_response,
        .user_data = state,
    });
}

static void on_ok_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    set_status(user_data, "OK clicked");
}

static void on_quit_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    assignment_state *state = user_data;
    g_application_quit(G_APPLICATION(state->app));
}

static GtkWidget *create_assignment_menubar(assignment_state *state) {
    const menu_item_config choix221[] = {
        {.label = "Choix221", .kind = MENU_ITEM_ACTION, .enabled = true, .on_activate = on_menu_choice, .user_data = state},
        {.label = "Choix222", .kind = MENU_ITEM_ACTION, .enabled = true, .on_activate = on_menu_choice, .user_data = state},
        {.label = "Choix223", .kind = MENU_ITEM_ACTION, .enabled = true, .on_activate = on_menu_choice, .user_data = state},
    };
    const menu_item_config submenu_items[] = {
        {.label = "Sous Choix 1", .kind = MENU_ITEM_ACTION, .enabled = true, .on_activate = on_menu_choice, .user_data = state},
        {.label = "Sous Choix2", .kind = MENU_ITEM_SUBMENU, .children = choix221, .child_count = G_N_ELEMENTS(choix221), .enabled = true},
        {.label = "Sous Choix3", .kind = MENU_ITEM_ACTION, .enabled = true, .on_activate = on_menu_choice, .user_data = state},
    };
    const menu_section_config sections[] = {
        {.label = "MonChoix", .items = submenu_items, .item_count = G_N_ELEMENTS(submenu_items)},
        {.label = "VotreChoix", .items = submenu_items, .item_count = G_N_ELEMENTS(submenu_items)},
        {.label = "LeurChoix", .items = submenu_items, .item_count = G_N_ELEMENTS(submenu_items)},
    };

    return create_menubar(&(menubar_config){
        .sections = sections,
        .section_count = G_N_ELEMENTS(sections),
        .action_map = G_ACTION_MAP(state->app),
        .layout = MENU_LAYOUT_VERTICAL,
        .show_arrow_indicators = true,
        .style = {.set_halign = true, .halign = GTK_ALIGN_START},
    });
}

static GtkWidget *create_radio_row(const char *title,
                                   const char *first_id_label,
                                   const char *second_label,
                                   const char *third_label,
                                   const char *fourth_label) {
    GtkWidget *root = create_box(&(box_config){
        .orientation = GTK_ORIENTATION_VERTICAL,
        .spacing = 4,
    });
    container_add(root, create_label(&(label_config){
        .text = title,
        .style = {.set_halign = true, .halign = GTK_ALIGN_START},
    }), 0, 0, 0, 0);

    GtkWidget *row = create_box(&(box_config){
        .orientation = GTK_ORIENTATION_HORIZONTAL,
        .spacing = 18,
        .style = {.set_halign = true, .halign = GTK_ALIGN_START},
    });
    GtkWidget *first = create_radio_button(&(radio_button_config){.label = first_id_label});
    container_add(row, first, 0, 0, 0, 0);
    container_add(row, create_radio_button(&(radio_button_config){.label = second_label, .group_with = first}), 0, 0, 0, 0);
    container_add(row, create_radio_button(&(radio_button_config){.label = third_label, .group_with = first}), 0, 0, 0, 0);
    container_add(row, create_radio_button(&(radio_button_config){.label = fourth_label, .group_with = first, .is_active = true}), 0, 0, 0, 0);
    container_add(root, row, 0, 0, 0, 0);

    return root;
}

static GtkWidget *create_comments_row(void) {
    GtkWidget *root = create_box(&(box_config){.orientation = GTK_ORIENTATION_VERTICAL, .spacing = 4});
    GtkWidget *row = create_box(&(box_config){
        .orientation = GTK_ORIENTATION_HORIZONTAL,
        .spacing = 18,
        .style = {.set_halign = true, .halign = GTK_ALIGN_START},
    });

    container_add(root, create_label(&(label_config){
        .text = "Vos Commentaires sur GTK+",
        .style = {.set_halign = true, .halign = GTK_ALIGN_START},
    }), 0, 0, 0, 0);
    container_add(row, create_checkbox(&(checkbox_config){.label = "Convivial"}), 0, 0, 0, 0);
    container_add(row, create_checkbox(&(checkbox_config){.label = "Intéressant", .active = true}), 0, 0, 0, 0);
    container_add(row, create_checkbox(&(checkbox_config){.label = "Facile à utiliser"}), 0, 0, 0, 0);
    container_add(row, create_checkbox(&(checkbox_config){.label = "Optimise le travail"}), 0, 0, 0, 0);
    container_add(root, row, 0, 0, 0, 0);

    return root;
}

static GtkWidget *create_form_area(void) {
    static const char *choices[] = {"Faire un choix", "Choix 1", "Choix 2", "Choix 3", NULL};
    GtkWidget *grid = create_grid(&(grid_config){
        .column_spacing = 10,
        .row_spacing = 10,
        .style = {.margin_top = 24, .margin_start = 18, .set_halign = true, .halign = GTK_ALIGN_START},
    });

    container_add(grid, create_label(&(label_config){.text = "Zone de saisie"}), 0, 0, 1, 1);
    container_add(grid, create_entry(&(entry_config){
        .placeholder = "Saisir un texte",
        .style = {.width_request = 280},
    }), 1, 0, 4, 1);

    container_add(grid, create_label(&(label_config){
        .text = "Choix de date",
        .style = {.css_class = "title-2"},
    }), 0, 1, 6, 1);

    container_add(grid, create_label(&(label_config){.text = "Jour"}), 0, 2, 1, 1);
    container_add(grid, create_spin_button(&(spin_button_config){
        .min_value = 1,
        .max_value = 31,
        .step = 1,
        .value = 1,
        .numeric_only = true,
        .style = {.width_request = 80},
    }), 1, 2, 1, 1);

    container_add(grid, create_label(&(label_config){.text = "mois"}), 2, 2, 1, 1);
    container_add(grid, create_spin_button(&(spin_button_config){
        .min_value = 0,
        .max_value = 12,
        .step = 1,
        .numeric_only = true,
        .style = {.width_request = 80},
    }), 3, 2, 1, 1);

    container_add(grid, create_label(&(label_config){.text = "année"}), 4, 2, 1, 1);
    container_add(grid, create_spin_button(&(spin_button_config){
        .min_value = 0,
        .max_value = 2100,
        .step = 1,
        .numeric_only = true,
        .style = {.width_request = 100},
    }), 5, 2, 1, 1);

    container_add(grid, create_dropdown(&(dropdown_config){
        .options = choices,
        .selected_index = 0,
        .style = {.width_request = 240},
    }), 0, 3, 3, 1);

    return grid;
}

static GtkWidget *build_assignment_ui(assignment_state *state) {
    GtkWidget *root = create_box(&(box_config){
        .orientation = GTK_ORIENTATION_VERTICAL,
        .spacing = 16,
        .style = {.margin_top = 16, .margin_bottom = 16, .margin_start = 16, .margin_end = 16},
    });

    container_add(root, create_assignment_menubar(state), 0, 0, 0, 0);
    container_add(root, create_radio_row("Vos connaissances de GTK+", "Faible", "Moyen", "Bien", "Très Bien"), 0, 0, 0, 0);
    container_add(root, create_comments_row(), 0, 0, 0, 0);
    container_add(root, create_form_area(), 0, 0, 0, 0);
    container_add(root, create_button(&(button_config){
        .label = "Afficher le dialogue",
        .icon_name = "dialog-warning-symbolic",
        .style = {.width_request = 260, .margin_top = 24, .set_halign = true, .halign = GTK_ALIGN_CENTER},
        .on_clicked = on_show_image_dialog,
        .user_data = state,
    }), 0, 0, 0, 0);
    container_add(root, create_radio_row("Le CoronaVirus", "Malade", "Suspect", "Guéri", "Sain"), 0, 0, 0, 0);

    GtkWidget *buttons = create_box(&(box_config){
        .orientation = GTK_ORIENTATION_HORIZONTAL,
        .spacing = 80,
        .style = {.margin_top = 24, .set_halign = true, .halign = GTK_ALIGN_CENTER},
    });
    container_add(buttons, create_button(&(button_config){
        .label = "_OK",
        .icon_name = "emblem-ok-symbolic",
        .use_underline = true,
        .style = {.width_request = 240, .height_request = 58},
        .on_clicked = on_ok_clicked,
        .user_data = state,
    }), 0, 0, 0, 0);
    container_add(buttons, create_button(&(button_config){
        .label = "Quitter",
        .icon_name = "application-exit-symbolic",
        .style = {.width_request = 240, .height_request = 58},
        .on_clicked = on_quit_clicked,
        .user_data = state,
    }), 0, 0, 0, 0);
    container_add(root, buttons, 0, 0, 0, 0);

    state->status_label = create_label(&(label_config){
        .text = "Ready",
        .style = {.margin_top = 8, .set_halign = true, .halign = GTK_ALIGN_START},
    });
    container_add(root, state->status_label, 0, 0, 0, 0);

    return root;
}

static void assignment_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    static bool once = false;
    if (!once) {
        apply_demo_theme();
        once = true;
    }

    assignment_state *state = g_new0(assignment_state, 1);
    state->app = app;
    state->window = create_window(app, &(window_config){
        .title = "Test des fonctions GTK+",
        .icon_name = "applications-development-symbolic",
        .background_image_path = "image2.png",
        .default_width = 900,
        .default_height = 900,
        .min_width = 760,
        .min_height = 720,
        .resizable = true,
        .decorated = true,
    });

    container_add(state->window, build_assignment_ui(state), 0, 0, 0, 0);
    gtk_window_present(GTK_WINDOW(state->window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.gtk.assignmentapi", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(assignment_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
