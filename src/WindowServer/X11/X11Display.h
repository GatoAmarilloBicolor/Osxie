#pragma once
#include <X11/Xlib.h>
#include <stdbool.h>

typedef struct {
    Display *dpy;
    int screen;
    Window root;
    int xi_opcode;
    bool has_xinput2;
    bool has_xfixes;
    bool has_xdamage;
    bool is_xwayland;
    double scale;
} X11Display;

extern X11Display g_x11;

bool X11Display_Init(void);
void X11Display_Shutdown(void);
void X11Display_ProcessEvents(void);
bool X11Display_IsXWayland(void);
