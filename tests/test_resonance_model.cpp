#include <gtest/gtest.h>
#include "../core/physics/resonance_model.h"

using namespace PianoSynth::Physics;

/**
 * @brief [AI GENERATED] Tests for ResonanceModel coupling algorithm.
 */
TEST(ResonanceModelTest, CouplingStrengthSemitoneDistance) {
    ResonanceModel model;
    model.initialize(44100.0, 88);
    
    // Sustain pedal fully down to activate resonance
    model.setSustainLevel(1.0);
    // Excite middle C (note 39)
    model.updateStringCoupling(39, 1.0, 261.626);

    // Neighbor note C#
    double near_resonance = model.getSympatheticResonance(40);
    // Distant note high C (note 60)
    double far_resonance = model.getSympatheticResonance(60);

    EXPECT_GT(std::abs(near_resonance), std::abs(far_resonance));
}

