#include "NoteSynth.h"
#include <cmath>
#include <cstdlib>

/**
 * @brief [AI GENERATED] Generate samples with a sustain phase and hammer noise.
 */
std::vector<double> NoteSynth::synthesize(const std::vector<NoteEvent>& events,
                                          int sampleRate) const {
    std::vector<double> samples;
    const double kHammerTime = 0.02;
    const double kSustainFraction = 0.7;
    for (const auto& e : events) {
        const int count = static_cast<int>(e.duration * sampleRate);
        int hammerSamples = static_cast<int>(kHammerTime * sampleRate);
        if (hammerSamples > count) {
            hammerSamples = count;
        }
        const int sustainStart = static_cast<int>(kSustainFraction * count);
        for (int i = 0; i < count; ++i) {
            const double phase = 2.0 * M_PI * e.frequency * static_cast<double>(i) / sampleRate;
            double value = 0.0;
            if (i < hammerSamples) {
                const double noise = static_cast<double>(std::rand()) / RAND_MAX * 2.0 - 1.0;
                value += 0.3 * noise;
            }
            double envelope = 1.0;
            if (i > sustainStart) {
                const double release = static_cast<double>(i - sustainStart);
                const double releaseLen = static_cast<double>(count - sustainStart);
                envelope = std::exp(-3.0 * release / releaseLen);
            }
            value += envelope * std::sin(phase);
            samples.push_back(value);
        }
    }
    return samples;
}
