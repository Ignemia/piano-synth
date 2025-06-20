#include "string_model.h"
#include "../utils/math_utils.h"
#include "../utils/constants.h"
#include <cmath>
#include <algorithm>

namespace PianoSynth {
namespace Physics {

StringModel::StringModel(int note_number) 
    : note_number_(note_number),
      fundamental_frequency_(Utils::MathUtils::midiToFrequency(note_number)),
      excitation_force_(0.0),
      excitation_time_(0.0),
      damper_position_(1.0),
      phase_(0.0),
      amplitude_(0.0),
      num_harmonics_(Constants::MAX_HARMONICS) {
    
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

    // [AI GENERATED] Calculate initial inharmonicity coefficient
    double r = diameter_ / 2.0;
    inharmonicity_coefficient_ = (std::pow(Utils::MathUtils::PI, 3) * Constants::YOUNG_MODULUS * std::pow(r, 4)) /
                                 (4.0 * tension_ * length_ * length_);
}

StringModel::~StringModel() = default;

void StringModel::initialize(double sample_rate) {
    sample_rate_ = sample_rate;
    dt_ = 1.0 / sample_rate;
    phase_ = 0.0;
    amplitude_ = 0.0;
    excitation_time_ = 0.0;
    excitation_force_ = 0.0;
    prev_wave_output_ = 0.0;

    updateHarmonics();
    // Enforce CFL condition r = c * dt / dx <= 0.5 for stability
    double min_dx = wave_speed_ * dt_ * 2.0;
    num_points_ = static_cast<int>(length_ / min_dx) + 1;
    num_points_ = std::clamp(num_points_, 32, 128);

    dx_ = length_ / static_cast<double>(num_points_ - 1);
    displacement_.assign(num_points_, 0.0);
    displacement_prev_.assign(num_points_, 0.0);
    displacement_prev2_.assign(num_points_, 0.0);
    velocity_.assign(num_points_, 0.0);
}

void StringModel::reset() {
    phase_ = 0.0;
    amplitude_ = 0.0;
    excitation_force_ = 0.0;
    excitation_time_ = 0.0;
    prev_wave_output_ = 0.0;
}

void StringModel::calculatePhysicalProperties() {
    cross_sectional_area_ = Utils::MathUtils::PI * diameter_ * diameter_ / 4.0;
    linear_density_ = Constants::STRING_DENSITY * cross_sectional_area_;
    wave_speed_ = Utils::MathUtils::calculateStringWaveSpeed(tension_, linear_density_);

    // [AI GENERATED] Update inharmonicity coefficient based on current parameters
    double r = diameter_ / 2.0;
    inharmonicity_coefficient_ = (std::pow(Utils::MathUtils::PI, 3) * Constants::YOUNG_MODULUS * std::pow(r, 4)) /
                                 (4.0 * tension_ * length_ * length_);
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
    
    // [AI GENERATED] Generate additive signal from harmonic table
    double signal = 0.0;
    for (size_t i = 0; i < harmonic_frequencies_.size(); ++i) {
        harmonic_phases_[i] += 2.0 * Utils::MathUtils::PI * harmonic_frequencies_[i] * dt_;
        if (harmonic_phases_[i] > 2.0 * Utils::MathUtils::PI) {
            harmonic_phases_[i] -= 2.0 * Utils::MathUtils::PI;
        }
        signal += harmonic_amplitudes_[i] * sin(harmonic_phases_[i]);
    }
    // [AI GENERATED] Update the full wave equation model
    updateWaveEquation();
    applyBoundaryConditions();
    applyExcitation();
    applyDamping();

    // Sample displacement near the pickup position (1/8 from the end)
    double pickup_pos = 0.125;
    double wave_output = interpolateDisplacement(pickup_pos);

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

    // Blend harmonic model with wave equation output
    double blended = signal * amplitude_ * 0.3 + wave_output * amplitude_;

    // [AI GENERATED] Smooth final output with one-pole low-pass filter
    double alpha = 0.98;
    blended = alpha * blended + (1.0 - alpha) * prev_wave_output_;
    prev_wave_output_ = blended;

    return blended;
}

void StringModel::updateWaveEquation() {
    // [AI GENERATED] Simple finite difference string update
    if (num_points_ < 5) return;

    std::vector<double> new_disp(num_points_, 0.0);
    double r = (wave_speed_ * dt_) / dx_;
    double r2 = r * r;
    double stiffness_factor = stiffness_coefficient_ * dt_ * dt_ / (dx_ * dx_ * dx_ * dx_);

    for (int i = 2; i < num_points_ - 2; ++i) {
        double wave_term = r2 * (displacement_prev_[i + 1] - 2.0 * displacement_prev_[i] + displacement_prev_[i - 1]);
        double stiffness_term = stiffness_factor * (displacement_prev_[i + 2] - 4.0 * displacement_prev_[i + 1] + 6.0 * displacement_prev_[i] - 4.0 * displacement_prev_[i - 1] + displacement_prev_[i - 2]);

        double damp_term = -damping_coefficient_ * (displacement_prev_[i] - displacement_prev2_[i]);
        new_disp[i] = 2.0 * displacement_prev_[i] - displacement_prev2_[i] + wave_term + stiffness_term + damp_term;
    }

    // [AI GENERATED] Apply spatial smoothing to reduce numerical noise
    for (int i = 1; i < num_points_ - 1; ++i) {
        new_disp[i] = 0.25 * new_disp[i - 1] + 0.5 * new_disp[i] + 0.25 * new_disp[i + 1];
    }

    displacement_prev2_ = displacement_prev_;
    displacement_prev_ = new_disp;
    displacement_ = new_disp;
}

void StringModel::applyBoundaryConditions() {
    if (num_points_ < 2) return;

    displacement_prev_[0] = 0.0;
    displacement_prev_[num_points_ - 1] *= (1.0 - damper_position_);
}

void StringModel::applyExcitation() {
    if (excitation_time_ <= excitation_duration_) {
        int index = static_cast<int>((excitation_position_ / length_) * (num_points_ - 1));
        if (index >= 0 && index < num_points_) {
            displacement_prev_[index] += excitation_force_ * dt_ * 0.1;
        }
    }
}

void StringModel::applyDamping() {
    double factor = 1.0 - damping_coefficient_ * dt_;
    for (double &d : displacement_prev_) {
        d *= factor;
    }
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
    // [AI GENERATED] Build harmonic tables with exponential decay
    harmonic_frequencies_.clear();
    harmonic_amplitudes_.clear();
    harmonic_phases_.clear();

    const int max_harmonics = num_harmonics_;
    for (int h = 1; h <= max_harmonics; ++h) {
        double freq = fundamental_frequency_ * static_cast<double>(h) *
                      std::sqrt(1.0 + inharmonicity_coefficient_ * h * h);
        if (freq >= sample_rate_ / 4.0) {

            break; // avoid aliasing
        }
        harmonic_frequencies_.push_back(freq);
        double amp = std::pow(Constants::HARMONIC_DECAY, h - 1) / (static_cast<double>(h) * static_cast<double>(h));
        harmonic_amplitudes_.push_back(amp);
        harmonic_phases_.push_back(0.0);
    }
}

double StringModel::getCurrentAmplitude() const {
    return amplitude_;
}

void StringModel::setTension(double tension) {
    tension_ = tension;
    calculatePhysicalProperties();
    fundamental_frequency_ = wave_speed_ / (2.0 * length_);
    updateHarmonics();
}

void StringModel::setDamperPosition(double position) {
    damper_position_ = Utils::MathUtils::clamp(position, 0.0, 1.0);
}

void StringModel::setLength(double length) {
    length_ = length;
    calculatePhysicalProperties();
    fundamental_frequency_ = wave_speed_ / (2.0 * length_);
    updateHarmonics();
}

void StringModel::setDiameter(double diameter) {
    diameter_ = diameter;
    calculatePhysicalProperties();
    updateHarmonics();
}

void StringModel::setDensity(double density) {
    // Update linear density and recalculate wave speed
    linear_density_ = density * cross_sectional_area_;
    wave_speed_ = Utils::MathUtils::calculateStringWaveSpeed(tension_, linear_density_);
    fundamental_frequency_ = wave_speed_ / (2.0 * length_);
    updateHarmonics();
}

void StringModel::setDamping(double damping) { 
    damping_coefficient_ = damping; 
}

void StringModel::setStiffness(double stiffness) {
    stiffness_coefficient_ = stiffness;
}

void StringModel::setNumHarmonics(int num) {
    num_harmonics_ = std::max(1, num);
    updateHarmonics();
}

void StringModel::setInharmonicityCoefficient(double B) {
    inharmonicity_coefficient_ = B;
    updateHarmonics();
}

void StringModel::setCouplingStrength(double strength) {
    // Store coupling strength for sympathetic resonance effects
    // This affects how much this string resonates with other strings
    (void)strength; // Currently unused but reserved for future implementation
}

} // namespace Physics
} // namespace PianoSynth