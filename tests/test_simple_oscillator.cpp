#include <gtest/gtest.h>
#include "../core/synthesis/simple_oscillator.h"

using namespace PianoSynth::Synthesis;

TEST(SimpleOscillatorTest, GeneratesNonZeroSamples) {
    SimpleOscillator osc(44100.0);
    osc.setFrequency(440.0);
    double first = osc.nextSample();
    double second = osc.nextSample();
    EXPECT_NE(first, second);
}

TEST(SimpleOscillatorTest, PhaseResetWorks) {
    SimpleOscillator osc(44100.0);
    osc.setFrequency(220.0);
    osc.nextSample();
    osc.reset();
    double after_reset = osc.nextSample();
    EXPECT_NEAR(after_reset, 0.0, 1e-6);
}
