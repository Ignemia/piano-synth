/**
 * @file NoteSynth.h
 * @brief [AI GENERATED] Basic oscillator-based synth emulating string
 * harmonics with subtle hammer noise.
 */

#pragma once
#include <vector>
#include "Abstractor.h"

/**
 * @brief [AI GENERATED] Converts note events into audio samples. Supports
 * overlapping notes for simple chord playback.
 */
class NoteSynth {
public:
    /**
     * @brief [AI GENERATED] Convert note events to samples using an
     * attack-sustain-release envelope and multiple harmonics.
     *
     * @param events Sequence of notes to synthesize.
     * @param sampleRate Target sample rate for output audio.
     * @return Vector containing synthesized PCM samples.
     */
    std::vector<double> synthesize(const std::vector<NoteEvent>& events,
                                   int sampleRate = 44100) const;
};
