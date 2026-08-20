#pragma once
#include <CoreGraphics/CoreGraphics.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>

enum {
    kCGSEventLeftMouseDown     = 1,
    kCGSEventLeftMouseUp       = 2,
    kCGSEventRightMouseDown    = 3,
    kCGSEventRightMouseUp      = 4,
    kCGSEventMouseMoved        = 5,
    kCGSEventLeftMouseDragged  = 6,
    kCGSEventRightMouseDragged = 7,
    kCGSEventMouseEntered      = 8,
    kCGSEventMouseExited       = 9,
    kCGSEventKeyDown           = 10,
    kCGSEventKeyUp             = 11,
    kCGSEventFlagsChanged      = 12,
    kCGSEventSystemDefined     = 14,
    kCGSEventScrollWheel       = 22,
    kCGSEventOtherMouseDown    = 25,
    kCGSEventOtherMouseUp      = 26,
    kCGSEventOtherMouseDragged = 27,
};

enum {
    kCGSEventFlagMaskShift      = 1ULL << 17,
    kCGSEventFlagMaskControl    = 1ULL << 18,
    kCGSEventFlagMaskAlternate  = 1ULL << 19,
    kCGSEventFlagMaskCommand    = 1ULL << 20,
    kCGSEventFlagMaskSecondaryFn = 1ULL << 23,
    kCGSEventFlagMaskAlphaShift = 1ULL << 16,
};

void CGSInitEventSystem(void);
CGError CGSPostEventRecord2(const CGSEventRecord *record);
int     CGSGetNextEventRecord(CGSEventRecord *out);
int     CGSPeekEventRecord(CGSEventRecord *out);
