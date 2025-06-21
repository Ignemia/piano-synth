/**
 * @file MidiInput.h
 * @brief [AI GENERATED] Simple MIDI input provider.
 */

#pragma once
#include <vector>

struct MidiMessage {
    int note;
    double duration;
};

/**
 * @brief [AI GENERATED] Generates MIDI events.
 */
class MidiInput {
public:
    std::vector<MidiMessage> generateDemo() const;
};
