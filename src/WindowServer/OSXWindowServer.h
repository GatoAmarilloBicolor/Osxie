/**
 * Osxie WindowServer Emulation
 * Provides window management and event handling for GUI applications
 */

#ifndef OSXIE_WINDOWSERVER_H
#define OSXIE_WINDOWSERVER_H

#include <stdint.h>
#include <stdbool.h>
#include <CoreGraphics/CoreGraphics.h>

#ifdef __cplusplus
extern "C" {
#endif

// Window Server Connection
typedef struct __OSXWindowServerConnection *OSXWindowServerConnectionRef;

// Window types
typedef enum {
    kOSXWindowTypeNormal = 0,
    kOSXWindowTypeMenu = 1,
    kOSXWindowTypeToolbar = 2,
    kOSXWindowTypePopup = 3,
    kOSXWindowTypeDock = 4,
    kOSXWindowTypeDesktop = 5,
    kOSXWindowTypeSplash = 6,
    kOSXWindowTypeDialog = 7,
    kOSXWindowTypeUtility = 8,
    kOSXWindowTypeSheet = 9,
    kOSXWindowTypeDrawer = 10,
    kOSXWindowTypeOverlay = 11
} OSXWindowType;

// Window levels
typedef enum {
    kOSXWindowLevelNormal = 0,
    kOSXWindowLevelFloating = 3,
    kOSXWindowLevelTornOffMenu = 5,
    kOSXWindowLevelDock = 6,
    kOSXWindowLevelMainMenu = 8,
    kOSXWindowLevelStatus = 9,
    kOSXWindowLevelModalPanel = 10,
    kOSXWindowLevelPopUpMenu = 11,
    kOSXWindowLevelDragging = 12,
    kOSXWindowLevelScreenSaver = 13,
    kOSXWindowLevelOverlay = 14
} OSXWindowLevel;

// Window attributes
typedef enum {
    kOSXWindowAttributeClosable = 1 << 0,
    kOSXWindowAttributeMinimizable = 1 << 1,
    kOSXWindowAttributeResizable = 1 << 2,
    kOSXWindowAttributeFullScreen = 1 << 3,
    kOSXWindowAttributeTransparent = 1 << 4,
    kOSXWindowAttributeShadow = 1 << 5,
    kOSXWindowAttributeBuffered = 1 << 6,
    kOSXWindowAttributeAccelerated = 1 << 7,
    kOSXWindowAttributeOpaque = 1 << 8,
    kOSXWindowAttributeVisible = 1 << 9
} OSXWindowAttribute;

// Event types
typedef enum {
    kOSXEventTypeNull = 0,
    kOSXEventTypeLeftMouseDown = 1,
    kOSXEventTypeLeftMouseUp = 2,
    kOSXEventTypeRightMouseDown = 3,
    kOSXEventTypeRightMouseUp = 4,
    kOSXEventTypeMouseMoved = 5,
    kOSXEventTypeLeftMouseDragged = 6,
    kOSXEventTypeRightMouseDragged = 7,
    kOSXEventTypeMouseEntered = 8,
    kOSXEventTypeMouseExited = 9,
    kOSXEventTypeKeyDown = 10,
    kOSXEventTypeKeyUp = 11,
    kOSXEventTypeFlagsChanged = 12,
    kOSXEventTypeScrollWheel = 22,
    kOSXEventTypeTabletPoint = 23,
    kOSXEventTypeTabletProximity = 24,
    kOSXEventTypeOtherMouseDown = 25,
    kOSXEventTypeOtherMouseUp = 26,
    kOSXEventTypeOtherMouseDragged = 27,
    kOSXEventTypeGesture = 29,
    kOSXEventTypeMagnify = 30,
    kOSXEventTypeSwipe = 31,
    kOSXEventTypeRotate = 32,
    kOSXEventTypeBeginGesture = 33,
    kOSXEventTypeEndGesture = 34,
    kOSXEventTypeSmartMagnify = 35,
    kOSXEventTypeQuickLook = 36,
    kOSXEventTypePressure = 37,
    kOSXEventTypeDirectTouch = 38,
    kOSXEventTypeSystemDefined = 14,
    kOSXEventTypeApplicationDefined = 15
} OSXEventType;

// Window Server functions

// Connection management
OSXWindowServerConnectionRef OSXWindowServerConnect(void);
void OSXWindowServerDisconnect(OSXWindowServerConnectionRef connection);
bool OSXWindowServerIsConnected(OSXWindowServerConnectionRef connection);

// Window creation and management
uint32_t OSXWindowServerCreateWindow(OSXWindowServerConnectionRef connection, 
                                      CGRect frame, 
                                      OSXWindowType type, 
                                      uint32_t attributes);
void OSXWindowServerDestroyWindow(OSXWindowServerConnectionRef connection, uint32_t windowID);
void OSXWindowServerShowWindow(OSXWindowServerConnectionRef connection, uint32_t windowID);
void OSXWindowServerHideWindow(OSXWindowServerConnectionRef connection, uint32_t windowID);
void OSXWindowServerMoveWindow(OSXWindowServerConnectionRef connection, uint32_t windowID, CGPoint location);
void OSXWindowServerResizeWindow(OSXWindowServerConnectionRef connection, uint32_t windowID, CGSize size);
void OSXWindowServerSetWindowLevel(OSXWindowServerConnectionRef connection, uint32_t windowID, OSXWindowLevel level);
void OSXWindowServerSetWindowTitle(OSXWindowServerConnectionRef connection, uint32_t windowID, const char *title);

// Window properties
CGRect OSXWindowServerGetWindowBounds(OSXWindowServerConnectionRef connection, uint32_t windowID);
bool OSXWindowServerIsWindowVisible(OSXWindowServerConnectionRef connection, uint32_t windowID);
OSXWindowLevel OSXWindowServerGetWindowLevel(OSXWindowServerConnectionRef connection, uint32_t windowID);

// Event handling
void OSXWindowServerPostEvent(OSXWindowServerConnectionRef connection, OSXEventType type, void *eventData);
bool OSXWindowServerGetNextEvent(OSXWindowServerConnectionRef connection, OSXEventType *type, void **eventData);
void OSXWindowServerFlushEvents(OSXWindowServerConnectionRef connection);

// Surface management (for rendering)
CGContextRef OSXWindowServerGetWindowContext(OSXWindowServerConnectionRef connection, uint32_t windowID);
void OSXWindowServerFlushWindow(OSXWindowServerConnectionRef connection, uint32_t windowID);
void OSXWindowServerFlushWindowRect(OSXWindowServerConnectionRef connection, uint32_t windowID, CGRect rect);

// Display management
CGDirectDisplayID OSXWindowServerGetMainDisplay(OSXWindowServerConnectionRef connection);
CGRect OSXWindowServerGetDisplayBounds(OSXWindowServerConnectionRef connection, CGDirectDisplayID display);
uint32_t OSXWindowServerGetDisplayCount(OSXWindowServerConnectionRef connection);

// Cursor management
void OSXWindowServerSetCursor(OSXWindowServerConnectionRef connection, CGImageRef cursorImage, CGPoint hotspot);
void OSXWindowServerShowCursor(OSXWindowServerConnectionRef connection);
void OSXWindowServerHideCursor(OSXWindowServerConnectionRef connection);
CGPoint OSXWindowServerGetCursorPosition(OSXWindowServerConnectionRef connection);
void OSXWindowServerSetCursorPosition(OSXWindowServerConnectionRef connection, CGPoint position);

// Menu bar
void OSXWindowServerSetMenuBarVisible(OSXWindowServerConnectionRef connection, bool visible);
bool OSXWindowServerIsMenuBarVisible(OSXWindowServerConnectionRef connection);
float OSXWindowServerGetMenuBarHeight(OSXWindowServerConnectionRef connection);

// Dock
void OSXWindowServerSetDockVisible(OSXWindowServerConnectionRef connection, bool visible);
bool OSXWindowServerIsDockVisible(OSXWindowServerConnectionRef connection);
CGRect OSXWindowServerGetDockBounds(OSXWindowServerConnectionRef connection);

// Session management
void OSXWindowServerLockScreen(OSXWindowServerConnectionRef connection);
void OSXWindowServerUnlockScreen(OSXWindowServerConnectionRef connection);
bool OSXWindowServerIsScreenLocked(OSXWindowServerConnectionRef connection);

#ifdef __cplusplus
}
#endif

#endif // OSXIE_WINDOWSERVER_H