#pragma once

#include <cmath>

namespace PianoSynth {
namespace Synthesis {

/**
 * @brief [AI GENERATED] Minimal sine wave oscillator.
 *
 * Generates a continuous sine wave at a configured frequency and
 * sample rate. The oscillator phase wraps automatically.
 */
class SimpleOscillator {
public:
    /// Construct oscillator with the given sample rate.
    explicit SimpleOscillator(double sample_rate = 44100.0);

    /// Set the oscillator frequency in Hz.
    void setFrequency(double frequency);

    /// Generate the next sample of the sine wave.
    double nextSample();

    /// Reset the oscillator phase to zero.
    void reset();

private:
    double sample_rate_;
    double phase_;
    double frequency_;
};

} // namespace Synthesis
} // namespace PianoSynth
