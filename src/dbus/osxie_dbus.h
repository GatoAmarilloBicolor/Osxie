#ifndef OSXIE_DBUS_H
#define OSXIE_DBUS_H

#include <stdint.h>

typedef struct osxie_dbus_menu osxie_dbus_menu_t;
typedef struct osxie_dbus_tray osxie_dbus_tray_t;
typedef struct osxie_dbus_portal osxie_dbus_portal_t;

typedef void (*osxie_dbus_menu_action_cb)(int item_id, void *userdata);
typedef void (*osxie_dbus_tray_activate_cb)(int x, int y, void *userdata);
typedef void (*osxie_dbus_tray_scroll_cb)(int delta, void *userdata);
typedef void (*osxie_dbus_portal_response_cb)(int response, const char **uris, int count, void *userdata);

int osxie_dbus_init(void);
void osxie_dbus_dispatch(void);
int osxie_dbus_is_available(void);

osxie_dbus_menu_t *osxie_dbus_menu_new(const char *app_name, const char *icon_name);
void osxie_dbus_menu_free(osxie_dbus_menu_t *menu);
int osxie_dbus_menu_register_window(osxie_dbus_menu_t *menu, uint32_t x11_window_id);
void osxie_dbus_menu_unregister_window(osxie_dbus_menu_t *menu, uint32_t x11_window_id);
void osxie_dbus_menu_update(osxie_dbus_menu_t *menu);
void osxie_dbus_menu_set_action_callback(osxie_dbus_menu_t *menu, osxie_dbus_menu_action_cb cb, void *userdata);

osxie_dbus_tray_t *osxie_dbus_tray_new(const char *id, const char *app_name);
void osxie_dbus_tray_free(osxie_dbus_tray_t *tray);
void osxie_dbus_tray_set_status(osxie_dbus_tray_t *tray, const char *status);
void osxie_dbus_tray_set_icon(osxie_dbus_tray_t *tray, const char *icon_name);
void osxie_dbus_tray_set_icon_pixbuf(osxie_dbus_tray_t *tray, const uint8_t *rgba, int width, int height);
void osxie_dbus_tray_set_tooltip(osxie_dbus_tray_t *tray, const char *text);
void osxie_dbus_tray_set_title(osxie_dbus_tray_t *tray, const char *title);
void osxie_dbus_tray_activate(osxie_dbus_tray_t *tray, int x, int y);
void osxie_dbus_tray_set_activate_callback(osxie_dbus_tray_t *tray, osxie_dbus_tray_activate_cb cb, void *userdata);
void osxie_dbus_tray_set_scroll_callback(osxie_dbus_tray_t *tray, osxie_dbus_tray_scroll_cb cb, void *userdata);

osxie_dbus_portal_t *osxie_dbus_portal_new(void);
void osxie_dbus_portal_free(osxie_dbus_portal_t *portal);
int osxie_dbus_portal_open_file(osxie_dbus_portal_t *portal, const char *title, int multiple,
                                const char *accept_label, osxie_dbus_portal_response_cb cb, void *userdata);
int osxie_dbus_portal_save_file(osxie_dbus_portal_t *portal, const char *title,
                                const char *suggested_name, osxie_dbus_portal_response_cb cb, void *userdata);
int osxie_dbus_portal_notify(osxie_dbus_portal_t *portal, const char *app_name,
                             const char *summary, const char *body, const char *icon_name);
int osxie_dbus_portal_get_color_scheme(osxie_dbus_portal_t *portal);
void osxie_dbus_portal_set_color_scheme_callback(osxie_dbus_portal_t *portal,
                                                  void (*cb)(int scheme, void *userdata), void *userdata);

#endif
