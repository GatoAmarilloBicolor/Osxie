/**
 * Osxie WindowServer Implementation
 * X11 backend for window management
 */

#include "OSXWindowServer.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XTest.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>

typedef struct __OSXWindow {
    uint32_t id;
    Window x11_window;
    CGRect frame;
    OSXWindowType type;
    OSXWindowLevel level;
    uint32_t attributes;
    char *title;
    bool visible;
    CGContextRef context;
    struct __OSXWindow *next;
} OSXWindow;

typedef struct __OSXWindowServerConnection {
    Display *display;
    int screen;
    Window root_window;
    pthread_mutex_t lock;
    OSXWindow *windows;
    uint32_t next_window_id;
    Atom wm_delete_window;
    Atom wm_protocols;
    bool connected;
} OSXWindowServerConnection;

static OSXWindowServerConnection *g_connection = NULL;

// Initialize X11 connection
OSXWindowServerConnectionRef OSXWindowServerConnect(void) {
    if (g_connection && g_connection->connected) {
        return g_connection;
    }
    
    OSXWindowServerConnection *conn = calloc(1, sizeof(OSXWindowServerConnection));
    if (!conn) return NULL;
    
    // Try to connect to X11
    const char *display_name = getenv("DISPLAY");
    if (!display_name) {
        display_name = ":0";
    }
    
    conn->display = XOpenDisplay(display_name);
    if (!conn->display) {
        fprintf(stderr, "[OSXIE] Failed to connect to X11 display %s\n", display_name);
        free(conn);
        return NULL;
    }
    
    conn->screen = DefaultScreen(conn->display);
    conn->root_window = RootWindow(conn->display, conn->screen);
    conn->next_window_id = 1000;
    conn->connected = true;
    
    // Setup atoms
    conn->wm_protocols = XInternAtom(conn->display, "WM_PROTOCOLS", False);
    conn->wm_delete_window = XInternAtom(conn->display, "WM_DELETE_WINDOW", False);
    
    pthread_mutex_init(&conn->lock, NULL);
    
    g_connection = conn;
    
    printf("[OSXIE] WindowServer connected to X11 display %s\n", display_name);
    
    return conn;
}

void OSXWindowServerDisconnect(OSXWindowServerConnectionRef connection) {
    if (!connection) return;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    pthread_mutex_lock(&conn->lock);
    
    // Close all windows
    OSXWindow *window = conn->windows;
    while (window) {
        OSXWindow *next = window->next;
        if (window->x11_window) {
            XDestroyWindow(conn->display, window->x11_window);
        }
        free(window->title);
        free(window);
        window = next;
    }
    
    conn->connected = false;
    
    if (conn->display) {
        XCloseDisplay(conn->display);
        conn->display = NULL;
    }
    
    pthread_mutex_unlock(&conn->lock);
    pthread_mutex_destroy(&conn->lock);
    
    if (g_connection == conn) {
        g_connection = NULL;
    }
    
    free(conn);
}

bool OSXWindowServerIsConnected(OSXWindowServerConnectionRef connection) {
    if (!connection) return false;
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    return conn->connected;
}

// Create a new window
uint32_t OSXWindowServerCreateWindow(OSXWindowServerConnectionRef connection, 
                                      CGRect frame, 
                                      OSXWindowType type, 
                                      uint32_t attributes) {
    if (!connection) return 0;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    pthread_mutex_lock(&conn->lock);
    
    // Create X11 window
    XSetWindowAttributes attrs;
    attrs.background_pixel = WhitePixel(conn->display, conn->screen);
    attrs.border_pixel = BlackPixel(conn->display, conn->screen);
    attrs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | 
                       ButtonPressMask | ButtonReleaseMask | 
                       PointerMotionMask | StructureNotifyMask |
                       EnterWindowMask | LeaveWindowMask;
    
    Window x11_window = XCreateWindow(
        conn->display,
        conn->root_window,
        frame.origin.x, frame.origin.y,
        frame.size.width, frame.size.height,
        0, // border width
        CopyFromParent, // depth
        InputOutput, // class
        CopyFromParent, // visual
        CWBackPixel | CWBorderPixel | CWEventMask,
        &attrs
    );
    
    // Set window manager protocols
    XSetWMProtocols(conn->display, x11_window, &conn->wm_delete_window, 1);
    
    // Create OSXWindow structure
    OSXWindow *window = calloc(1, sizeof(OSXWindow));
    window->id = conn->next_window_id++;
    window->x11_window = x11_window;
    window->frame = frame;
    window->type = type;
    window->level = kOSXWindowLevelNormal;
    window->attributes = attributes;
    window->visible = false;
    
    // Add to window list
    window->next = conn->windows;
    conn->windows = window;
    
    pthread_mutex_unlock(&conn->lock);
    
    printf("[OSXIE] Created window %u (X11: %lu) at (%.0f,%.0f) size %.0fx%.0f\n",
           window->id, x11_window, frame.origin.x, frame.origin.y,
           frame.size.width, frame.size.height);
    
    return window->id;
}

void OSXWindowServerDestroyWindow(OSXWindowServerConnectionRef connection, uint32_t windowID) {
    if (!connection) return;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    pthread_mutex_lock(&conn->lock);
    
    OSXWindow **prev = &conn->windows;
    OSXWindow *window = conn->windows;
    
    while (window) {
        if (window->id == windowID) {
            *prev = window->next;
            
            if (window->x11_window) {
                XDestroyWindow(conn->display, window->x11_window);
            }
            
            free(window->title);
            free(window);
            
            XFlush(conn->display);
            break;
        }
        prev = &window->next;
        window = window->next;
    }
    
    pthread_mutex_unlock(&conn->lock);
}

void OSXWindowServerShowWindow(OSXWindowServerConnectionRef connection, uint32_t windowID) {
    if (!connection) return;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    pthread_mutex_lock(&conn->lock);
    
    OSXWindow *window = conn->windows;
    while (window) {
        if (window->id == windowID) {
            XMapWindow(conn->display, window->x11_window);
            XFlush(conn->display);
            window->visible = true;
            break;
        }
        window = window->next;
    }
    
    pthread_mutex_unlock(&conn->lock);
}

void OSXWindowServerHideWindow(OSXWindowServerConnectionRef connection, uint32_t windowID) {
    if (!connection) return;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    pthread_mutex_lock(&conn->lock);
    
    OSXWindow *window = conn->windows;
    while (window) {
        if (window->id == windowID) {
            XUnmapWindow(conn->display, window->x11_window);
            XFlush(conn->display);
            window->visible = false;
            break;
        }
        window = window->next;
    }
    
    pthread_mutex_unlock(&conn->lock);
}

void OSXWindowServerMoveWindow(OSXWindowServerConnectionRef connection, uint32_t windowID, CGPoint location) {
    if (!connection) return;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    pthread_mutex_lock(&conn->lock);
    
    OSXWindow *window = conn->windows;
    while (window) {
        if (window->id == windowID) {
            XMoveWindow(conn->display, window->x11_window, location.x, location.y);
            XFlush(conn->display);
            window->frame.origin = location;
            break;
        }
        window = window->next;
    }
    
    pthread_mutex_unlock(&conn->lock);
}

void OSXWindowServerResizeWindow(OSXWindowServerConnectionRef connection, uint32_t windowID, CGSize size) {
    if (!connection) return;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    pthread_mutex_lock(&conn->lock);
    
    OSXWindow *window = conn->windows;
    while (window) {
        if (window->id == windowID) {
            XResizeWindow(conn->display, window->x11_window, size.width, size.height);
            XFlush(conn->display);
            window->frame.size = size;
            break;
        }
        window = window->next;
    }
    
    pthread_mutex_unlock(&conn->lock);
}

void OSXWindowServerSetWindowTitle(OSXWindowServerConnectionRef connection, uint32_t windowID, const char *title) {
    if (!connection || !title) return;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    pthread_mutex_lock(&conn->lock);
    
    OSXWindow *window = conn->windows;
    while (window) {
        if (window->id == windowID) {
            XStoreName(conn->display, window->x11_window, title);
            XFlush(conn->display);
            
            free(window->title);
            window->title = strdup(title);
            break;
        }
        window = window->next;
    }
    
    pthread_mutex_unlock(&conn->lock);
}

CGRect OSXWindowServerGetWindowBounds(OSXWindowServerConnectionRef connection, uint32_t windowID) {
    CGRect bounds = {{0, 0}, {0, 0}};
    
    if (!connection) return bounds;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    pthread_mutex_lock(&conn->lock);
    
    OSXWindow *window = conn->windows;
    while (window) {
        if (window->id == windowID) {
            bounds = window->frame;
            break;
        }
        window = window->next;
    }
    
    pthread_mutex_unlock(&conn->lock);
    
    return bounds;
}

bool OSXWindowServerIsWindowVisible(OSXWindowServerConnectionRef connection, uint32_t windowID) {
    if (!connection) return false;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    bool visible = false;
    
    pthread_mutex_lock(&conn->lock);
    
    OSXWindow *window = conn->windows;
    while (window) {
        if (window->id == windowID) {
            visible = window->visible;
            break;
        }
        window = window->next;
    }
    
    pthread_mutex_unlock(&conn->lock);
    
    return visible;
}

// Event handling
void OSXWindowServerPostEvent(OSXWindowServerConnectionRef connection, OSXEventType type, void *eventData) {
    if (!connection) return;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    // Convert to X11 event and send
    switch (type) {
        case kOSXEventTypeLeftMouseDown:
            XTestFakeButtonEvent(conn->display, 1, True, CurrentTime);
            break;
        case kOSXEventTypeLeftMouseUp:
            XTestFakeButtonEvent(conn->display, 1, False, CurrentTime);
            break;
        case kOSXEventTypeRightMouseDown:
            XTestFakeButtonEvent(conn->display, 3, True, CurrentTime);
            break;
        case kOSXEventTypeRightMouseUp:
            XTestFakeButtonEvent(conn->display, 3, False, CurrentTime);
            break;
        // Add more event types as needed
    }
    
    XFlush(conn->display);
}

// Display management
CGDirectDisplayID OSXWindowServerGetMainDisplay(OSXWindowServerConnectionRef connection) {
    if (!connection) return 0;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    return (CGDirectDisplayID)conn->screen;
}

CGRect OSXWindowServerGetDisplayBounds(OSXWindowServerConnectionRef connection, CGDirectDisplayID display) {
    CGRect bounds = {{0, 0}, {0, 0}};
    
    if (!connection) return bounds;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    bounds.size.width = DisplayWidth(conn->display, conn->screen);
    bounds.size.height = DisplayHeight(conn->display, conn->screen);
    
    return bounds;
}

uint32_t OSXWindowServerGetDisplayCount(OSXWindowServerConnectionRef connection) {
    if (!connection) return 0;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    return ScreenCount(conn->display);
}

// Cursor management
void OSXWindowServerShowCursor(OSXWindowServerConnectionRef connection) {
    // X11 cursor is always visible by default
}

void OSXWindowServerHideCursor(OSXWindowServerConnectionRef connection) {
    if (!connection) return;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    // Hide cursor by creating an invisible cursor
    Pixmap blank;
    XColor dummy;
    char data[1] = {0};
    
    blank = XCreateBitmapFromData(conn->display, conn->root_window, data, 1, 1);
    Cursor invisible = XCreatePixmapCursor(conn->display, blank, blank, &dummy, &dummy, 0, 0);
    
    XDefineCursor(conn->display, conn->root_window, invisible);
    XFreeCursor(conn->display, invisible);
    XFreePixmap(conn->display, blank);
    XFlush(conn->display);
}

CGPoint OSXWindowServerGetCursorPosition(OSXWindowServerConnectionRef connection) {
    CGPoint pos = {0, 0};
    
    if (!connection) return pos;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    
    XQueryPointer(conn->display, conn->root_window, &root, &child,
                  &root_x, &root_y, &win_x, &win_y, &mask);
    
    pos.x = root_x;
    pos.y = root_y;
    
    return pos;
}

void OSXWindowServerSetCursorPosition(OSXWindowServerConnectionRef connection, CGPoint position) {
    if (!connection) return;
    
    OSXWindowServerConnection *conn = (OSXWindowServerConnection *)connection;
    
    XWarpPointer(conn->display, None, conn->root_window, 0, 0, 0, 0,
                 position.x, position.y);
    XFlush(conn->display);
}

// Menu bar
float OSXWindowServerGetMenuBarHeight(OSXWindowServerConnectionRef connection) {
    return 22.0; // Standard macOS menu bar height
}