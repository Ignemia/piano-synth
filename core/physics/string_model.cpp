#include "string_model.h"
#include "../utils/math_utils.h"
#include "../utils/constants.h"
#include <cmath>

namespace PianoSynth {
namespace Physics {

StringModel::StringModel(int note_number) 
    : note_number_(note_number),
      fundamental_frequency_(Utils::MathUtils::midiToFrequency(note_number)),
      excitation_force_(0.0),
      excitation_time_(0.0),
      damper_position_(1.0),
      phase_(0.0),
      amplitude_(0.0) {
    
    // Set basic physical properties
    diameter_ = 0.0008 + (108 - note_number) * 0.00003;
    tension_ = Constants::STRING_TENSION_BASE * (0.5 + (note_number - 21) * 0.008);
    damping_coefficient_ = 0.02 + 0.01 * (fundamental_frequency_ / 440.0);
    stiffness_coefficient_ = Constants::STRING_STIFFNESS;
    
    // Calculate cross-sectional area and linear density
    cross_sectional_area_ = Utils::MathUtils::PI * diameter_ * diameter_ / 4.0;
    linear_density_ = Constants::STRING_DENSITY * cross_sectional_area_;
    
    // Calculate string length for the desired fundamental frequency
    wave_speed_ = Utils::MathUtils::calculateStringWaveSpeed(tension_, linear_density_);
    length_ = wave_speed_ / (2.0 * fundamental_frequency_);
    length_ = Utils::MathUtils::clamp(length_, 0.1, 2.5);
}

StringModel::~StringModel() = default;

void StringModel::initialize(double sample_rate) {
    sample_rate_ = sample_rate;
    dt_ = 1.0 / sample_rate;
    phase_ = 0.0;
    amplitude_ = 0.0;
    excitation_time_ = 0.0;
    excitation_force_ = 0.0;
}

void StringModel::reset() {
    phase_ = 0.0;
    amplitude_ = 0.0;
    excitation_force_ = 0.0;
    excitation_time_ = 0.0;
}

void StringModel::calculatePhysicalProperties() {
    cross_sectional_area_ = Utils::MathUtils::PI * diameter_ * diameter_ / 4.0;
    linear_density_ = Constants::STRING_DENSITY * cross_sectional_area_;
    wave_speed_ = Utils::MathUtils::calculateStringWaveSpeed(tension_, linear_density_);
}

void StringModel::excite(double position, double force, double duration) {
    excitation_position_ = Utils::MathUtils::clamp(position, 0.1, 0.9) * length_;
    excitation_force_ = Utils::MathUtils::clamp(force, 0.0, 10.0);
    excitation_duration_ = Utils::MathUtils::clamp(duration, 0.0001, 0.01);
    excitation_time_ = 0.0;
    amplitude_ = excitation_force_;
    phase_ = 0.0;
}

/**
 * [AI GENERATED] Advance the string simulation by one sample.
 * Returns the current displacement at the pickup point.
 */
double StringModel::step() {
    if (excitation_force_ == 0.0 || amplitude_ < 1e-12) {
        return 0.0;
    }
    
    excitation_time_ += dt_;
    
    // Generate oscillation at fundamental frequency
    phase_ += 2.0 * Utils::MathUtils::PI * fundamental_frequency_ * dt_;
    if (phase_ > 2.0 * Utils::MathUtils::PI) {
        phase_ -= 2.0 * Utils::MathUtils::PI;
    }
    
    // Create rich harmonic content
    double fundamental = sin(phase_);
    double harmonic2 = 0.4 * sin(2.0 * phase_);
    double harmonic3 = 0.2 * sin(3.0 * phase_);
    double harmonic4 = 0.1 * sin(4.0 * phase_);
    double harmonic5 = 0.05 * sin(5.0 * phase_);
    
    double signal = fundamental + harmonic2 + harmonic3 + harmonic4 + harmonic5;
    
    // [AI GENERATED] Apply realistic string decay with pedal influence
    // Base decay from air damping
    double decay_rate = damping_coefficient_;
    amplitude_ *= (1.0 - decay_rate * dt_);

    // Extra damping from the damper when pedal is released. The damper
    // increases decay proportional to how far it is pressed.
    double damper_factor = 1.0 + (1.0 - damper_position_) * 20.0;
    amplitude_ *= (1.0 - decay_rate * damper_factor * dt_);
    
    // Apply velocity-dependent brightness
    double velocity_brightness = Utils::MathUtils::clamp(excitation_force_ / 5.0, 0.3, 1.0);
    signal *= velocity_brightness;
    
    return signal * amplitude_ * 0.3; // Scale for reasonable output level
}

void StringModel::updateWaveEquation() {
    // Not used in simplified model
}

void StringModel::applyBoundaryConditions() {
    // Not used in simplified model
}

void StringModel::applyExcitation() {
    // Not used in simplified model
}

void StringModel::applyDamping() {
    // Not used in simplified model
}

double StringModel::calculateStiffnessEffect(int point) {
    // Simple stiffness model - higher frequency modes are damped more
    if (point <= 0 || point >= num_points_) {
        return 0.0;
    }
    
    // Stiffness affects higher harmonics more
    double mode_factor = static_cast<double>(point) / num_points_;
    return stiffness_coefficient_ * mode_factor * mode_factor;
}

double StringModel::interpolateDisplacement(double position) {
    // Clamp position to valid range [0, 1]
    position = Utils::MathUtils::clamp(position, 0.0, 1.0);
    
    // Convert to array index
    double scaled_pos = position * (num_points_ - 1);
    int index = static_cast<int>(scaled_pos);
    double fraction = scaled_pos - index;
    
    // Linear interpolation between adjacent points
    if (index >= num_points_ - 1) {
        return displacement_[num_points_ - 1];
    }
    
    return displacement_[index] * (1.0 - fraction) + displacement_[index + 1] * fraction;
}

void StringModel::updateHarmonics() {
    // Not used in simplified model
}

double StringModel::getCurrentAmplitude() const {
    return amplitude_;
}

void StringModel::setTension(double tension) {
    tension_ = tension;
    calculatePhysicalProperties();
    fundamental_frequency_ = wave_speed_ / (2.0 * length_);
}

void StringModel::setDamperPosition(double position) {
    damper_position_ = Utils::MathUtils::clamp(position, 0.0, 1.0);
}

void StringModel::setLength(double length) { 
    length_ = length; 
    calculatePhysicalProperties(); 
}

void StringModel::setDiameter(double diameter) { 
    diameter_ = diameter; 
    calculatePhysicalProperties(); 
}

void StringModel::setDensity(double density) { 
    // Update linear density and recalculate wave speed
    linear_density_ = density * cross_sectional_area_;
    wave_speed_ = Utils::MathUtils::calculateStringWaveSpeed(tension_, linear_density_);
    fundamental_frequency_ = wave_speed_ / (2.0 * length_);
}

void StringModel::setDamping(double damping) { 
    damping_coefficient_ = damping; 
}

void StringModel::setStiffness(double stiffness) { 
    stiffness_coefficient_ = stiffness; 
}

void StringModel::setCouplingStrength(double strength) {
    // Store coupling strength for sympathetic resonance effects
    // This affects how much this string resonates with other strings
    (void)strength; // Currently unused but reserved for future implementation
}

} // namespace Physics
} // namespace PianoSynth