/**
 * @file MidiInput.h
 * @brief [AI GENERATED] Simple MIDI input provider.
 */

#pragma once
#include <vector>

/**
 * @brief [AI GENERATED] Represents a single MIDI note message.
 */
struct MidiMessage {
    int note;          /**< MIDI note number. */
    double duration;   /**< Duration of the note in seconds. */
    double startTime;  /**< Start time of the note in seconds. */
};

/**
 * @brief [AI GENERATED] Generates MIDI events.
 */
class MidiInput {
public:
    std::vector<MidiMessage> generateDemo() const;
};
