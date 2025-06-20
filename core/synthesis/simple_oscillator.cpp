#include "simple_oscillator.h"

namespace PianoSynth {
namespace Synthesis {

SimpleOscillator::SimpleOscillator(double sample_rate)
    : sample_rate_(sample_rate), phase_(0.0), frequency_(440.0) {}

void SimpleOscillator::setFrequency(double frequency) {
    frequency_ = frequency;
}

double SimpleOscillator::nextSample() {
    double sample = sin(phase_);
    phase_ += 2.0 * M_PI * frequency_ / sample_rate_;
    if (phase_ > 2.0 * M_PI) {
        phase_ -= 2.0 * M_PI;
    }
    return sample;
}

void SimpleOscillator::reset() {
    phase_ = 0.0;
}

} // namespace Synthesis
} // namespace PianoSynth
