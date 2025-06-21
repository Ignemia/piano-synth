/**
 * @file NoteSynth.h
 * @brief [AI GENERATED] Basic oscillator-based synth with simple
 * string-decay emulation.
 */

#pragma once
#include <vector>
#include "Abstractor.h"

/**
 * @brief [AI GENERATED] Converts note events into audio samples.
 */
class NoteSynth {
public:
    /**
     * @brief [AI GENERATED] Convert note events to samples using an
     * exponential decay envelope.
     *
     * @param events Sequence of notes to synthesize.
     * @param sampleRate Target sample rate for output audio.
     * @return Vector containing synthesized PCM samples.
     */
    std::vector<double> synthesize(const std::vector<NoteEvent>& events,
                                   int sampleRate = 44100) const;
};
