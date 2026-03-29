#ifndef GTK_WRAPPER_XML_PARSER_H
#define GTK_WRAPPER_XML_PARSER_H

#include <gtk/gtk.h>
#include <stdbool.h>

/* ── Callback signatures matching the wrapper patterns ── */
typedef void (*xml_button_callback)(GtkButton *button, gpointer user_data);
typedef void (*xml_entry_callback)(GtkEditable *editable, gpointer user_data);
typedef void (*xml_switch_callback)(GtkSwitch *sw, GParamSpec *pspec, gpointer user_data);
typedef void (*xml_checkbox_callback)(GtkCheckButton *button, gpointer user_data);
typedef void (*xml_spin_callback)(GtkSpinButton *spin, gpointer user_data);
typedef void (*xml_dropdown_callback)(GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data);
typedef void (*xml_calendar_callback)(GtkCalendar *calendar, gpointer user_data);

/* ── Callback registry entry ── */
typedef struct {
    const char *name;
    void       *func;
} xml_callback_entry;

/* ── Parse context passed by the caller ── */
typedef struct {
    GtkApplication        *app;
    gpointer               user_data;
    const xml_callback_entry *callbacks;
    size_t                  callback_count;
} xml_parse_context;

/* ── Public API ── */

/*
 * Parse an XML file and build the full widget tree.
 * Returns the top-level GtkWidget* (typically from <window>).
 * The caller must present the window after this call.
 */
GtkWidget *xml_parse_file(const char *filename, const xml_parse_context *ctx);

/*
 * Convenience: look up a widget by its id attribute after parsing.
 * Returns NULL if not found.
 */
GtkWidget *xml_get_widget_by_id(const char *id);

#endif
