#pragma once
#include <X11/Xlib.h>
#include "CGSEvent.h"

uint16_t X11KeySymToMacKeyCode(KeySym ks);
CGSEventFlag X11StateToCGSFlags(unsigned int state);
