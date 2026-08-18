/*
 This file is part of Osxie.

 Copyright (C) 2021 Lubos Dolezel

 Osxie is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Osxie is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Osxie.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CopyCFStringToFilesystemRepresentation(void)
{
    if (verbose) puts("STUB: CopyCFStringToFilesystemRepresentation called");
    return NULL;
}

void* CopyCFStringToUTF8(void)
{
    if (verbose) puts("STUB: CopyCFStringToUTF8 called");
    return NULL;
}

void* GetHitPart(void)
{
    if (verbose) puts("STUB: GetHitPart called");
    return NULL;
}

void* GetScrollBarHitPart(void)
{
    if (verbose) puts("STUB: GetScrollBarHitPart called");
    return NULL;
}

void* JRSAccessibilityUnregisterUniqueIdForUIElement(void)
{
    if (verbose) puts("STUB: JRSAccessibilityUnregisterUniqueIdForUIElement called");
    return NULL;
}

void* JRSCopyCanonicalLanguageForPrimaryLanguage(void)
{
    if (verbose) puts("STUB: JRSCopyCanonicalLanguageForPrimaryLanguage called");
    return NULL;
}

void* JRSCopyOSJavaSupportVersion(void)
{
    if (verbose) puts("STUB: JRSCopyOSJavaSupportVersion called");
    return NULL;
}

void* JRSCopyOSName(void)
{
    if (verbose) puts("STUB: JRSCopyOSName called");
    return NULL;
}

void* JRSCopyOSVersion(void)
{
    if (verbose) puts("STUB: JRSCopyOSVersion called");
    return NULL;
}

void* JRSCopyPrimaryLanguage(void)
{
    if (verbose) puts("STUB: JRSCopyPrimaryLanguage called");
    return NULL;
}

void* JRSFontAlignStyleForFractionalMeasurement(void)
{
    if (verbose) puts("STUB: JRSFontAlignStyleForFractionalMeasurement called");
    return NULL;
}

void* JRSFontAlignStyleForIntegerMeasurement(void)
{
    if (verbose) puts("STUB: JRSFontAlignStyleForIntegerMeasurement called");
    return NULL;
}

void* JRSFontCreateFallbackFontForCharacters(void)
{
    if (verbose) puts("STUB: JRSFontCreateFallbackFontForCharacters called");
    return NULL;
}

void* JRSFontGetAdvancesForGlyphsAndStyle(void)
{
    if (verbose) puts("STUB: JRSFontGetAdvancesForGlyphsAndStyle called");
    return NULL;
}

void* JRSFontGetBoundingBoxesForGlyphsAndStyle(void)
{
    if (verbose) puts("STUB: JRSFontGetBoundingBoxesForGlyphsAndStyle called");
    return NULL;
}

void* JRSFontGetRenderingStyleForContext(void)
{
    if (verbose) puts("STUB: JRSFontGetRenderingStyleForContext called");
    return NULL;
}

void* JRSFontGetRenderingStyleForHints(void)
{
    if (verbose) puts("STUB: JRSFontGetRenderingStyleForHints called");
    return NULL;
}

void* JRSFontSetRenderingStyleOnContext(void)
{
    if (verbose) puts("STUB: JRSFontSetRenderingStyleOnContext called");
    return NULL;
}

void* JRSFontStyleIsAntialiased(void)
{
    if (verbose) puts("STUB: JRSFontStyleIsAntialiased called");
    return NULL;
}

void* JRSFontStyleUsesFractionalMetrics(void)
{
    if (verbose) puts("STUB: JRSFontStyleUsesFractionalMetrics called");
    return NULL;
}

void* JRSSetDefaultLocalization(void)
{
    if (verbose) puts("STUB: JRSSetDefaultLocalization called");
    return NULL;
}

typedef CFMutableDictionaryRef JRSUIControlRef;

enum
{
    JRSUIPropWidget                    = 0x5100,
    JRSUIPropState                     = 0x5101,
    JRSUIPropSize                      = 0x5102,
    JRSUIPropDirection                 = 0x5103,
    JRSUIPropOrientation               = 0x5104,
    JRSUIPropAlignmentVertical         = 0x5105,
    JRSUIPropAlignmentHorizontal       = 0x5106,
    JRSUIPropSegmentPosition           = 0x5107,
    JRSUIPropScrollBarPart             = 0x5108,
    JRSUIPropVariant                   = 0x5109,
    JRSUIPropWindowType                = 0x510A,
    JRSUIPropShowArrows                = 0x510B,
    JRSUIPropAnimating                 = 0x510C,
    JRSUIPropPresentationState         = 0x510D,
    JRSUIPropUserInterfaceLayoutDirection = 0x510E,
};

static CFMutableDictionaryRef g_jrsuiKeyCache = NULL;
static pthread_mutex_t g_jrsuiKeyLock = PTHREAD_MUTEX_INITIALIZER;

static CFStringRef JRSUIKeyForConstant(int constant)
{
    CFStringRef key = NULL;
    CFNumberRef box = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &constant);

    pthread_mutex_lock(&g_jrsuiKeyLock);
    if (g_jrsuiKeyCache == NULL)
        g_jrsuiKeyCache = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    key = CFDictionaryGetValue(g_jrsuiKeyCache, box);
    if (key == NULL)
    {
        key = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("k%x"), (unsigned)constant);
        CFDictionarySetValue(g_jrsuiKeyCache, box, key);
        CFRelease(key);
    }
    pthread_mutex_unlock(&g_jrsuiKeyLock);

    CFRelease(box);
    return key;
}

static void JRSUIControlSetIntProperty(JRSUIControlRef control, int propertyId, int value)
{
    if (control == NULL) return;
    CFStringRef key = JRSUIKeyForConstant(propertyId);
    CFNumberRef num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &value);
    CFDictionarySetValue(control, key, num);
    CFRelease(num);
}

CFStringRef JRSUIGetKey(int constant)
{
    return JRSUIKeyForConstant(constant);
}

void* JRSUIRendererCreate(void)
{
    return (void*)(uintptr_t)0x1;
}

void* JRSUIRendererRelease(void* renderer)
{
    return NULL;
}

void* JRSUIControlCreate(int isFlipped)
{
    return CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
}

void* JRSUIControlRelease(void* control)
{
    if (control) CFRelease((CFMutableDictionaryRef)control);
    return NULL;
}

void* JRSUIControlSetValueByKey(void* control, CFStringRef key, CFTypeRef value)
{
    if (control && key && value)
        CFDictionarySetValue((CFMutableDictionaryRef)control, key, value);
    return NULL;
}

void* JRSUIControlSetWidget(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropWidget, value); return NULL; }
void* JRSUIControlSetState(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropState, value); return NULL; }
void* JRSUIControlSetSize(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropSize, value); return NULL; }
void* JRSUIControlSetDirection(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropDirection, value); return NULL; }
void* JRSUIControlSetOrientation(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropOrientation, value); return NULL; }
void* JRSUIControlSetAlignmentVertical(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropAlignmentVertical, value); return NULL; }
void* JRSUIControlSetAlignmentHorizontal(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropAlignmentHorizontal, value); return NULL; }
void* JRSUIControlSetSegmentPosition(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropSegmentPosition, value); return NULL; }
void* JRSUIControlSetScrollBarPart(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropScrollBarPart, value); return NULL; }
void* JRSUIControlSetVariant(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropVariant, value); return NULL; }
void* JRSUIControlSetWindowType(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropWindowType, value); return NULL; }
void* JRSUIControlSetShowArrows(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropShowArrows, value); return NULL; }
void* JRSUIControlSetAnimating(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropAnimating, value); return NULL; }
void* JRSUIControlSetPresentationState(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropPresentationState, value); return NULL; }
void* JRSUIControlSetUserInterfaceLayoutDirection(void* control, int value) { JRSUIControlSetIntProperty(control, JRSUIPropUserInterfaceLayoutDirection, value); return NULL; }

void* JRSUIControlDraw(void* renderer, void* control, void* cgContext)
{
    return NULL;
}

int JRSUIControlGetHitPart(void* renderer, void* control)
{
    return 0;
}

double JRSUIControlGetScrollBarOffsetFor(void* control)
{
    return 0.0;
}

void* JRSUIControlGetScrollBarPartBounds(void* outRect, void* control, int part)
{
    if (outRect)
    {
        const double zero[4] = { 0.0, 0.0, 0.0, 0.0 };
        __builtin_memcpy(outRect, zero, sizeof(zero));
    }
    return outRect;
}

int JRSUIControlShouldScrollToClick(void)
{
    return 0;
}

void* do_transfer_port_send_right(void)
{
    if (verbose) puts("STUB: do_transfer_port_send_right called");
    return NULL;
}

void* port_send_right_transfer_server(void)
{
    if (verbose) puts("STUB: port_send_right_transfer_server called");
    return NULL;
}

void* port_send_right_transfer_server_routine(void)
{
    if (verbose) puts("STUB: port_send_right_transfer_server_routine called");
    return NULL;
}

void* transfer_port_send_right(void)
{
    if (verbose) puts("STUB: transfer_port_send_right called");
    return NULL;
}
