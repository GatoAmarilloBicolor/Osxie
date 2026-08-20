#include "X11Keymap.h"
#define XK_MISCELLANY  1
#define XK_LATIN1      1
#define XK_LATIN2      1
#define XK_LATIN3      1
#define XK_LATIN4      1
#define XK_LATIN8      1
#define XK_ARABIC      1
#define XK_CYRILLIC    1
#define XK_GREEK       1
#define XK_TECHNICAL   1
#define XK_SPECIAL     1
#define XK_FUNCTION_KEY 1
#define XK_KEYPAD      1
#define XK_XKB_KEYS    1
#include <X11/keysymdef.h>

static const struct {
    KeySym xkey;
    uint16_t mac;
} table[] = {
    { XK_a, 0x00 }, { XK_s, 0x01 }, { XK_d, 0x02 }, { XK_f, 0x03 },
    { XK_h, 0x04 }, { XK_g, 0x05 }, { XK_z, 0x06 }, { XK_x, 0x07 },
    { XK_c, 0x08 }, { XK_v, 0x09 }, { XK_b, 0x0B }, { XK_q, 0x0C },
    { XK_w, 0x0D }, { XK_e, 0x0E }, { XK_r, 0x0F }, { XK_y, 0x10 },
    { XK_t, 0x11 }, { XK_1, 0x12 }, { XK_2, 0x13 }, { XK_3, 0x14 },
    { XK_4, 0x15 }, { XK_6, 0x16 }, { XK_5, 0x17 }, { XK_equal, 0x18 },
    { XK_9, 0x19 }, { XK_7, 0x1A }, { XK_minus, 0x1B }, { XK_8, 0x1C },
    { XK_0, 0x1D }, { XK_bracketright, 0x1E }, { XK_o, 0x1F },
    { XK_u, 0x20 }, { XK_bracketleft, 0x21 }, { XK_i, 0x22 },
    { XK_p, 0x23 }, { XK_Return, 0x24 }, { XK_l, 0x25 }, { XK_j, 0x26 },
    { XK_apostrophe, 0x27 }, { XK_k, 0x28 }, { XK_semicolon, 0x29 },
    { XK_backslash, 0x2A }, { XK_comma, 0x2B }, { XK_slash, 0x2C },
    { XK_n, 0x2D }, { XK_m, 0x2E }, { XK_period, 0x2F },
    { XK_Tab, 0x30 }, { XK_space, 0x31 }, { XK_grave, 0x32 },
    { XK_BackSpace, 0x33 }, { XK_Escape, 0x35 },
    { XK_Meta_L, 0x37 }, { XK_Shift_L, 0x38 }, { XK_Caps_Lock, 0x39 },
    { XK_Alt_L, 0x3A }, { XK_Control_L, 0x3B },
    { XK_Shift_R, 0x3C }, { XK_Alt_R, 0x3D }, { XK_Control_R, 0x3E },
    { XK_Meta_R, 0x36 },
    { XK_Left, 0x7B }, { XK_Right, 0x7C }, { XK_Down, 0x7D }, { XK_Up, 0x7E },
    { XK_F1, 0x7A }, { XK_F2, 0x78 }, { XK_F3, 0x63 }, { XK_F4, 0x76 },
    { XK_F5, 0x60 }, { XK_F6, 0x61 }, { XK_F7, 0x62 }, { XK_F8, 0x64 },
    { XK_F9, 0x65 }, { XK_F10, 0x6D }, { XK_F11, 0x67 }, { XK_F12, 0x6F },
    { XK_Insert, 0x72 }, { XK_Delete, 0x75 }, { XK_Home, 0x73 },
    { XK_End, 0x77 }, { XK_Page_Up, 0x74 }, { XK_Page_Down, 0x79 },
    { XK_Clear, 0x47 }, { XK_KP_Decimal, 0x41 }, { XK_KP_Enter, 0x4C },
    { {0}, 0 }
};

uint16_t X11KeySymToMacKeyCode(KeySym ks) {
    for (size_t i = 0; table[i].mac != 0 || table[i].xkey != 0; i++) {
        if (table[i].xkey == ks)
            return table[i].mac;
    }
    return 0xFF;
}

CGSEventFlag X11StateToCGSFlags(unsigned int state) {
    CGSEventFlag flags = 0;
    if (state & ShiftMask)   flags |= kCGSEventFlagMaskShift;
    if (state & ControlMask) flags |= kCGSEventFlagMaskControl;
    if (state & Mod1Mask)    flags |= kCGSEventFlagMaskAlternate;
    if (state & Mod4Mask)    flags |= kCGSEventFlagMaskCommand;
    if (state & LockMask)    flags |= kCGSEventFlagMaskAlphaShift;
    return flags;
}
