#include "hammer_model.h"
#include "../utils/constants.h"
#include "../utils/math_utils.h"
#include <algorithm>
#include <cmath>

namespace PianoSynth {
namespace Physics {

HammerModel::HammerModel(int note_number) 
    : note_number_(note_number),
      mass_(getHammerMass(note_number)),
      stiffness_(getHammerStiffness(note_number)),
      damping_(Constants::HAMMER_DAMPING),
      felt_hardness_(0.5),
      strike_position_(0.125),
      sample_rate_(44100.0),
      dt_(1.0 / 44100.0),
      position_(0.0),
      velocity_(0.0),
      acceleration_(0.0),
      contact_force_(0.0),
      in_contact_(false),
      contact_stiffness_(Constants::HAMMER_STIFFNESS),
      contact_damping_(damping_),
      contact_compression_(0.0),
      max_compression_(0.001),
      history_length_(10) {
    
    compression_history_.resize(history_length_, 0.0);
}

HammerModel::~HammerModel() = default;

void HammerModel::initialize(double sample_rate) {
    sample_rate_ = sample_rate;
    dt_ = 1.0 / sample_rate;
    
    reset();
}

void HammerModel::reset() {
    position_ = 0.0;
    velocity_ = 0.0;
    acceleration_ = 0.0;
    contact_force_ = 0.0;
    in_contact_ = false;
    contact_compression_ = 0.0;
    
    std::fill(compression_history_.begin(), compression_history_.end(), 0.0);
}

void HammerModel::setMass(double mass) {
    mass_ = std::max(mass, 0.001); // Minimum mass to avoid division by zero
}

void HammerModel::setStiffness(double stiffness) {
    stiffness_ = std::max(stiffness, 0.0);
}

void HammerModel::setDamping(double damping) {
    damping_ = std::max(damping, 0.0);
}

void HammerModel::setFeltHardness(double hardness) {
    felt_hardness_ = Utils::MathUtils::clamp(hardness, 0.1, 1.0);
}

void HammerModel::strike(double velocity, double strike_position) {
    velocity_ = Utils::MathUtils::clamp(velocity, 0.1, 10.0); // Reasonable velocity range
    strike_position_ = Utils::MathUtils::clamp(strike_position, 0.0, 1.0);
    position_ = -0.001; // Start slightly away from string for smoother contact
    in_contact_ = false;
    contact_force_ = 0.0;
    contact_compression_ = 0.0;
    
    // Calculate contact stiffness based on felt hardness with smoother scaling
    contact_stiffness_ = stiffness_ * (0.3 + 0.7 * felt_hardness_);
    contact_damping_ = damping_ * (0.5 + 0.5 * felt_hardness_);
}

double HammerModel::step(double string_displacement) {
    // Calculate compression (how much hammer and string overlap)
    contact_compression_ = std::max(0.0, position_ - string_displacement);
    
    // Use hysteresis for contact detection to prevent chattering
    static double contact_threshold = 1e-6;
    static double release_threshold = 5e-7;
    
    if (!in_contact_ && contact_compression_ > contact_threshold) {
        in_contact_ = true;
    } else if (in_contact_ && contact_compression_ < release_threshold) {
        in_contact_ = false;
    }
    
    if (in_contact_) {
        // Calculate contact force with smoothed compression velocity
        static double prev_compression = 0.0;
        double compression_velocity = (contact_compression_ - prev_compression) / dt_;
        compression_velocity = Utils::MathUtils::clamp(compression_velocity, -1000.0, 1000.0);
        prev_compression = contact_compression_;
        
        contact_force_ = calculateContactForce(contact_compression_, compression_velocity);
        
        // Update compression history for felt modeling
        updateCompressionHistory(contact_compression_);
    } else {
        contact_force_ = 0.0;
    }
    
    // Update hammer motion
    updateHammerMotion();
    
    // Apply gentle smoothing to force output
    static double prev_force = 0.0;
    double smoothed_force = 0.7 * contact_force_ + 0.3 * prev_force;
    prev_force = smoothed_force;
    
    return smoothed_force;
}

void HammerModel::updateHammerMotion() {
    // Calculate acceleration from contact force with safety limits
    acceleration_ = -Utils::MathUtils::clamp(contact_force_, 0.0, 50000.0) / mass_;
    
    // Add gravity (small effect) - reduced for stability
    acceleration_ -= 2.0;
    
    // Add air resistance (very small effect) - increased damping for stability
    acceleration_ -= 0.5 * velocity_;
    
    // Limit acceleration to prevent numerical instability
    acceleration_ = Utils::MathUtils::clamp(acceleration_, -10000.0, 10000.0);
    
    // Update velocity and position using improved Verlet integration
    double new_velocity = velocity_ + acceleration_ * dt_;
    new_velocity = Utils::MathUtils::clamp(new_velocity, -100.0, 100.0);
    
    double new_position = position_ + new_velocity * dt_;
    
    // Apply position constraints more smoothly
    if (new_position < -0.01) {
        new_position = -0.01;
        new_velocity = std::max(0.0, new_velocity); // Don't move further away
    }
    
    velocity_ = new_velocity;
    position_ = new_position;
}

double HammerModel::calculateContactForce(double compression, double compression_velocity) {
    if (compression <= 0.0) {
        return 0.0;
    }
    
    // Nonlinear contact stiffness (Hertzian contact) with smoother curve
    double felt_stiffness = calculateFeltStiffness(compression);
    double power_factor = 1.2 + 0.3 * felt_hardness_; // Vary power with felt hardness
    double elastic_force = felt_stiffness * pow(compression, power_factor);
    
    // Damping force proportional to compression velocity with nonlinear term
    double velocity_factor = 1.0 + 0.1 * std::abs(compression_velocity);
    double damping_force = contact_damping_ * compression_velocity * velocity_factor;
    
    // Total force with smooth combination
    double total_force = elastic_force + damping_force;
    
    // Apply gentle compression to prevent harsh transients
    total_force = tanh(total_force / 5000.0) * 5000.0;
    
    // Limit maximum force to prevent instability with softer limiting
    double max_force = 8000.0; // Reduced maximum force
    return Utils::MathUtils::clamp(total_force, 0.0, max_force);
}

double HammerModel::calculateFeltStiffness(double compression) {
    // Felt stiffness increases with compression (nonlinear) with smoother curve
    double compression_ratio = compression / max_compression_;
    compression_ratio = Utils::MathUtils::clamp(compression_ratio, 0.0, 1.0);
    
    // Smoother stiffening curve using tanh
    double stiffness_factor = 1.0 + 5.0 * tanh(2.0 * compression_ratio);
    
    // Include felt hardness effect more smoothly
    double hardness_effect = 0.5 + 0.5 * felt_hardness_;
    
    return contact_stiffness_ * stiffness_factor * hardness_effect;
}

void HammerModel::updateCompressionHistory(double compression) {
    // Shift history
    for (int i = history_length_ - 1; i > 0; --i) {
        compression_history_[i] = compression_history_[i-1];
    }
    
    compression_history_[0] = compression;
}

double HammerModel::getHammerMass(int note_number) {
    // Hammer mass decreases with note number (higher notes have lighter hammers)
    double base_mass = Constants::HAMMER_MASS;
    double note_factor = 1.0 - (note_number - Constants::LOWEST_KEY) * 0.8 / 
                        (Constants::HIGHEST_KEY - Constants::LOWEST_KEY);
    note_factor = Utils::MathUtils::clamp(note_factor, 0.2, 1.0);
    
    return base_mass * note_factor;
}

double HammerModel::getHammerStiffness(int note_number) {
    // Hammer stiffness varies with note (empirical relationship)
    double base_stiffness = Constants::HAMMER_STIFFNESS;
    double note_factor = 1.0 + (note_number - 60) * 0.01; // Slightly stiffer for higher notes
    note_factor = Utils::MathUtils::clamp(note_factor, 0.5, 2.0);
    
    return base_stiffness * note_factor;
}

} // namespace Physics
} // namespace PianoSynth
