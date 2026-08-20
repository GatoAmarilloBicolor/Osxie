#include "CGSWindow.h"
#include "X11/X11Display.h"
#include <CoreGraphics/CoreGraphics.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#define MAX_WINDOWS 1024

typedef struct {
    CGWindowID wid;
    Window     xwin;
    CGRect     bounds;
    bool       is_popup;
} WindowEntry;

static WindowEntry g_entries[MAX_WINDOWS] = {0};
static int g_entry_count = 0;

static WindowEntry *find_entry(CGWindowID wid) {
    for (int i = 0; i < g_entry_count; i++)
        if (g_entries[i].wid == wid)
            return &g_entries[i];
    return NULL;
}

void CGSWindowServerInit(void) {
    memset(g_entries, 0, sizeof(g_entries));
    g_entry_count = 0;
}

void CGSWindowTrackX11(CGWindowID wid, Window xwin) {
    if (g_entry_count >= MAX_WINDOWS) return;
    g_entries[g_entry_count].wid = wid;
    g_entries[g_entry_count].xwin = xwin;
    g_entries[g_entry_count].bounds = CGRectZero;
    g_entries[g_entry_count].is_popup = false;
    if (g_x11.dpy) {
        XWindowAttributes attr;
        if (XGetWindowAttributes(g_x11.dpy, xwin, &attr)) {
            g_entries[g_entry_count].bounds.origin.x = attr.x;
            g_entries[g_entry_count].bounds.origin.y = attr.y;
            g_entries[g_entry_count].bounds.size.width = attr.width;
            g_entries[g_entry_count].bounds.size.height = attr.height;
        }
    }
    g_entry_count++;
}

void CGSWindowUpdateBounds(CGWindowID wid, CGRect bounds) {
    WindowEntry *e = find_entry(wid);
    if (e) e->bounds = bounds;
}

void CGSWindowUntrack(CGWindowID wid) {
    for (int i = 0; i < g_entry_count; i++) {
        if (g_entries[i].wid == wid) {
            g_entries[i] = g_entries[g_entry_count - 1];
            g_entry_count--;
            return;
        }
    }
}

void CGSWindowMarkPopup(CGWindowID wid) {
    WindowEntry *e = find_entry(wid);
    if (e) e->is_popup = true;
}

bool CGSWindowIsPopup(CGWindowID wid) {
    WindowEntry *e = find_entry(wid);
    return e ? e->is_popup : false;
}

void* CGSWindowGetX11Window(CGWindowID wid) {
    WindowEntry *e = find_entry(wid);
    return e ? (void*)(uintptr_t)e->xwin : NULL;
}

CGPoint CGSConvertPointToScreen(CGWindowID wid, CGPoint local) {
    WindowEntry *e = find_entry(wid);
    if (!e) return local;
    CGPoint screen;
    screen.x = local.x + e->bounds.origin.x;
    screen.y = local.y + e->bounds.origin.y;
    return screen;
}

CGPoint CGSConvertPointFromScreen(CGWindowID wid, CGPoint screen) {
    WindowEntry *e = find_entry(wid);
    if (!e) return screen;
    CGPoint local;
    local.x = screen.x - e->bounds.origin.x;
    local.y = screen.y - e->bounds.origin.y;
    return local;
}
