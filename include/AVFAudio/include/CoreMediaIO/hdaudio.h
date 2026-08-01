#include <stdint.h>

#pragma once

namespace AVFAudio { namespace CoreMediaIO {

using HDAMode = int32_t;

struct HDAudioInfo {
    uint8_t numChannels;
    enum {
        kDefault = 0,
        kMute,
        kDisableHDMIAnnouncement,
    } DeviceType;
};

extern const HDAMode av_hda_default_mode(void);

} }
