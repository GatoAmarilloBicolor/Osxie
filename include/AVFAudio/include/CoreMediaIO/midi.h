#include <stdint.h>

#pragma once

namespace AVFAudio { namespace CoreMediaIO {

using MIDIEventType = int32_t;

struct MIDIEvent {
    uint8_t channel;
    uint8_t command;
    union {
        struct {
            int16_t argumentHigh;
            int16_t argumentLow;
        } hargs;
        uint32_t data;
    };
};

extern const MIDIEventType av_midi_no_type(void);

} }
