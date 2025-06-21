#include "NoteSynth.h"
#include <cmath>

/**
 * @brief [AI GENERATED] Generate audio samples using a sine wave with an
 * exponential decay envelope to emulate a piano string losing energy.
 */
std::vector<double> NoteSynth::synthesize(const std::vector<NoteEvent>& events, int sampleRate) const {
    std::vector<double> samples;
    for (const auto& e : events) {
        const int count = static_cast<int>(e.duration * sampleRate);
        for (int i = 0; i < count; ++i) {
            const double phase = 2.0 * M_PI * e.frequency * static_cast<double>(i) / sampleRate;
            const double envelope = std::exp(-3.0 * static_cast<double>(i) / static_cast<double>(count));
            const double value = envelope * std::sin(phase);
            samples.push_back(value);
        }
    }
    return samples;
}
