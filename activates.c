#include "activates.h"
#include "widgets.h"
#include "button.h"
#include "containers.h"
#include "input.h"    

void activatewin1(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    Window winConfig = {
        .title = "Formulaire d'inscription",
        .width = 600,
        .height = 400,
        .maximized = false
    };
    GtkWidget *window = create_window(app, &winConfig);

    Grid gridConfig = {
        .cl_spacing = 15,
        .margin_top = 0,
        .r_spacing = 20,
    };
    GtkWidget *grid = create_grid(&gridConfig);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER); 
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER); 

    input_config inpConfig = {
        .placeholder = "Nom d'utilisateur...",
        .default_text = NULL,
        .css_class = "",
        .is_password = false,
        .max_length = 50
    };
    GtkWidget *input_field = create_input(&inpConfig);
    gtk_widget_set_size_request(input_field, 250, -1);

    input_config passConfig = {
        .placeholder = "Mot de passe...",
        .default_text = NULL,
        .css_class = "",
        .is_password = true, 
        .max_length = 50
    };
    GtkWidget *password_field = create_input(&passConfig);
    gtk_widget_set_size_request(password_field, 250, -1);

    GtkWidget *role_label = gtk_label_new("Rôle :");
    gtk_widget_set_halign(role_label, GTK_ALIGN_START);

    GtkWidget *sexe_label = gtk_label_new("Sexe :");
    gtk_widget_set_halign(sexe_label, GTK_ALIGN_START);

    const char *dropdown_options[] = {"Administrateur", "Enseignant", "Etudiant", NULL}; 
    select_config selConfig = {
        .options = dropdown_options,
        .selected_index = 0, 
        .css_class = ""
    };
    GtkWidget *select_field = create_select(&selConfig);
    gtk_widget_set_size_request(select_field, 250, -1);

    Box boxConfig = {
        .orientation = GTK_ORIENTATION_HORIZONTAL,
        .spacing = 10,
        .homogeneous = true
    };
    GtkWidget *box = create_box(&boxConfig);

    Radiobutton R1 = { .label = "Masculin", .group_with = NULL, .is_active = true };
    GtkWidget *radio1 = create_radio_button(&R1);
    
    Radiobutton R2 = { .label = "Feminin", .group_with = radio1, .is_active = false };
    GtkWidget *radio2 = create_radio_button(&R2);
    
    gtk_box_append(GTK_BOX(box), radio1);
    gtk_box_append(GTK_BOX(box), radio2);
    gtk_widget_set_halign(box, GTK_ALIGN_START);

    GtkWidget *aide_btn = gtk_button_new();
    gtk_button_set_has_frame(GTK_BUTTON(aide_btn), FALSE);
    GtkWidget *aide_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(aide_label), "<span underline='single' color='#3584e4'>Aide</span>");
    gtk_button_set_child(GTK_BUTTON(aide_btn), aide_label);
    gtk_widget_set_margin_top(aide_btn, 15);
    g_signal_connect(aide_btn, "clicked", G_CALLBACK(on_aide_clicked), window);

    button btnConfig = {
        .label = "_Inscription",
        .css_class = "",
        .has_frame = true,
        .use_underline = true,
        .width = 250,
        .height = 40 
    };
    GtkWidget *submit_button = create_button(&btnConfig);

    gtk_grid_attach(GTK_GRID(grid), input_field, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), password_field, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), role_label, 0, 2, 1, 1);   
    gtk_grid_attach(GTK_GRID(grid), select_field, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sexe_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), box, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), submit_button, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), aide_btn, 0, 5, 2, 1);

    gtk_window_set_child(GTK_WINDOW(window), grid);
    gtk_window_present(GTK_WINDOW(window));
}