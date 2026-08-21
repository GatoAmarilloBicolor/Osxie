#include "osxie_dbus.h"
#include <dbus/dbus.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static DBusConnection *g_bus = NULL;
static int g_available = 0;
static int g_initialized = 0;

int osxie_dbus_init(void) {
    if (g_initialized)
        return g_available;
    g_initialized = 1;

    DBusError err;
    dbus_error_init(&err);

    g_bus = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "[DBUS] session bus connect failed: %s\n", err.message);
        dbus_error_free(&err);
        return 0;
    }
    if (!g_bus) {
        fprintf(stderr, "[DBUS] session bus is NULL\n");
        return 0;
    }

    dbus_connection_set_exit_on_disconnect(g_bus, FALSE);
    g_available = 1;
    fprintf(stderr, "[DBUS] connected to session bus\n");
    return 1;
}

void osxie_dbus_dispatch(void) {
    if (!g_bus) return;
    while (dbus_connection_dispatch(g_bus) == DBUS_DISPATCH_DATA_REMAINS) {}
    dbus_connection_flush(g_bus);
}

int osxie_dbus_is_available(void) {
    return g_available;
}

static DBusConnection *get_bus(void) {
    if (!g_bus && !g_initialized)
        osxie_dbus_init();
    return g_bus;
}

static uint32_t next_id = 1;

static void append_variant_string(DBusMessageIter *iter, const char *value) {
    DBusMessageIter var;
    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &var);
    dbus_message_iter_append_basic(&var, DBUS_TYPE_STRING, &value);
    dbus_message_iter_close_container(iter, &var);
}

static void append_variant_int32(DBusMessageIter *iter, int32_t value) {
    DBusMessageIter var;
    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_INT32_AS_STRING, &var);
    dbus_message_iter_append_basic(&var, DBUS_TYPE_INT32, &value);
    dbus_message_iter_close_container(iter, &var);
}

static void append_variant_bool(DBusMessageIter *iter, int value) {
    DBusMessageIter var;
    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &var);
    dbus_message_iter_append_basic(&var, DBUS_TYPE_BOOLEAN, &value);
    dbus_message_iter_close_container(iter, &var);
}

static void append_variant_byte(DBusMessageIter *iter, uint8_t value) {
    DBusMessageIter var;
    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_BYTE_AS_STRING, &var);
    dbus_message_iter_append_basic(&var, DBUS_TYPE_BYTE, &value);
    dbus_message_iter_close_container(iter, &var);
}

/* ==================== DBusMenu ==================== */

typedef struct menu_item {
    int id;
    char *label;
    char *icon_name;
    int enabled;
    int visible;
    int is_separator;
    int has_submenu;
    struct menu_item *children;
    int n_children;
    int toggle_type;
    int toggle_state;
    char *shortcut_key;
} menu_item_t;

struct osxie_dbus_menu {
    char *bus_name;
    char *object_path;
    menu_item_t *items;
    int n_items;
    int revision;
    uint32_t registered_id;
    osxie_dbus_menu_action_cb action_cb;
    void *action_userdata;
};

static int menu_item_id_counter = 1;

static void free_menu_items(menu_item_t *items, int count) {
    for (int i = 0; i < count; i++) {
        free(items[i].label);
        free(items[i].icon_name);
        free(items[i].shortcut_key);
        if (items[i].children)
            free_menu_items(items[i].children, items[i].n_children);
    }
    free(items);
}

osxie_dbus_menu_t *osxie_dbus_menu_new(const char *app_name, const char *icon_name) {
    osxie_dbus_menu_t *menu = calloc(1, sizeof(*menu));
    menu->bus_name = NULL;
    menu->object_path = NULL;
    menu->revision = 1;
    return menu;
}

void osxie_dbus_menu_free(osxie_dbus_menu_t *menu) {
    if (!menu) return;
    free(menu->bus_name);
    free(menu->object_path);
    if (menu->items)
        free_menu_items(menu->items, menu->n_items);
    free(menu);
}

static void build_layout_reply(DBusMessage *reply, osxie_dbus_menu_t *menu) {
    DBusMessageIter args, ret;
    dbus_message_iter_init_append(reply, &args);

    dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &menu->revision);

    DBusMessageIter layout;
    dbus_message_iter_open_container(&args, DBUS_TYPE_STRUCT, NULL, &layout);

    int32_t root_id = 0;
    dbus_message_iter_append_basic(&layout, DBUS_TYPE_INT32, &root_id);

    DBusMessageIter root_props;
    dbus_message_iter_open_container(&layout, DBUS_TYPE_ARRAY, "{sv}", &root_props);

    const char *root_type = "standard";
    DBusMessageIter prop_entry, prop_val;
    dbus_message_iter_open_container(&root_props, DBUS_TYPE_DICT_ENTRY, NULL, &prop_entry);
    dbus_message_iter_append_basic(&prop_entry, DBUS_TYPE_STRING, &root_type);
    append_variant_string(&prop_val, "standard");
    dbus_message_iter_open_container(&prop_entry, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &prop_val);
    dbus_message_iter_append_basic(&prop_val, DBUS_TYPE_STRING, &root_type);
    dbus_message_iter_close_container(&prop_entry, &prop_val);
    dbus_message_iter_close_container(&root_props, &prop_entry);

    dbus_message_iter_close_container(&layout, &root_props);

    DBusMessageIter children;
    dbus_message_iter_open_container(&layout, DBUS_TYPE_ARRAY, "(ia{sv}av)", &children);

    for (int i = 0; i < menu->n_items; i++) {
        menu_item_t *item = &menu->items[i];
        DBusMessageIter child;
        dbus_message_iter_open_container(&children, DBUS_TYPE_STRUCT, NULL, &child);

        dbus_message_iter_append_basic(&child, DBUS_TYPE_INT32, &item->id);

        DBusMessageIter props;
        dbus_message_iter_open_container(&child, DBUS_TYPE_ARRAY, "{sv}", &props);

        const char *label = item->label ? item->label : "";
        DBusMessageIter entry;

        dbus_message_iter_open_container(&props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
        const char *key = "label";
        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
        append_variant_string(&entry, label);
        dbus_message_iter_close_container(&props, &entry);

        dbus_message_iter_open_container(&props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
        key = "enabled";
        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
        append_variant_bool(&entry, item->enabled);
        dbus_message_iter_close_container(&props, &entry);

        dbus_message_iter_open_container(&props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
        key = "visible";
        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
        append_variant_bool(&entry, item->visible);
        dbus_message_iter_close_container(&props, &entry);

        if (item->icon_name && item->icon_name[0]) {
            dbus_message_iter_open_container(&props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
            key = "icon-name";
            dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
            append_variant_string(&entry, item->icon_name);
            dbus_message_iter_close_container(&props, &entry);
        }

        if (item->is_separator) {
            dbus_message_iter_open_container(&props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
            key = "type";
            dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
            const char *sep = "separator";
            append_variant_string(&entry, sep);
            dbus_message_iter_close_container(&props, &entry);
        }

        if (item->toggle_type) {
            dbus_message_iter_open_container(&props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
            key = "toggle-type";
            dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
            const char *ttype = item->toggle_type == 1 ? "checkmark" : "radio";
            append_variant_string(&entry, ttype);
            dbus_message_iter_close_container(&props, &entry);

            dbus_message_iter_open_container(&props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
            key = "toggle-state";
            dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
            append_variant_int32(&entry, item->toggle_state);
            dbus_message_iter_close_container(&props, &entry);
        }

        dbus_message_iter_close_container(&child, &props);

        DBusMessageIter child_items;
        if (item->has_submenu) {
            dbus_message_iter_open_container(&child, DBUS_TYPE_ARRAY, "(ia{sv}av)", &child_items);
            for (int j = 0; j < item->n_children; j++) {
                menu_item_t *sub = &item->children[j];
                DBusMessageIter sub_child;
                dbus_message_iter_open_container(&child_items, DBUS_TYPE_STRUCT, NULL, &sub_child);
                dbus_message_iter_append_basic(&sub_child, DBUS_TYPE_INT32, &sub->id);

                DBusMessageIter sub_props;
                dbus_message_iter_open_container(&sub_child, DBUS_TYPE_ARRAY, "{sv}", &sub_props);

                const char *sl = sub->label ? sub->label : "";
                dbus_message_iter_open_container(&sub_props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                key = "label";
                dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
                append_variant_string(&entry, sl);
                dbus_message_iter_close_container(&sub_props, &entry);

                dbus_message_iter_open_container(&sub_props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                key = "enabled";
                dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
                append_variant_bool(&entry, sub->enabled);
                dbus_message_iter_close_container(&sub_props, &entry);

                dbus_message_iter_open_container(&sub_props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                key = "visible";
                dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
                append_variant_bool(&entry, sub->visible);
                dbus_message_iter_close_container(&sub_props, &entry);

                if (sub->icon_name && sub->icon_name[0]) {
                    dbus_message_iter_open_container(&sub_props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                    key = "icon-name";
                    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
                    append_variant_string(&entry, sub->icon_name);
                    dbus_message_iter_close_container(&sub_props, &entry);
                }

                if (sub->is_separator) {
                    dbus_message_iter_open_container(&sub_props, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                    key = "type";
                    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
                    const char *sep = "separator";
                    append_variant_string(&entry, sep);
                    dbus_message_iter_close_container(&sub_props, &entry);
                }

                dbus_message_iter_close_container(&sub_child, &sub_props);
                DBusMessageIter empty_arr;
                dbus_message_iter_open_container(&sub_child, DBUS_TYPE_ARRAY, "(ia{sv}av)", &empty_arr);
                dbus_message_iter_close_container(&sub_child, &empty_arr);
                dbus_message_iter_close_container(&child_items, &sub_child);
            }
            dbus_message_iter_close_container(&child, &child_items);
        } else {
            dbus_message_iter_open_container(&child, DBUS_TYPE_ARRAY, "(ia{sv}av)", &child_items);
            dbus_message_iter_close_container(&child, &child_items);
        }

        dbus_message_iter_close_container(&children, &child);
    }

    dbus_message_iter_close_container(&layout, &children);
    dbus_message_iter_close_container(&args, &layout);
}

static void free_menu_item_tree(menu_item_t *items, int n) {
    for (int i = 0; i < n; i++) {
        free(items[i].label);
        free(items[i].icon_name);
        free(items[i].shortcut_key);
        if (items[i].children)
            free_menu_item_tree(items[i].children, items[i].n_children);
    }
    free(items);
}

int osxie_dbus_menu_register_window(osxie_dbus_menu_t *menu, uint32_t x11_window_id) {
    DBusConnection *bus = get_bus();
    if (!bus) return -1;

    if (!menu->bus_name) {
        DBusError err;
        dbus_error_init(&err);
        int ret = dbus_bus_request_name(bus, "com.canonical.appmenu.Registrar",
                                        DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
        if (dbus_error_is_set(&err)) {
            fprintf(stderr, "[DBUS] failed to request menu registrar name: %s\n", err.message);
            dbus_error_free(&err);
            return -1;
        }
    }

    DBusMessage *msg = dbus_message_new_method_call(
        "com.canonical.AppMenu.Registrar",
        "/com/canonical/AppMenu/Registrar",
        "com.canonical.AppMenu.Registrar",
        "RegisterWindow");

    if (!msg) return -1;

    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);

    dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &x11_window_id);

    const char *object_path = "/org/osxie/dbusmenu";
    DBusMessageIter obj_var;
    dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, DBUS_TYPE_OBJECT_PATH_AS_STRING, &obj_var);
    dbus_message_iter_append_basic(&obj_var, DBUS_TYPE_OBJECT_PATH, &object_path);
    dbus_message_iter_close_container(&args, &obj_var);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(bus, msg, 2000, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "[DBUS] RegisterWindow failed: %s\n", err.message);
        dbus_error_free(&err);
        return -1;
    }
    if (reply) dbus_message_unref(reply);

    menu->registered_id = x11_window_id;
    fprintf(stderr, "[DBUS] registered menu for window %u\n", x11_window_id);
    return 0;
}

void osxie_dbus_menu_unregister_window(osxie_dbus_menu_t *menu, uint32_t x11_window_id) {
    DBusConnection *bus = get_bus();
    if (!bus) return;

    DBusMessage *msg = dbus_message_new_method_call(
        "com.canonical.AppMenu.Registrar",
        "/com/canonical/AppMenu/Registrar",
        "com.canonical.AppMenu.Registrar",
        "UnregisterWindow");

    if (!msg) return;

    dbus_message_append_args(msg, DBUS_TYPE_UINT32, &x11_window_id, DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(bus, msg, 2000, &err);
    dbus_message_unref(msg);
    if (reply) dbus_message_unref(reply);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "[DBUS] UnregisterWindow failed: %s\n", err.message);
        dbus_error_free(&err);
    }
}

void osxie_dbus_menu_update(osxie_dbus_menu_t *menu) {
    menu->revision++;
}

void osxie_dbus_menu_set_action_callback(osxie_dbus_menu_t *menu, osxie_dbus_menu_action_cb cb, void *userdata) {
    menu->action_cb = cb;
    menu->action_userdata = userdata;
}

/* ==================== StatusNotifierItem ==================== */

#define SNI_INTERFACE "org.kde.StatusNotifierItem"

struct osxie_dbus_tray {
    char *id;
    char *bus_name;
    char *object_path;
    char *icon_name;
    char *tooltip;
    char *title;
    char *status;
    osxie_dbus_tray_activate_cb activate_cb;
    void *activate_userdata;
    osxie_dbus_tray_scroll_cb scroll_cb;
    void *scroll_userdata;
    uint32_t registered_id;
};

static osxie_dbus_tray_t **g_trays = NULL;
static int g_n_trays = 0;

static void add_tray(osxie_dbus_tray_t *tray) {
    g_trays = realloc(g_trays, sizeof(osxie_dbus_tray_t*) * (g_n_trays + 1));
    g_trays[g_n_trays++] = tray;
}

osxie_dbus_tray_t *osxie_dbus_tray_new(const char *id, const char *app_name) {
    DBusConnection *bus = get_bus();
    if (!bus) return NULL;

    osxie_dbus_tray_t *tray = calloc(1, sizeof(*tray));
    tray->id = strdup(id);
    tray->status = strdup("Active");
    tray->object_path = strdup("/StatusNotifierItem");

    DBusError err;
    dbus_error_init(&err);

    char bus_name[256];
    snprintf(bus_name, sizeof(bus_name), "org.kde.StatusNotifierItem-%d-1", getpid());

    int ret = dbus_bus_request_name(bus, bus_name, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "[DBUS] failed to request tray name: %s\n", err.message);
        dbus_error_free(&err);
        free(tray->id);
        free(tray->status);
        free(tray->object_path);
        free(tray);
        return NULL;
    }

    tray->bus_name = strdup(bus_name);
    add_tray(tray);

    fprintf(stderr, "[DBUS] StatusNotifierItem registered as %s\n", bus_name);

    DBusMessage *msg = dbus_message_new_method_call(
        "org.kde.StatusNotifierWatcher",
        "/StatusNotifierWatcher",
        "org.kde.StatusNotifierWatcher",
        "RegisterStatusNotifierItem");

    if (msg) {
        char service[512];
        snprintf(service, sizeof(service), "%s%s", bus_name, tray->object_path);
        const char *service_str = service;
        dbus_message_append_args(msg, DBUS_TYPE_STRING, &service_str, DBUS_TYPE_INVALID);
        DBusMessage *reply = dbus_connection_send_with_reply_and_block(bus, msg, 2000, &err);
        dbus_message_unref(msg);
        if (reply) dbus_message_unref(reply);
        if (dbus_error_is_set(&err)) {
            fprintf(stderr, "[DBUS] RegisterStatusNotifierItem failed: %s\n", err.message);
            dbus_error_free(&err);
        }
    }

    return tray;
}

void osxie_dbus_tray_free(osxie_dbus_tray_t *tray) {
    if (!tray) return;
    free(tray->id);
    free(tray->bus_name);
    free(tray->object_path);
    free(tray->icon_name);
    free(tray->tooltip);
    free(tray->title);
    free(tray->status);
    free(tray);
}

void osxie_dbus_tray_set_status(osxie_dbus_tray_t *tray, const char *status) {
    free(tray->status);
    tray->status = strdup(status);
}

void osxie_dbus_tray_set_icon(osxie_dbus_tray_t *tray, const char *icon_name) {
    free(tray->icon_name);
    tray->icon_name = icon_name ? strdup(icon_name) : NULL;
}

void osxie_dbus_tray_set_icon_pixbuf(osxie_dbus_tray_t *tray, const uint8_t *rgba, int width, int height) {
}

void osxie_dbus_tray_set_tooltip(osxie_dbus_tray_t *tray, const char *text) {
    free(tray->tooltip);
    tray->tooltip = text ? strdup(text) : NULL;
}

void osxie_dbus_tray_set_title(osxie_dbus_tray_t *tray, const char *title) {
    free(tray->title);
    tray->title = title ? strdup(title) : NULL;
}

void osxie_dbus_tray_activate(osxie_dbus_tray_t *tray, int x, int y) {
    if (tray->activate_cb)
        tray->activate_cb(x, y, tray->activate_userdata);
}

void osxie_dbus_tray_set_activate_callback(osxie_dbus_tray_t *tray, osxie_dbus_tray_activate_cb cb, void *userdata) {
    tray->activate_cb = cb;
    tray->activate_userdata = userdata;
}

void osxie_dbus_tray_set_scroll_callback(osxie_dbus_tray_t *tray, osxie_dbus_tray_scroll_cb cb, void *userdata) {
    tray->scroll_cb = cb;
    tray->scroll_userdata = userdata;
}

/* ==================== XDG Portals ==================== */

struct osxie_dbus_portal {
    void (*color_scheme_cb)(int scheme, void *userdata);
    void *color_scheme_userdata;
};

osxie_dbus_portal_t *osxie_dbus_portal_new(void) {
    return calloc(1, sizeof(osxie_dbus_portal_t));
}

void osxie_dbus_portal_free(osxie_dbus_portal_t *portal) {
    free(portal);
}

int osxie_dbus_portal_open_file(osxie_dbus_portal_t *portal, const char *title, int multiple,
                                const char *accept_label, osxie_dbus_portal_response_cb cb, void *userdata) {
    DBusConnection *bus = get_bus();
    if (!bus) return -1;

    const char *handle = "/org/osxie/portal/1";
    DBusMessage *msg = dbus_message_new_method_call(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.FileChooser",
        "OpenFile");

    if (!msg) return -1;

    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);

    dbus_message_iter_append_basic(&args, DBUS_TYPE_OBJECT_PATH, &handle);

    DBusMessageIter options;
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &options);

    const char *key;
    DBusMessageIter entry;

    key = "handle_token";
    dbus_message_iter_open_container(&options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    append_variant_string(&entry, "osxie1");
    dbus_message_iter_close_container(&options, &entry);

    if (title) {
        key = "title";
        dbus_message_iter_open_container(&options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
        append_variant_string(&entry, title);
        dbus_message_iter_close_container(&options, &entry);
    }

    key = "multiple";
    dbus_message_iter_open_container(&options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    append_variant_bool(&entry, multiple);
    dbus_message_iter_close_container(&options, &entry);

    dbus_message_iter_close_container(&args, &options);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(bus, msg, 5000, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "[DBUS] portal OpenFile failed: %s\n", err.message);
        dbus_error_free(&err);
        return -1;
    }
    if (reply) {
        const char *request_path = NULL;
        dbus_message_get_args(reply, NULL, DBUS_TYPE_OBJECT_PATH, &request_path, DBUS_TYPE_INVALID);
        fprintf(stderr, "[DBUS] portal OpenFile request: %s\n", request_path ? request_path : "(null)");
        dbus_message_unref(reply);
        if (cb) cb(0, NULL, 0, userdata);
    }
    return 0;
}

int osxie_dbus_portal_save_file(osxie_dbus_portal_t *portal, const char *title,
                                const char *suggested_name, osxie_dbus_portal_response_cb cb, void *userdata) {
    DBusConnection *bus = get_bus();
    if (!bus) return -1;

    const char *handle = "/org/osxie/portal/2";
    DBusMessage *msg = dbus_message_new_method_call(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.FileChooser",
        "SaveFile");

    if (!msg) return -1;

    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_OBJECT_PATH, &handle);

    DBusMessageIter options;
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &options);

    const char *key;
    DBusMessageIter entry;

    key = "handle_token";
    dbus_message_iter_open_container(&options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    append_variant_string(&entry, "osxie2");
    dbus_message_iter_close_container(&options, &entry);

    if (title) {
        key = "title";
        dbus_message_iter_open_container(&options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
        append_variant_string(&entry, title);
        dbus_message_iter_close_container(&options, &entry);
    }

    if (suggested_name) {
        key = "current_name";
        dbus_message_iter_open_container(&options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
        append_variant_string(&entry, suggested_name);
        dbus_message_iter_close_container(&options, &entry);
    }

    dbus_message_iter_close_container(&args, &options);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(bus, msg, 5000, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "[DBUS] portal SaveFile failed: %s\n", err.message);
        dbus_error_free(&err);
        return -1;
    }
    if (reply) dbus_message_unref(reply);
    if (cb) cb(0, NULL, 0, userdata);
    return 0;
}

int osxie_dbus_portal_notify(osxie_dbus_portal_t *portal, const char *app_name,
                             const char *summary, const char *body, const char *icon_name) {
    DBusConnection *bus = get_bus();
    if (!bus) return -1;

    DBusMessage *msg = dbus_message_new_method_call(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        "Notify");

    if (!msg) return -1;

    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);

    const char *app = app_name ? app_name : "osxie";
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &app);

    uint32_t replaces_id = 0;
    dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &replaces_id);

    const char *icon = icon_name ? icon_name : "";
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &icon);

    const char *sum = summary ? summary : "";
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &sum);

    const char *body_text = body ? body : "";
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &body_text);

    DBusMessageIter actions;
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "s", &actions);
    dbus_message_iter_close_container(&args, &actions);

    DBusMessageIter hints;
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &hints);
    dbus_message_iter_close_container(&args, &hints);

    int32_t expire = -1;
    dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &expire);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(bus, msg, 5000, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "[DBUS] Notify failed: %s\n", err.message);
        dbus_error_free(&err);
        return -1;
    }

    uint32_t notification_id = 0;
    if (reply) {
        dbus_message_get_args(reply, NULL, DBUS_TYPE_UINT32, &notification_id, DBUS_TYPE_INVALID);
        dbus_message_unref(reply);
    }

    return (int)notification_id;
}

int osxie_dbus_portal_get_color_scheme(osxie_dbus_portal_t *portal) {
    DBusConnection *bus = get_bus();
    if (!bus) return -1;

    DBusMessage *msg = dbus_message_new_method_call(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Settings",
        "Read");

    if (!msg) return -1;

    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);

    const char *group = "org.freedesktop.appearance";
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &group);
    const char *key = "color-scheme";
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &key);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(bus, msg, 2000, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err) || !reply) {
        if (dbus_error_is_set(&err)) dbus_error_free(&err);
        return -1;
    }

    DBusMessageIter reply_args;
    dbus_message_iter_init_append(reply, &reply_args);
    DBusMessageIter variant;
    dbus_message_iter_open_container(&reply_args, DBUS_TYPE_VARIANT, DBUS_TYPE_UINT32_AS_STRING, &variant);
    uint32_t scheme = 0;
    dbus_message_iter_get_basic(&variant, &scheme);
    dbus_message_iter_close_container(&reply_args, &variant);
    dbus_message_unref(reply);

    return (int)scheme;
}

void osxie_dbus_portal_set_color_scheme_callback(osxie_dbus_portal_t *portal,
                                                  void (*cb)(int scheme, void *userdata), void *userdata) {
    portal->color_scheme_cb = cb;
    portal->color_scheme_userdata = userdata;
}
