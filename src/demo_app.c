#include "demo_app.h"

#include "gtk/gtk.h"
#include "widgets/button.h"
#include "widgets/container.h"
#include "widgets/date.h"
#include "widgets/dialog.h"
#include "widgets/display.h"
#include "widgets/input.h"
#include "widgets/menu.h"
#include "widgets/separator.h"
#include "widgets/theme.h"
#include "widgets/toggle.h"
#include "widgets/window.h"

#include <string.h>

typedef struct {
  GtkWidget *status_label;
  GtkWidget *progress;
  GtkWidget *spinner;
  GtkWidget *switch_widget;
  bool spinner_running;
} showcase_state;

static void set_status(showcase_state *state, const char *text) {
  gtk_label_set_text(GTK_LABEL(state->status_label), text);
}

static void on_entry_changed(GtkEditable *editable, gpointer user_data) {
  showcase_state *state = user_data;
  char msg[96];
  g_snprintf(msg, sizeof(msg), "Input length: %zu",
             strlen(gtk_editable_get_text(editable)));
  set_status(state, msg);
}

static void on_role_selected(GtkDropDown *dropdown, GParamSpec *pspec,
                             gpointer user_data) {
  (void)pspec;
  showcase_state *state = user_data;
  char msg[96];
  g_snprintf(msg, sizeof(msg), "Role selected index: %u",
             gtk_drop_down_get_selected(dropdown));
  set_status(state, msg);
}

static void on_spin_value_changed(GtkSpinButton *spin, gpointer user_data) {
  showcase_state *state = user_data;
  char msg[96];
  g_snprintf(msg, sizeof(msg), "Spin value: %.2f",
             gtk_spin_button_get_value(spin));
  set_status(state, msg);
}

static void on_button_clicked(GtkButton *button, gpointer user_data) {
  (void)button;
  set_status((showcase_state *)user_data, "Primary action clicked");
}

static void on_checkbox_toggled(GtkCheckButton *button, gpointer user_data) {
  showcase_state *state = user_data;
  set_status(state, gtk_check_button_get_active(button) ? "Checkbox enabled"
                                                        : "Checkbox disabled");
}

static void on_switch_changed(GtkSwitch *sw, GParamSpec *pspec,
                              gpointer user_data) {
  (void)pspec;
  showcase_state *state = user_data;
  set_status(state,
             gtk_switch_get_active(sw) ? "Switch active" : "Switch inactive");
}

static void on_day_selected(GtkCalendar *calendar, gpointer user_data) {
  showcase_state *state = user_data;
  GDateTime *d = gtk_calendar_get_date(calendar);
  char *s = g_date_time_format(d, "%Y-%m-%d");
  char msg[128];
  g_snprintf(msg, sizeof(msg), "Date selected: %s", s);
  set_status(state, msg);
  g_free(s);
  g_date_time_unref(d);
}

static void on_alert_result(int response_index, gpointer user_data) {
  showcase_state *state = user_data;
  char msg[96];
  g_snprintf(msg, sizeof(msg), "Dialog response index: %d", response_index);
  set_status(state, msg);
}

static void on_show_dialog(GtkButton *button, gpointer user_data) {
  (void)button;
  showcase_state *state = user_data;
  GtkRoot *root = gtk_widget_get_root(state->status_label);
  alert_button_config buttons[] = {
      {.label = "Cancel", .appearance = "flat"},
      {.label = "Confirm", .appearance = "suggested"},
  };

  show_alert_dialog(&(alert_dialog_config){
      .parent = (root && GTK_IS_WINDOW(root)) ? GTK_WINDOW(root) : NULL,
      .message = "Proceed with wrapper-driven action?",
      .detail = "This dialog uses GtkAlertDialog only (no deprecated API).",
      .buttons = buttons,
      .button_count = G_N_ELEMENTS(buttons),
      .default_button = 1,
      .cancel_button = 0,
      .modal = true,
      .on_response = on_alert_result,
      .user_data = state,
  });
}

static void on_inc_progress(GtkButton *button, gpointer user_data) {
  (void)button;
  showcase_state *state = user_data;
  double f = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(state->progress));
  f += 0.12;
  if (f > 1.0) {
    f = 0.0;
  }
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(state->progress), f);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(state->progress),
                            f > 0.99 ? "Done" : "Working");
  set_status(state, "Progress updated");
}

static void on_toggle_spinner(GtkButton *button, gpointer user_data) {
  (void)button;
  showcase_state *state = user_data;
  state->spinner_running = !state->spinner_running;
  if (state->spinner_running) {
    gtk_spinner_start(GTK_SPINNER(state->spinner));
    set_status(state, "Spinner started");
  } else {
    gtk_spinner_stop(GTK_SPINNER(state->spinner));
    set_status(state, "Spinner stopped");
  }
}

static GtkWidget *make_inputs_page(showcase_state *state) {
  GtkWidget *grid = create_grid(&(grid_config){
      .column_spacing = 12,
      .row_spacing = 12,
      .homogeneous_columns = false,
      .homogeneous_rows = false,
      .style = {.margin_top = 16,
                .margin_bottom = 16,
                .margin_start = 16,
                .margin_end = 16},
  });

  const char *roles[] = {"Admin", "Teacher", "Student", "Guest", NULL};
  GtkWidget *label_user = create_label(&(label_config){
      .text = "Username",
      .selectable = false,
      .wrap = false,
      .ellipsize = PANGO_ELLIPSIZE_NONE,
      .max_width_chars = 0,
      .style = {0},
  });
  GtkWidget *entry_user = create_entry(&(entry_config){
      .placeholder = "enter username",
      .default_text = "",
      .purpose_hint = "text",
      .password = false,
      .editable = true,
      .max_length = 60,
      .style = {.hexpand = true, .set_hexpand = true},
      .on_changed = on_entry_changed,
      .user_data = state,
  });

  GtkWidget *label_pass = create_label(&(label_config){
      .text = "Password",
      .selectable = false,
      .wrap = false,
      .ellipsize = PANGO_ELLIPSIZE_NONE,
      .max_width_chars = 0,
      .style = {0},
  });
  GtkWidget *entry_pass = create_entry(&(entry_config){
      .placeholder = "enter password",
      .default_text = "",
      .purpose_hint = "text",
      .password = true,
      .editable = true,
      .max_length = 60,
      .style = {.hexpand = true, .set_hexpand = true},
      .on_changed = on_entry_changed,
      .user_data = state,
  });

  GtkWidget *dropdown = create_dropdown(&(dropdown_config){
      .options = roles,
      .selected_index = 0,
      .enable_search = true,
      .style = {.hexpand = true, .set_hexpand = true},
      .on_selected = on_role_selected,
      .user_data = state,
  });

  GtkWidget *spin = create_spin_button(&(spin_button_config){
      .min_value = 0.0,
      .max_value = 100.0,
      .step = 0.5,
      .page = 5.0,
      .value = 12.5,
      .digits = 1,
      .numeric_only = true,
      .wrap = false,
      .style = {.width_request = 140},
      .on_value_changed = on_spin_value_changed,
      .user_data = state,
  });

  container_add(grid, label_user, 0, 0, 1, 1);
  container_add(grid, entry_user, 1, 0, 1, 1);
  container_add(grid, label_pass, 0, 1, 1, 1);
  container_add(grid, entry_pass, 1, 1, 1, 1);
  container_add(grid,
                create_label(&(label_config){.text = "Role", .style = {0}}),
                0, 2, 1, 1);
  container_add(grid, dropdown, 1, 2, 1, 1);
  container_add(grid,
                create_label(&(label_config){.text = "Score", .style = {0}}),
                0, 3, 1, 1);
  container_add(grid, spin, 1, 3, 1, 1);
  return grid;
}

static GtkWidget *make_buttons_page(showcase_state *state) {
  GtkWidget *box = create_box(&(box_config){
      .orientation = GTK_ORIENTATION_VERTICAL,
      .spacing = 10,
      .homogeneous = false,
      .style = {.margin_top = 16,
                .margin_bottom = 16,
                .margin_start = 16,
                .margin_end = 16},
  });

    container_add(box,
          create_button(&(button_config){
            .label = "_Save",
            .icon_name = "document-save-symbolic",
            .theme_variant = "primary",
            .use_underline = true,
            .has_frame = true,
            .can_shrink = false,
            .icon_size = 16,
            .style = {.width_request = 180, .height_request = 38},
            .on_clicked = on_button_clicked,
            .user_data = state,
          }),
          0, 0, 0, 0);

    container_add(box,
          create_button(&(button_config){
            .label = "Approve",
            .icon_name = "emblem-ok-symbolic",
            .theme_variant = "success",
            .use_underline = false,
            .has_frame = true,
            .can_shrink = true,
            .icon_size = 16,
            .style = {.width_request = 180, .height_request = 38},
            .on_clicked = on_button_clicked,
            .user_data = state,
          }),
          0, 0, 0, 0);

    container_add(box,
          create_button(&(button_config){
            .label = "Delete",
            .icon_name = "user-trash-symbolic",
            .theme_variant = "danger",
            .use_underline = false,
            .has_frame = true,
            .can_shrink = true,
            .icon_size = 16,
            .style = {.width_request = 180, .height_request = 38},
            .on_clicked = on_button_clicked,
            .user_data = state,
          }),
          0, 0, 0, 0);

  GtkWidget *radio_a = create_radio_button(&(radio_button_config){
      .label = "Mode A",
      .group_with = NULL,
      .is_active = true,
      .style = {0},
      .on_toggled = NULL,
      .user_data = NULL,
  });
  GtkWidget *radio_b = create_radio_button(&(radio_button_config){
      .label = "Mode B",
      .group_with = radio_a,
      .is_active = false,
      .style = {0},
      .on_toggled = NULL,
      .user_data = NULL,
  });
  container_add(box, radio_a, 0, 0, 0, 0);
  container_add(box, radio_b, 0, 0, 0, 0);
  return box;
}

static GtkWidget *make_toggles_page(showcase_state *state) {
  GtkWidget *box = create_box(&(box_config){
      .orientation = GTK_ORIENTATION_VERTICAL,
      .spacing = 12,
      .homogeneous = false,
      .style = {.margin_top = 16,
                .margin_bottom = 16,
                .margin_start = 16,
                .margin_end = 16},
  });

    container_add(box,
          create_checkbox(&(checkbox_config){
            .label = "Enable notifications",
            .active = false,
            .inconsistent = false,
            .style = {.margin_bottom = 6},
            .on_toggled = on_checkbox_toggled,
            .user_data = state,
          }),
          0, 0, 0, 0);

  GtkWidget *switch_row = create_switch_row(
      &(switch_config){
          .label = "Realtime sync",
          .active = true,
          .state = true,
          .style = {0},
          .on_active_changed = on_switch_changed,
          .user_data = state,
      },
      &state->switch_widget);
  container_add(box, switch_row, 0, 0, 0, 0);
  return box;
}

static GtkWidget *make_displays_page(showcase_state *state) {
  GtkWidget *box = create_box(&(box_config){
      .orientation = GTK_ORIENTATION_VERTICAL,
      .spacing = 12,
      .homogeneous = false,
      .style = {.margin_top = 16,
                .margin_bottom = 16,
                .margin_start = 16,
                .margin_end = 16},
  });

  state->progress = create_progress_bar(&(progress_bar_config){
      .fraction = 0.32,
      .text = "Working",
      .orientation = GTK_ORIENTATION_HORIZONTAL,
      .show_text = true,
      .inverted = false,
      .pulse_once = false,
      .style = {.width_request = 340},
  });
  state->spinner = create_spinner(&(spinner_config){
      .size = 26,
      .spinning = true,
      .style = {0},
  });
  state->spinner_running = true;

    container_add(box, state->progress, 0, 0, 0, 0);
    container_add(box, state->spinner, 0, 0, 0, 0);
    container_add(box,
          create_button(&(button_config){
            .label = "Advance progress",
            .icon_name = "go-next-symbolic",
            .theme_variant = "primary",
            .use_underline = false,
            .has_frame = true,
            .can_shrink = true,
            .icon_size = 16,
            .style = {.width_request = 220},
            .on_clicked = on_inc_progress,
            .user_data = state,
          }),
          0, 0, 0, 0);
    container_add(box,
          create_button(&(button_config){
            .label = "Toggle spinner",
            .icon_name = "view-refresh-symbolic",
            .theme_variant = "flat",
            .use_underline = false,
            .has_frame = false,
            .can_shrink = true,
            .icon_size = 16,
            .style = {.width_request = 220},
            .on_clicked = on_toggle_spinner,
            .user_data = state,
          }),
          0, 0, 0, 0);
  return box;
}

static GtkWidget *make_dialogs_page(showcase_state *state) {
  GDateTime *now = g_date_time_new_now_local();
  GtkWidget *box = create_box(&(box_config){
      .orientation = GTK_ORIENTATION_VERTICAL,
      .spacing = 10,
      .homogeneous = false,
      .style = {.margin_top = 16,
                .margin_bottom = 16,
                .margin_start = 16,
                .margin_end = 16},
  });

    container_add(box,
          create_button(&(button_config){
            .label = "Open Alert",
            .icon_name = "dialog-information-symbolic",
            .theme_variant = "primary",
            .use_underline = false,
            .has_frame = true,
            .can_shrink = true,
            .icon_size = 16,
            .style = {.width_request = 180},
            .on_clicked = on_show_dialog,
            .user_data = state,
          }),
          0, 0, 0, 0);

    container_add(box,
          create_calendar(&(calendar_config){
            .selected_date = now,
            .show_day_names = true,
            .show_week_numbers = true,
            .no_month_change = false,
            .style = {.margin_top = 8},
            .on_day_selected = on_day_selected,
            .user_data = state,
          }),
          0, 0, 0, 0);
  g_date_time_unref(now);
  return box;
}

static void on_menu_about(GSimpleAction *action, GVariant *parameter,
                          gpointer user_data) {
  (void)action;
  (void)parameter;
  GtkApplication *app = GTK_APPLICATION(user_data);
  GtkWindow *parent = gtk_application_get_active_window(app);

  show_alert_dialog(&(alert_dialog_config){
      .parent = parent,
      .message = "GTK UI Wrapper Showcase",
      .detail = "Menu actions now use direct callbacks from menu structs.",
      .buttons = NULL,
      .button_count = 0,
      .default_button = -1,
      .cancel_button = -1,
      .modal = true,
      .on_response = NULL,
      .user_data = NULL,
  });
}

static void on_menu_quit(GSimpleAction *action, GVariant *parameter,
                         gpointer user_data) {
  (void)action;
  (void)parameter;
  g_application_quit(G_APPLICATION(user_data));
}

static GtkWidget *build_showcase(GtkApplication *app, GtkWindow *window,
                                 showcase_state *state) {
  (void)window;
  GtkWidget *root = create_box(&(box_config){
      .orientation = GTK_ORIENTATION_VERTICAL,
      .spacing = 0,
      .homogeneous = false,
      .style = {.hexpand = true,
                .set_hexpand = true,
                .vexpand = true,
                .set_vexpand = true},
  });

  const menu_item_config recent_children[] = {
      {.label = "Project A",
       .kind = MENU_ITEM_ACTION,
       .children = NULL,
       .child_count = 0,
       .enabled = true,
       .on_activate = on_menu_about,
       .user_data = app},
      {.label = "Project B",
       .kind = MENU_ITEM_ACTION,
       .children = NULL,
       .child_count = 0,
       .enabled = true,
       .on_activate = on_menu_about,
       .user_data = app},
  };
  const menu_item_config export_children[] = {
      {.label = "PNG",
       .kind = MENU_ITEM_ACTION,
       .children = NULL,
       .child_count = 0,
       .enabled = true,
       .on_activate = on_menu_about,
       .user_data = app},
      {.label = "PDF",
       .kind = MENU_ITEM_ACTION,
       .children = NULL,
       .child_count = 0,
       .enabled = true,
       .on_activate = on_menu_about,
       .user_data = app},
  };
  const menu_item_config file_items[] = {
      {.label = "New",
       .kind = MENU_ITEM_ACTION,
       .children = NULL,
       .child_count = 0,
       .enabled = true,
       .on_activate = on_menu_about,
       .user_data = app},
      {.label = "Open",
       .kind = MENU_ITEM_ACTION,
       .children = NULL,
       .child_count = 0,
       .enabled = true,
       .on_activate = on_menu_about,
       .user_data = app},
      {.label = "Recent",
       .kind = MENU_ITEM_SUBMENU,
       .children = recent_children,
       .child_count = G_N_ELEMENTS(recent_children),
       .enabled = true,
       .on_activate = NULL,
       .user_data = NULL},
      {.label = "Export",
       .kind = MENU_ITEM_SUBMENU,
       .children = export_children,
       .child_count = G_N_ELEMENTS(export_children),
       .enabled = true,
       .on_activate = NULL,
       .user_data = NULL},
      {.label = "Quit",
       .kind = MENU_ITEM_ACTION,
       .children = NULL,
       .child_count = 0,
       .enabled = true,
       .on_activate = on_menu_quit,
       .user_data = app},
  };
  const menu_item_config help_items[] = {
      {.label = "About",
       .kind = MENU_ITEM_ACTION,
       .children = NULL,
       .child_count = 0,
       .enabled = true,
       .on_activate = on_menu_about,
       .user_data = app},
  };
  const menu_section_config sections[] = {
      {.label = "File",
       .items = file_items,
       .item_count = G_N_ELEMENTS(file_items)},
      {.label = "Help",
       .items = help_items,
       .item_count = G_N_ELEMENTS(help_items)},
  };
    container_add(root,
          create_menubar(&(menubar_config){
            .sections = sections,
            .section_count = G_N_ELEMENTS(sections),
            .action_map = G_ACTION_MAP(app),
            .layout = MENU_LAYOUT_VERTICAL,
            .show_arrow_indicators = true,
          }),
          0, 0, 0, 0);

  GtkWidget *hbox = create_box(&(box_config){
      .orientation = GTK_ORIENTATION_HORIZONTAL,
      .spacing = 0,
      .homogeneous = false,
      .style = {.hexpand = true,
                .set_hexpand = true,
                .vexpand = true,
                .set_vexpand = true},
  });
  GtkWidget *stack = create_stack(&(stack_config){
      .transition = GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN,
      .duration_ms = 280,
      .vhomogeneous = false,
      .hhomogeneous = false,
      .style = {.hexpand = true,
                .set_hexpand = true,
                .vexpand = true,
                .set_vexpand = true},
  });
  GtkWidget *sidebar = create_stack_sidebar(stack, &(widget_style_config){
                                                       .margin_top = 10,
                                                       .margin_bottom = 10,
                                                       .margin_start = 10,
                                                       .margin_end = 6,
                                                   });

  stack_add_page(stack, make_inputs_page(state), "inputs", "Inputs");
  stack_add_page(stack, make_buttons_page(state), "buttons", "Buttons");
  stack_add_page(stack, make_toggles_page(state), "toggles", "Toggles");
  stack_add_page(stack, make_displays_page(state), "displays", "Displays");
  stack_add_page(stack, make_dialogs_page(state), "dialogs", "Dialogs");

  container_add(hbox, sidebar, 0, 0, 0, 0);
  container_add(hbox, stack, 0, 0, 0, 0);
  container_add(root, hbox, 0, 0, 0, 0);

    container_add(root,
          create_separator(&(separator_config){
            .orientation = GTK_ORIENTATION_HORIZONTAL}),
          0, 0, 0, 0);
  state->status_label = create_label(&(label_config){
      .text = "Ready",
      .selectable = false,
      .wrap = false,
      .ellipsize = PANGO_ELLIPSIZE_END,
      .max_width_chars = 96,
      .style = {.margin_top = 8,
                .margin_bottom = 8,
                .margin_start = 12,
                .margin_end = 12},
  });
  container_add(root, state->status_label, 0, 0, 0, 0);
  return root;
}

void demo_app_start(GtkApplication *app, gpointer user_data) {
  (void)user_data;
  static bool once = false;
  if (!once) {
    apply_demo_theme();
    once = true;
  }

  GtkWidget *window =
      create_window(app, &(window_config){
                             .title = "GTK UI Wrapper Showcasea",
                              .icon_name = "applications-development-symbolic",
                              .widget_name = "wrapper-window",
                              // .background_color = "#86cde7",
                              .background_image_path = NULL,
                             .default_width = 700,
                             .default_height = 300,
                             .min_width = 400,
                             .min_height = 300,
                             .max_width = 0,
                             .max_height = 0,
                             .resizable = true,
                             .decorated = true,
                             .modal = false,
                             .maximized = false,
                             .present_on_create = false,
                             .style = {0},
                         });

  showcase_state *state = g_new0(showcase_state, 1);

  gtk_window_set_child(GTK_WINDOW(window),
                       build_showcase(app, GTK_WINDOW(window), state));
  gtk_window_present(GTK_WINDOW(window));
}
