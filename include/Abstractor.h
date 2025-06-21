/**
 * @file Abstractor.h
 * @brief [AI GENERATED] Converts MIDI to note events.
 */

#pragma once
#include <vector>
#include <cmath>
#include "MidiInput.h"

struct NoteEvent {
    double frequency;
    double duration;
};

/**
 * @brief [AI GENERATED] Translates MIDI messages to frequencies.
 */
class Abstractor {
public:
    std::vector<NoteEvent> convert(const std::vector<MidiMessage>& midi) const;
};
