/**
 * @file Abstractor.h
 * @brief [AI GENERATED] Converts MIDI to note events.
 */

#pragma once
#include <vector>
#include <cmath>
#include "MidiInput.h"

/**
 * @brief [AI GENERATED] Represents a synthesized note event.
 */
struct NoteEvent {
    double frequency;  /**< Frequency of the note in Hz. */
    double duration;   /**< Duration of the note in seconds. */
    double startTime;  /**< Start time of the note in seconds. */
};

/**
 * @brief [AI GENERATED] Translates MIDI messages to frequencies.
 */
class Abstractor {
public:
    std::vector<NoteEvent> convert(const std::vector<MidiMessage>& midi) const;
};
