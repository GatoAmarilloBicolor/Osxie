#pragma once
#include <CoreGraphics/CoreGraphics.h>
#include <stdbool.h>
#include <X11/Xlib.h>

void CGSWindowServerInit(void);
void CGSWindowTrackX11(CGWindowID wid, Window xwin);
void CGSWindowUpdateBounds(CGWindowID wid, CGRect bounds);
void CGSWindowUntrack(CGWindowID wid);
void CGSWindowMarkPopup(CGWindowID wid);

CGPoint CGSConvertPointToScreen(CGWindowID wid, CGPoint localPoint);
CGPoint CGSConvertPointFromScreen(CGWindowID wid, CGPoint screenPoint);
bool    CGSWindowIsPopup(CGWindowID wid);
void*   CGSWindowGetX11Window(CGWindowID wid);
