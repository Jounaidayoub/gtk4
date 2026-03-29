#include "widgets/xml_parser.h"

#include "widgets/button.h"
#include "widgets/container.h"
#include "widgets/display.h"
#include "widgets/input.h"
#include "widgets/separator.h"
#include "widgets/toggle.h"
#include "widgets/window.h"
#include "widgets/date.h"
#include "widgets/common.h"

#include <string.h>
#include <stdlib.h>

/* ── Max nesting depth for the widget tree ── */
#define MAX_DEPTH 32

/* ── ID registry (flat table, filled during parse) ── */
#define MAX_IDS 128

typedef struct {
    const char *id;
    GtkWidget  *widget;
} id_entry;

static id_entry  g_id_table[MAX_IDS];
static int        g_id_count = 0;

static void register_id(const char *id, GtkWidget *widget) {
    if (!id || g_id_count >= MAX_IDS) return;
    g_id_table[g_id_count].id     = g_strdup(id);
    g_id_table[g_id_count].widget = widget;
    g_id_count++;
}

GtkWidget *xml_get_widget_by_id(const char *id) {
    for (int i = 0; i < g_id_count; i++) {
        if (g_strcmp0(g_id_table[i].id, id) == 0)
            return g_id_table[i].widget;
    }
    return NULL;
}

/* ── Internal parser state ── */
typedef struct {
    const xml_parse_context *ctx;

    GtkWidget *stack[MAX_DEPTH];
    int        depth;

    GtkWidget *result;       /* final top-level widget */

    /* For dropdown options memory management */
    GPtrArray *string_arrays;
} parser_state;

/* ── Helpers ── */

static const char *find_attr(const gchar **names, const gchar **values,
                             const char *key) {
    for (int i = 0; names[i]; i++) {
        if (g_strcmp0(names[i], key) == 0)
            return values[i];
    }
    return NULL;
}

static int attr_int(const gchar **names, const gchar **values,
                    const char *key, int fallback) {
    const char *v = find_attr(names, values, key);
    return v ? atoi(v) : fallback;
}

static double attr_double(const gchar **names, const gchar **values,
                          const char *key, double fallback) {
    const char *v = find_attr(names, values, key);
    return v ? atof(v) : fallback;
}

static bool attr_bool(const gchar **names, const gchar **values,
                      const char *key, bool fallback) {
    const char *v = find_attr(names, values, key);
    if (!v) return fallback;
    return (g_strcmp0(v, "true") == 0 || g_strcmp0(v, "1") == 0);
}

static GtkOrientation attr_orientation(const gchar **names,
                                       const gchar **values,
                                       const char *key,
                                       GtkOrientation fallback) {
    const char *v = find_attr(names, values, key);
    if (!v) return fallback;
    if (g_strcmp0(v, "horizontal") == 0) return GTK_ORIENTATION_HORIZONTAL;
    if (g_strcmp0(v, "vertical") == 0)   return GTK_ORIENTATION_VERTICAL;
    return fallback;
}

static GtkAlign attr_align(const gchar **names,
                           const gchar **values,
                           const char *key,
                           GtkAlign fallback) {
    const char *v = find_attr(names, values, key);
    if (!v) return fallback;
    if (g_strcmp0(v, "start") == 0) return GTK_ALIGN_START;
    if (g_strcmp0(v, "center") == 0) return GTK_ALIGN_CENTER;
    if (g_strcmp0(v, "end") == 0) return GTK_ALIGN_END;
    if (g_strcmp0(v, "fill") == 0) return GTK_ALIGN_FILL;
    return fallback;
}

/* Parse the style sub-attributes from the element attributes. */
static widget_style_config parse_style(const gchar **names,
                                       const gchar **values) {
    widget_style_config s = {0};
    s.margin_top    = attr_int(names, values, "style-margin-top", 0);
    s.margin_bottom = attr_int(names, values, "style-margin-bottom", 0);
    s.margin_start  = attr_int(names, values, "style-margin-start", 0);
    s.margin_end    = attr_int(names, values, "style-margin-end", 0);
    s.width_request  = attr_int(names, values, "style-width", 0);
    s.height_request = attr_int(names, values, "style-height", 0);
    
    if (find_attr(names, values, "style-halign")) {
        s.halign = attr_align(names, values, "style-halign", GTK_ALIGN_FILL);
        s.set_halign = true;
    }
    if (find_attr(names, values, "style-valign")) {
        s.valign = attr_align(names, values, "style-valign", GTK_ALIGN_FILL);
        s.set_valign = true;
    }

    if (find_attr(names, values, "style-hexpand")) {
        s.hexpand = attr_bool(names, values, "style-hexpand", false);
        s.set_hexpand = true;
    }
    if (find_attr(names, values, "style-vexpand")) {
        s.vexpand = attr_bool(names, values, "style-vexpand", false);
        s.set_vexpand = true;
    }

    const char *css = find_attr(names, values, "style-css-class");
    if (css) s.css_class = css;

    const char *tip = find_attr(names, values, "tooltip");
    if (tip) s.tooltip = tip;

    return s;
}

/* Look up a callback by name from the registry */
static void *lookup_callback(const parser_state *ps, const char *name) {
    if (!name || !ps->ctx->callbacks) return NULL;
    for (size_t i = 0; i < ps->ctx->callback_count; i++) {
        if (g_strcmp0(ps->ctx->callbacks[i].name, name) == 0)
            return ps->ctx->callbacks[i].func;
    }
    g_warning("xml_parser: callback '%s' not found in registry", name);
    return NULL;
}

/* Push a container onto the stack so children get added to it */
static void push(parser_state *ps, GtkWidget *widget) {
    if (ps->depth >= MAX_DEPTH) {
        g_warning("xml_parser: max nesting depth exceeded");
        return;
    }
    ps->stack[ps->depth++] = widget;
}

/* Current parent container (top of stack) */
static GtkWidget *current_parent(parser_state *ps) {
    return ps->depth > 0 ? ps->stack[ps->depth - 1] : NULL;
}

/* Add a newly-created widget to the current parent container */
static void add_to_parent(parser_state *ps, GtkWidget *widget, 
                          const gchar **names, const gchar **values) {
    GtkWidget *parent = current_parent(ps);
    if (parent) {
        int col = attr_int(names, values, "layout-col", 0);
        int row = attr_int(names, values, "layout-row", 0);
        int colspan = attr_int(names, values, "layout-colspan", 1);
        int rowspan = attr_int(names, values, "layout-rowspan", 1);
        container_add(parent, widget, col, row, colspan, rowspan);
    }
}

/* ── Element handlers ── */

static GtkWidget *handle_window(parser_state *ps,
                                const gchar **names, const gchar **values) {
    window_config cfg = {
        .title          = find_attr(names, values, "title"),
        .icon_name      = find_attr(names, values, "icon-name"),
        .widget_name    = find_attr(names, values, "widget-name"),
        .background_image_path = find_attr(names, values, "background-image"),
        .default_width  = attr_int(names, values, "default-width", 800),
        .default_height = attr_int(names, values, "default-height", 600),
        .min_width      = attr_int(names, values, "min-width", 0),
        .min_height     = attr_int(names, values, "min-height", 0),
        .max_width      = attr_int(names, values, "max-width", 0),
        .max_height     = attr_int(names, values, "max-height", 0),
        .resizable      = attr_bool(names, values, "resizable", true),
        .decorated      = attr_bool(names, values, "decorated", true),
        .modal          = attr_bool(names, values, "modal", false),
        .maximized      = attr_bool(names, values, "maximized", false),
        .present_on_create = false,
        .style          = parse_style(names, values),
    };
    return create_window(ps->ctx->app, &cfg);
}

static GtkWidget *handle_box(parser_state *ps,
                             const gchar **names, const gchar **values) {
    (void)ps;
    box_config cfg = {
        .orientation = attr_orientation(names, values, "orientation", GTK_ORIENTATION_VERTICAL),
        .spacing     = attr_int(names, values, "spacing", 0),
        .homogeneous = attr_bool(names, values, "homogeneous", false),
        .style       = parse_style(names, values),
        .align       = attr_align(names, values, "align", GTK_ALIGN_FILL),
    };
    return create_box(&cfg);
}

static GtkWidget *handle_grid(parser_state *ps,
                              const gchar **names, const gchar **values) {
    (void)ps;
    grid_config cfg = {
        .column_spacing       = attr_int(names, values, "column-spacing", 0),
        .row_spacing          = attr_int(names, values, "row-spacing", 0),
        .homogeneous_columns  = attr_bool(names, values, "homogeneous-columns", false),
        .homogeneous_rows     = attr_bool(names, values, "homogeneous-rows", false),
        .style                = parse_style(names, values),
    };
    return create_grid(&cfg);
}

static GtkWidget *handle_stack(parser_state *ps,
                               const gchar **names, const gchar **values) {
    (void)ps;
    
    const char *trans_str = find_attr(names, values, "transition");
    GtkStackTransitionType trans = GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN;
    if (g_strcmp0(trans_str, "none") == 0) trans = GTK_STACK_TRANSITION_TYPE_NONE;
    else if (g_strcmp0(trans_str, "crossfade") == 0) trans = GTK_STACK_TRANSITION_TYPE_CROSSFADE;
    else if (g_strcmp0(trans_str, "slide") == 0) trans = GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT;

    stack_config cfg = {
        .transition    = trans,
        .duration_ms   = (guint)attr_int(names, values, "duration-ms", 280),
        .vhomogeneous  = attr_bool(names, values, "vhomogeneous", false),
        .hhomogeneous  = attr_bool(names, values, "hhomogeneous", false),
        .style         = parse_style(names, values),
    };
    return create_stack(&cfg);
}

static GtkWidget *handle_stack_sidebar(parser_state *ps,
                               const gchar **names, const gchar **values) {
    const char *stack_id = find_attr(names, values, "stack-id");
    GtkWidget *stack = xml_get_widget_by_id(stack_id);
    if (!stack) {
        g_warning("xml_parser: stack-sidebar requires a valid stack-id");
        return NULL;
    }
    
    widget_style_config style = parse_style(names, values);
    return create_stack_sidebar(stack, &style);
}

static GtkWidget *handle_button(parser_state *ps,
                                const gchar **names, const gchar **values) {
    const char *cb_name = find_attr(names, values, "on-clicked");
    button_config cfg = {
        .label         = find_attr(names, values, "label"),
        .icon_name     = find_attr(names, values, "icon-name"),
        .theme_variant = find_attr(names, values, "theme-variant"),
        .use_underline = attr_bool(names, values, "use-underline", false),
        .has_frame     = attr_bool(names, values, "has-frame", true),
        .can_shrink    = attr_bool(names, values, "can-shrink", false),
        .icon_size     = attr_int(names, values, "icon-size", 0),
        .style         = parse_style(names, values),
        .on_clicked    = (xml_button_callback)lookup_callback(ps, cb_name),
        .user_data     = ps->ctx->user_data,
    };
    return create_button(&cfg);
}

static GtkWidget *handle_entry(parser_state *ps,
                               const gchar **names, const gchar **values) {
    const char *cb_name = find_attr(names, values, "on-changed");
    entry_config cfg = {
        .placeholder  = find_attr(names, values, "placeholder"),
        .default_text = find_attr(names, values, "default-text"),
        .purpose_hint = find_attr(names, values, "purpose-hint"),
        .password     = attr_bool(names, values, "password", false),
        .editable     = attr_bool(names, values, "editable", true),
        .max_length   = attr_int(names, values, "max-length", 0),
        .style        = parse_style(names, values),
        .on_changed   = (xml_entry_callback)lookup_callback(ps, cb_name),
        .user_data    = ps->ctx->user_data,
    };
    return create_entry(&cfg);
}

static GtkWidget *handle_spin_button(parser_state *ps,
                                     const gchar **names,
                                     const gchar **values) {
    const char *cb_name = find_attr(names, values, "on-value-changed");
    spin_button_config cfg = {
        .min_value    = attr_double(names, values, "min", 0),
        .max_value    = attr_double(names, values, "max", 100),
        .step         = attr_double(names, values, "step", 1),
        .value        = attr_double(names, values, "value", 0),
        .digits       = (guint)attr_int(names, values, "digits", 0),
        .numeric_only = attr_bool(names, values, "numeric-only", false),
        .wrap         = attr_bool(names, values, "wrap", false),
        .style        = parse_style(names, values),
        .on_value_changed = (xml_spin_callback)lookup_callback(ps, cb_name),
        .user_data    = ps->ctx->user_data,
    };
    return create_spin_button(&cfg);
}

static GtkWidget *handle_dropdown(parser_state *ps,
                                  const gchar **names, const gchar **values) {
    const char *cb_name = find_attr(names, values, "on-selected");
    const char *options_str = find_attr(names, values, "options");
    
    char **string_array = NULL;
    if (options_str) {
        string_array = g_strsplit(options_str, ",", -1);
        g_ptr_array_add(ps->string_arrays, string_array); // Schedule for cleanup
    }

    dropdown_config cfg = {
        .options = (const char **)string_array,
        .selected_index = (guint)attr_int(names, values, "selected-index", 0),
        .enable_search = attr_bool(names, values, "enable-search", false),
        .style = parse_style(names, values),
        .on_selected = (xml_dropdown_callback)lookup_callback(ps, cb_name),
        .user_data = ps->ctx->user_data,
    };
    return create_dropdown(&cfg);
}

static GtkWidget *handle_calendar(parser_state *ps,
                                  const gchar **names, const gchar **values) {
    const char *cb_name = find_attr(names, values, "on-day-selected");
    GDateTime *now = g_date_time_new_now_local(); // Use current date for simplicity by default
    
    calendar_config cfg = {
        .selected_date = now,
        .show_day_names = attr_bool(names, values, "show-day-names", true),
        .show_week_numbers = attr_bool(names, values, "show-week-numbers", false),
        .no_month_change = attr_bool(names, values, "no-month-change", false),
        .style = parse_style(names, values),
        .on_day_selected = (xml_calendar_callback)lookup_callback(ps, cb_name),
        .user_data = ps->ctx->user_data,
    };
    GtkWidget *cal = create_calendar(&cfg);
    g_date_time_unref(now);
    return cal;
}

static GtkWidget *handle_label(parser_state *ps,
                               const gchar **names, const gchar **values) {
    (void)ps;
    label_config cfg = {
        .text            = find_attr(names, values, "text"),
        .selectable      = attr_bool(names, values, "selectable", false),
        .wrap            = attr_bool(names, values, "wrap", false),
        .max_width_chars = attr_int(names, values, "max-width-chars", -1),
        .style           = parse_style(names, values),
        .ellipsize       = find_attr(names, values, "ellipsize") ? PANGO_ELLIPSIZE_END : PANGO_ELLIPSIZE_NONE,
    };
    return create_label(&cfg);
}

static GtkWidget *handle_image(parser_state *ps,
                               const gchar **names, const gchar **values) {
    (void)ps;
    image_config cfg = {
        .icon_name  = find_attr(names, values, "icon-name"),
        .file_path  = find_attr(names, values, "file-path"),
        .pixel_size = attr_int(names, values, "pixel-size", 0),
        .keep_aspect = attr_bool(names, values, "keep-aspect", true),
        .style       = parse_style(names, values),
    };
    return create_image(&cfg);
}

static GtkWidget *handle_progress_bar(parser_state *ps,
                                      const gchar **names,
                                      const gchar **values) {
    (void)ps;
    progress_bar_config cfg = {
        .fraction   = attr_double(names, values, "fraction", 0),
        .text       = find_attr(names, values, "text"),
        .show_text  = attr_bool(names, values, "show-text", false),
        .inverted   = attr_bool(names, values, "inverted", false),
        .style      = parse_style(names, values),
    };
    return create_progress_bar(&cfg);
}

static GtkWidget *handle_spinner(parser_state *ps,
                                 const gchar **names, const gchar **values) {
    (void)ps;
    spinner_config cfg = {
        .size     = attr_int(names, values, "size", 32),
        .spinning = attr_bool(names, values, "spinning", true),
        .style    = parse_style(names, values),
    };
    return create_spinner(&cfg);
}

static GtkWidget *handle_separator(parser_state *ps,
                                   const gchar **names,
                                   const gchar **values) {
    (void)ps;
    separator_config cfg = {
        .orientation = attr_orientation(names, values, "orientation",
                                        GTK_ORIENTATION_HORIZONTAL),
        .style       = parse_style(names, values),
    };
    return create_separator(&cfg);
}

static GtkWidget *handle_checkbox(parser_state *ps,
                                  const gchar **names,
                                  const gchar **values) {
    const char *cb_name = find_attr(names, values, "on-toggled");
    checkbox_config cfg = {
        .label        = find_attr(names, values, "label"),
        .active       = attr_bool(names, values, "active", false),
        .inconsistent = attr_bool(names, values, "inconsistent", false),
        .style        = parse_style(names, values),
        .on_toggled   = (xml_checkbox_callback)lookup_callback(ps, cb_name),
        .user_data    = ps->ctx->user_data,
    };
    return create_checkbox(&cfg);
}

static GtkWidget *handle_switch(parser_state *ps,
                                const gchar **names,
                                const gchar **values) {
    const char *cb_name = find_attr(names, values, "on-active-changed");
    const char *id = find_attr(names, values, "id");
    
    switch_config cfg = {
        .label  = find_attr(names, values, "label"),
        .active = attr_bool(names, values, "active", false),
        .state  = attr_bool(names, values, "state", false),
        .style  = parse_style(names, values),
        .on_active_changed = (xml_switch_callback)lookup_callback(ps, cb_name),
        .user_data         = ps->ctx->user_data,
    };
    
    GtkWidget *switch_widget = NULL;
    GtkWidget *row = create_switch_row(&cfg, &switch_widget);
    
    if (id && switch_widget) {
        /* Usually the id maps to the row, but if they want the inner switch they 
         * should use another id. For now we register the row. */
    }
    
    return row;
}

static GtkWidget *handle_radio_button(parser_state *ps,
                                      const gchar **names,
                                      const gchar **values) {
    const char *cb_name = find_attr(names, values, "on-toggled");
    const char *group_id = find_attr(names, values, "group-with");
    GtkWidget *group_widget = group_id ? xml_get_widget_by_id(group_id) : NULL;

    radio_button_config cfg = {
        .label  = find_attr(names, values, "label"),
        .group_with = group_widget,
        .is_active = attr_bool(names, values, "is-active", false),
        .style  = parse_style(names, values),
        .on_toggled = (void *)lookup_callback(ps, cb_name),
        .user_data         = ps->ctx->user_data,
    };
    return create_radio_button(&cfg);
}

/* ── GMarkupParser callbacks ── */

static void xml_start_element(GMarkupParseContext *context,
                              const gchar         *element_name,
                              const gchar        **attribute_names,
                              const gchar        **attribute_values,
                              gpointer             user_data,
                              GError             **error) {
    (void)context;
    (void)error;
    parser_state *ps = user_data;
    GtkWidget *widget = NULL;
    bool is_container = false;

    /* ── Containers (push onto stack) ── */
    if (g_strcmp0(element_name, "window") == 0) {
        widget = handle_window(ps, attribute_names, attribute_values);
        is_container = true;
    }
    else if (g_strcmp0(element_name, "box") == 0) {
        widget = handle_box(ps, attribute_names, attribute_values);
        is_container = true;
    }
    else if (g_strcmp0(element_name, "grid") == 0) {
        widget = handle_grid(ps, attribute_names, attribute_values);
        is_container = true;
    }
    else if (g_strcmp0(element_name, "stack") == 0) {
        widget = handle_stack(ps, attribute_names, attribute_values);
        is_container = true;
    }
    /* ── Leaf widgets ── */
    else if (g_strcmp0(element_name, "stack-sidebar") == 0) {
        widget = handle_stack_sidebar(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "button") == 0) {
        widget = handle_button(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "entry") == 0) {
        widget = handle_entry(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "spin-button") == 0) {
        widget = handle_spin_button(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "dropdown") == 0) {
        widget = handle_dropdown(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "calendar") == 0) {
        widget = handle_calendar(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "label") == 0) {
        widget = handle_label(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "image") == 0) {
        widget = handle_image(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "progress-bar") == 0) {
        widget = handle_progress_bar(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "spinner") == 0) {
        widget = handle_spinner(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "separator") == 0) {
        widget = handle_separator(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "checkbox") == 0) {
        widget = handle_checkbox(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "switch") == 0) {
        widget = handle_switch(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "radio-button") == 0) {
        widget = handle_radio_button(ps, attribute_names, attribute_values);
    }
    else if (g_strcmp0(element_name, "stack-page") == 0) {
        is_container = true; /* stack page contains a box with the elements */
        
        GtkWidget *page_wrapper = create_box(&(box_config){
            .orientation = GTK_ORIENTATION_VERTICAL,
            .style = parse_style(attribute_names, attribute_values),
        });
        
        const char *name  = find_attr(attribute_names, attribute_values, "name");
        const char *title = find_attr(attribute_names, attribute_values, "title");
        GtkWidget *parent_stack = current_parent(ps);
        if (parent_stack) {
            stack_add_page(parent_stack, page_wrapper, name ? name : "page", title ? title : "Page");
        }
        
        widget = page_wrapper;
    }
    else {
        g_warning("xml_parser: unknown element '%s'", element_name);
        return;
    }

    if (!widget) return;

    /* Register id if present */
    const char *id = find_attr(attribute_names, attribute_values, "id");
    if (id) register_id(id, widget);

    /* Add to parent, or mark as root */
    if (ps->depth == 0) {
        ps->result = widget;
    } else {
        if (g_strcmp0(element_name, "stack-page") != 0) {
            add_to_parent(ps, widget, attribute_names, attribute_values);
        }
    }

    /* Container elements get pushed so children go inside them */
    if (is_container) {
        push(ps, widget);
    }
}

static void xml_end_element(GMarkupParseContext *context,
                            const gchar         *element_name,
                            gpointer             user_data,
                            GError             **error) {
    (void)context;
    (void)error;
    parser_state *ps = user_data;

    /* Pop from stack when closing a container element */
    if (g_strcmp0(element_name, "window") == 0 ||
        g_strcmp0(element_name, "box") == 0 ||
        g_strcmp0(element_name, "grid") == 0 ||
        g_strcmp0(element_name, "stack") == 0 ||
        g_strcmp0(element_name, "stack-page") == 0) {
        if (ps->depth > 0)
            ps->depth--;
    }
}

/* ── Public API ── */

GtkWidget *xml_parse_file(const char *filename, const xml_parse_context *ctx) {
    g_return_val_if_fail(filename != NULL, NULL);
    g_return_val_if_fail(ctx != NULL, NULL);
    g_return_val_if_fail(ctx->app != NULL, NULL);

    /* Reset ID table */
    for (int i = 0; i < g_id_count; i++)
        g_free((char *)g_id_table[i].id);
    g_id_count = 0;

    /* Read file */
    gchar *content = NULL;
    gsize length = 0;
    GError *err = NULL;
    if (!g_file_get_contents(filename, &content, &length, &err)) {
        g_warning("xml_parser: failed to read '%s': %s", filename, err->message);
        g_error_free(err);
        return NULL;
    }

    /* Set up parser */
    parser_state ps = {
        .ctx   = ctx,
        .depth = 0,
        .result = NULL,
        .string_arrays = g_ptr_array_new(),
    };
    memset(ps.stack, 0, sizeof(ps.stack));

    GMarkupParser parser = {
        .start_element = xml_start_element,
        .end_element   = xml_end_element,
        .text          = NULL,
        .passthrough   = NULL,
        .error         = NULL,
    };

    GMarkupParseContext *parse_ctx =
        g_markup_parse_context_new(&parser, 0, &ps, NULL);

    if (!g_markup_parse_context_parse(parse_ctx, content, (gssize)length, &err)) {
        g_warning("xml_parser: parse error: %s", err->message);
        g_error_free(err);
    }

    g_markup_parse_context_end_parse(parse_ctx, NULL);
    g_markup_parse_context_free(parse_ctx);
    g_free(content);
    
    // Cleanup string arrays created during parse
    for (guint i = 0; i < ps.string_arrays->len; i++) {
        g_strfreev(g_ptr_array_index(ps.string_arrays, i));
    }
    g_ptr_array_free(ps.string_arrays, TRUE);

    return ps.result;
}
