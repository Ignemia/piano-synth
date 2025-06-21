#include "NoteSynth.h"
#include <cmath>

/**
 * @brief [AI GENERATED] Generate audio samples using a sine wave.
 */
std::vector<double> NoteSynth::synthesize(const std::vector<NoteEvent>& events, int sampleRate) const {
    std::vector<double> samples;
    for (const auto& e : events) {
        int count = static_cast<int>(e.duration * sampleRate);
        for (int i = 0; i < count; ++i) {
            double value = std::sin(2.0 * M_PI * e.frequency * i / sampleRate);
            samples.push_back(value);
        }
    }
    return samples;
}
