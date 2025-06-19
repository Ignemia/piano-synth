#pragma once

#include "../utils/constants.h"
#include <vector>
#include <memory>

namespace PianoSynth {
namespace Physics {

/**
 * Models the piano soundboard resonance and coupling between strings
 * Includes sympathetic resonance and soundboard filtering
 */
class ResonanceModel {
public:
    ResonanceModel();
    ~ResonanceModel();

    void initialize(double sample_rate, int num_strings);
    void reset();
    
    // String coupling
    void updateStringCoupling(int string_index, double displacement, double frequency);
    double getSympatheticResonance(int string_index);
    
    // Soundboard modeling
    double processSoundboard(const std::vector<double>& string_outputs);
    void setSoundboardDamping(double damping);
    void setSoundboardResonance(double resonance);
    
    // Room acoustics (simple reverb model)
    double processRoomAcoustics(double input);
    void setRoomSize(double size);
    void setRoomDamping(double damping);
    
    // String coupling control
    void setCouplingStrength(double strength);
    
private:
    double sample_rate_;
    int num_strings_;
    
    // String coupling matrix
    std::vector<std::vector<double>> coupling_matrix_;
    std::vector<double> string_displacements_;
    std::vector<double> string_frequencies_;
    std::vector<double> sympathetic_forces_;
    
    // Soundboard model (simplified as a multi-resonant filter)
    struct SoundboardResonator {
        double frequency;
        double amplitude;
        double damping;
        double phase;
        double filter_state1;
        double filter_state2;
    };
    std::vector<SoundboardResonator> soundboard_resonators_;
    
    // Room acoustics (all-pass reverb)
    struct DelayLine {
        std::vector<double> buffer;
        int write_index;
        double feedback;
        double damping;
    };
    std::vector<DelayLine> reverb_delays_;
    
    // Internal methods
    void calculateCouplingMatrix();
    void updateSympatheticResonance();
    double processResonator(SoundboardResonator& resonator, double input);
    double processDelayLine(DelayLine& delay, double input);
    void initializeSoundboardResonators();
    void initializeReverbDelays();
    
    // Utility functions
    double calculateCouplingStrength(double freq1, double freq2);
    double noteToFrequency(int note_number);
};

} // namespace Physics
} // namespace PianoSynth
