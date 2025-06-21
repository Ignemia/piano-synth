/**
 * @file NoteSynth.h
 * @brief [AI GENERATED] Basic oscillator-based synth.
 */

#pragma once
#include <vector>
#include "Abstractor.h"

/**
 * @brief [AI GENERATED] Converts note events into audio samples.
 */
class NoteSynth {
public:
    std::vector<double> synthesize(const std::vector<NoteEvent>& events, int sampleRate = 44100) const;
};
