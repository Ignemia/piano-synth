#include "resonance_model.h"
#include "../utils/math_utils.h"
#include "../utils/constants.h"
#include <algorithm>
#include <cmath>

namespace PianoSynth {
namespace Physics {

ResonanceModel::ResonanceModel()
    : sample_rate_(44100.0),
      num_strings_(0) {
}

ResonanceModel::~ResonanceModel() = default;

void ResonanceModel::initialize(double sample_rate, int num_strings) {
    sample_rate_ = sample_rate;
    num_strings_ = num_strings;
    
    // Initialize string coupling arrays
    coupling_matrix_.resize(num_strings_);
    for (auto& row : coupling_matrix_) {
        row.resize(num_strings_, 0.0);
    }
    
    string_displacements_.resize(num_strings_, 0.0);
    string_frequencies_.resize(num_strings_, 0.0);
    sympathetic_forces_.resize(num_strings_, 0.0);
    
    // Initialize string frequencies (MIDI notes 21-108)
    for (int i = 0; i < num_strings_; ++i) {
        string_frequencies_[i] = noteToFrequency(Constants::LOWEST_KEY + i);
    }
    
    calculateCouplingMatrix();
    initializeSoundboardResonators();
    initializeReverbDelays();
    
    reset();
}

void ResonanceModel::reset() {
    std::fill(string_displacements_.begin(), string_displacements_.end(), 0.0);
    std::fill(sympathetic_forces_.begin(), sympathetic_forces_.end(), 0.0);
    
    // Reset soundboard resonators
    for (auto& resonator : soundboard_resonators_) {
        resonator.phase = 0.0;
        resonator.filter_state1 = 0.0;
        resonator.filter_state2 = 0.0;
    }
    
    // Reset reverb delays
    for (auto& delay : reverb_delays_) {
        std::fill(delay.buffer.begin(), delay.buffer.end(), 0.0);
        delay.write_index = 0;
    }
}

void ResonanceModel::updateStringCoupling(int string_index, double displacement, double frequency) {
    if (string_index < 0 || string_index >= num_strings_) {
        return;
    }
    
    string_displacements_[string_index] = displacement;
    string_frequencies_[string_index] = frequency;
    
    updateSympatheticResonance();
}

double ResonanceModel::getSympatheticResonance(int string_index) {
    if (string_index < 0 || string_index >= num_strings_) {
        return 0.0;
    }
    
    return sympathetic_forces_[string_index];
}

double ResonanceModel::processSoundboard(const std::vector<double>& string_outputs) {
    double soundboard_output = 0.0;
    
    // Sum all string outputs
    double total_input = 0.0;
    for (double output : string_outputs) {
        total_input += output;
    }
    
    // Process through soundboard resonators
    for (auto& resonator : soundboard_resonators_) {
        soundboard_output += processResonator(resonator, total_input);
    }
    
    // Normalize output
    soundboard_output /= soundboard_resonators_.size();
    
    return soundboard_output;
}

double ResonanceModel::processRoomAcoustics(double input) {
    double output = input;
    
    // Process through reverb delay lines
    for (auto& delay : reverb_delays_) {
        output += processDelayLine(delay, output) * 0.3; // 30% reverb mix
    }
    
    return output;
}

void ResonanceModel::setSoundboardDamping(double damping) {
    damping = Utils::MathUtils::clamp(damping, 0.0, 1.0);
    
    for (auto& resonator : soundboard_resonators_) {
        resonator.damping = 0.001 + damping * 0.1; // Range: 0.001 to 0.101
    }
}

void ResonanceModel::setSoundboardResonance(double resonance) {
    resonance = Utils::MathUtils::clamp(resonance, 0.0, 1.0);
    
    for (auto& resonator : soundboard_resonators_) {
        resonator.amplitude = 0.1 + resonance * 0.9; // Range: 0.1 to 1.0
    }
}

void ResonanceModel::setRoomSize(double size) {
    size = Utils::MathUtils::clamp(size, 1.0, 100.0);
    
    // Adjust delay line lengths based on room size
    for (size_t i = 0; i < reverb_delays_.size(); ++i) {
        int base_delay = static_cast<int>(sample_rate_ * 0.02); // 20ms base
        int new_delay = static_cast<int>(base_delay * size / 10.0);
        
        if (new_delay != static_cast<int>(reverb_delays_[i].buffer.size())) {
            reverb_delays_[i].buffer.resize(new_delay, 0.0);
            reverb_delays_[i].write_index = 0;
        }
    }
}

void ResonanceModel::setRoomDamping(double damping) {
    damping = Utils::MathUtils::clamp(damping, 0.0, 1.0);
    
    for (auto& delay : reverb_delays_) {
        delay.damping = 0.995 - damping * 0.3; // Range: 0.695 to 0.995
    }
}

void ResonanceModel::calculateCouplingMatrix() {
    // Calculate coupling strength between all string pairs
    for (int i = 0; i < num_strings_; ++i) {
        for (int j = 0; j < num_strings_; ++j) {
            if (i == j) {
                coupling_matrix_[i][j] = 0.0; // No self-coupling
            } else {
                double freq_i = string_frequencies_[i];
                double freq_j = string_frequencies_[j];
                coupling_matrix_[i][j] = calculateCouplingStrength(freq_i, freq_j);
            }
        }
    }
}

void ResonanceModel::updateSympatheticResonance() {
    // Calculate sympathetic forces for each string
    for (int i = 0; i < num_strings_; ++i) {
        sympathetic_forces_[i] = 0.0;
        
        for (int j = 0; j < num_strings_; ++j) {
            if (i != j) {
                double coupling = coupling_matrix_[i][j];
                double displacement = string_displacements_[j];
                sympathetic_forces_[i] += coupling * displacement;
            }
        }
        
        // Apply overall sympathetic resonance strength
        sympathetic_forces_[i] *= Constants::SYMPATHETIC_RESONANCE;
    }
}

double ResonanceModel::processResonator(SoundboardResonator& resonator, double input) {
    // Simple resonant filter using biquad topology
    double frequency = resonator.frequency;
    double q = 1.0 / (2.0 * resonator.damping); // Quality factor
    double amplitude = resonator.amplitude;
    
    // Calculate filter coefficients
    double omega = Utils::MathUtils::TWO_PI * frequency / sample_rate_;
    double sin_omega = sin(omega);
    double cos_omega = cos(omega);
    double alpha = sin_omega / (2.0 * q);
    
    double b0 = amplitude * alpha;
    double b1 = 0.0;
    double b2 = -amplitude * alpha;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cos_omega;
    double a2 = 1.0 - alpha;
    
    // Normalize coefficients
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
    
    // Apply filter
    double output = b0 * input + b1 * 0.0 + b2 * 0.0 - a1 * resonator.filter_state1 - a2 * resonator.filter_state2;
    
    // Update filter states
    resonator.filter_state2 = resonator.filter_state1;
    resonator.filter_state1 = output;
    
    return output;
}

double ResonanceModel::processDelayLine(DelayLine& delay, double input) {
    if (delay.buffer.empty()) {
        return 0.0;
    }
    
    // Get delayed sample
    double delayed_sample = delay.buffer[delay.write_index];
    
    // Write new sample with feedback
    delay.buffer[delay.write_index] = input + delayed_sample * delay.feedback * delay.damping;
    
    // Update write index
    delay.write_index = (delay.write_index + 1) % delay.buffer.size();
    
    return delayed_sample;
}

void ResonanceModel::initializeSoundboardResonators() {
    // Create multiple resonators for different soundboard modes
    soundboard_resonators_.clear();
    
    // Main soundboard resonances (based on typical piano soundboard frequencies)
    std::vector<double> resonant_frequencies = {
        100.0, 150.0, 200.0, 280.0, 350.0, 420.0, 
        500.0, 650.0, 800.0, 1000.0, 1250.0, 1600.0
    };
    
    for (double freq : resonant_frequencies) {
        SoundboardResonator resonator;
        resonator.frequency = freq;
        resonator.amplitude = 0.5;
        resonator.damping = Constants::SOUNDBOARD_DAMPING;
        resonator.phase = 0.0;
        resonator.filter_state1 = 0.0;
        resonator.filter_state2 = 0.0;
        
        soundboard_resonators_.push_back(resonator);
    }
}

void ResonanceModel::initializeReverbDelays() {
    // Create multiple delay lines for reverb (all-pass network)
    reverb_delays_.clear();
    
    // Different delay times for natural reverb
    std::vector<double> delay_times = {0.02, 0.025, 0.03, 0.037, 0.044, 0.051};
    std::vector<double> feedback_amounts = {0.5, 0.4, 0.6, 0.3, 0.7, 0.2};
    
    for (size_t i = 0; i < delay_times.size(); ++i) {
        DelayLine delay;
        int delay_samples = static_cast<int>(delay_times[i] * sample_rate_);
        delay.buffer.resize(delay_samples, 0.0);
        delay.write_index = 0;
        delay.feedback = feedback_amounts[i];
        delay.damping = 0.95; // Default damping
        
        reverb_delays_.push_back(delay);
    }
}

double ResonanceModel::calculateCouplingStrength(double freq1, double freq2) {
    if (freq1 <= 0.0 || freq2 <= 0.0) {
        return 0.0;
    }
    
    // Calculate frequency ratio
    double ratio = freq2 / freq1;
    
    // Strong coupling for harmonic relationships
    double coupling = 0.0;
    
    // Check for harmonic relationships (octaves, fifths, etc.)
    std::vector<double> harmonic_ratios = {2.0, 3.0, 4.0, 5.0, 6.0, 1.5, 2.5, 3.5};
    
    for (double harmonic_ratio : harmonic_ratios) {
        // Check both directions
        if (fabs(ratio - harmonic_ratio) < 0.02) {
            coupling = 0.1 / harmonic_ratio; // Stronger for lower harmonics
            break;
        }
        if (fabs(ratio - 1.0/harmonic_ratio) < 0.02) {
            coupling = 0.1 / harmonic_ratio;
            break;
        }
    }
    
    // Additional coupling for nearby frequencies
    double freq_diff = fabs(freq1 - freq2);
    if (freq_diff < 50.0) { // Within 50 Hz
        coupling += 0.02 * exp(-freq_diff / 20.0);
    }
    
    return Utils::MathUtils::clamp(coupling, 0.0, 0.2);
}

void ResonanceModel::setCouplingStrength(double strength) {
    // Clamp coupling strength to reasonable range
    strength = Utils::MathUtils::clamp(strength, 0.0, 1.0);
    
    // Recalculate coupling matrix with new strength multiplier
    for (int i = 0; i < num_strings_; ++i) {
        for (int j = 0; j < num_strings_; ++j) {
            if (i != j) {
                double base_coupling = calculateCouplingStrength(string_frequencies_[i], string_frequencies_[j]);
                coupling_matrix_[i][j] = base_coupling * strength;
            }
        }
    }
}

double ResonanceModel::noteToFrequency(int note_number) {
    return Utils::MathUtils::midiToFrequency(note_number);
}

} // namespace Physics
} // namespace PianoSynth