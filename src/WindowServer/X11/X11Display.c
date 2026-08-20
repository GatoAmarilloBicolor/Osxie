#include "X11Display.h"
#include "X11Keymap.h"
#include "../CGSEvent.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xfixes.h>
#include <X11/XKBlib.h>

X11Display g_x11 = {0};

bool X11Display_Init(void) {
    g_x11.dpy = XOpenDisplay(NULL);
    if (!g_x11.dpy) {
        fprintf(stderr, "Osxie: Cannot open X display\n");
        return false;
    }

    g_x11.screen = DefaultScreen(g_x11.dpy);
    g_x11.root   = DefaultRootWindow(g_x11.dpy);

    const char *vendor = ServerVendor(g_x11.dpy);
    g_x11.is_xwayland = vendor && strstr(vendor, "XWayland");

    int event, error;
    if (XQueryExtension(g_x11.dpy, "XInputExtension", &g_x11.xi_opcode, &event, &error)) {
        int major = 2, minor = 0;
        if (XIQueryVersion(g_x11.dpy, &major, &minor) == Success)
            g_x11.has_xinput2 = true;
    }

    g_x11.has_xfixes  = XQueryExtension(g_x11.dpy, "XFIXES", &event, &error, &error);
    g_x11.has_xdamage = XQueryExtension(g_x11.dpy, "DAMAGE", &event, &error, &error);
    g_x11.scale = 1.0;

    XkbSetDetectableAutoRepeat(g_x11.dpy, True, NULL);

    printf("Osxie WindowServer: X11 initialized%s\n",
           g_x11.is_xwayland ? " (XWayland)" : "");
    return true;
}

void X11Display_Shutdown(void) {
    if (g_x11.dpy) {
        XCloseDisplay(g_x11.dpy);
        g_x11.dpy = NULL;
    }
}

bool X11Display_IsXWayland(void) {
    return g_x11.is_xwayland;
}

static uint64_t get_monotonic_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void X11Display_ProcessEvents(void) {
    if (!g_x11.dpy) return;

    while (XPending(g_x11.dpy)) {
        XEvent xev;
        XNextEvent(g_x11.dpy, &xev);

        CGSEventRecord rec;
        memset(&rec, 0, sizeof(rec));
        rec.major = 1;
        rec.length = sizeof(rec);
        rec.connection = CGSMainConnectionID();
        rec.time = get_monotonic_ns();

        switch (xev.type) {
        case ButtonPress:
        case ButtonRelease: {
            bool down = (xev.type == ButtonPress);
            if (xev.xbutton.button == Button1)
                rec.type = down ? kCGSEventLeftMouseDown : kCGSEventLeftMouseUp;
            else if (xev.xbutton.button == Button3)
                rec.type = down ? kCGSEventRightMouseDown : kCGSEventRightMouseUp;
            else if (xev.xbutton.button == Button2)
                rec.type = down ? kCGSEventOtherMouseDown : kCGSEventOtherMouseUp;
            else continue;

            rec.location.x = xev.xbutton.x_root;
            rec.location.y = xev.xbutton.y_root;
            rec.flags = X11StateToCGSFlags(xev.xbutton.state);
            rec.data.mouse.buttonNumber = xev.xbutton.button;
            break;
        }
        case MotionNotify: {
            rec.type = kCGSEventMouseMoved;
            rec.location.x = xev.xmotion.x_root;
            rec.location.y = xev.xmotion.y_root;
            rec.flags = X11StateToCGSFlags(xev.xmotion.state);
            break;
        }
        case KeyPress:
        case KeyRelease: {
            KeySym ks = XLookupKeysym(&xev.xkey, 0);
            rec.type = (xev.type == KeyPress) ? kCGSEventKeyDown : kCGSEventKeyUp;
            rec.data.key.keyCode = X11KeySymToMacKeyCode(ks);
            rec.flags = X11StateToCGSFlags(xev.xkey.state);
            break;
        }
        case ConfigureNotify:
            continue;
        default:
            continue;
        }

        CGSPostEventRecord2(&rec);
    }
}
