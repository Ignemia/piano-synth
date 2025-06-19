#pragma once

#include "../utils/constants.h"
#include <vector>

namespace PianoSynth {
namespace Physics {

/**
 * Physical model of a piano hammer striking a string
 * Models the nonlinear contact interaction
 */
class HammerModel {
public:
    HammerModel(int note_number);
    ~HammerModel();

    void initialize(double sample_rate);
    void reset();
    
    // Hammer properties
    void setMass(double mass);
    void setStiffness(double stiffness);
    void setDamping(double damping);
    void setFeltHardness(double hardness);
    
    // Strike parameters
    void strike(double velocity, double strike_position = 0.125); // Default: 1/8 from string end
    
    // Simulation
    double step(double string_displacement); // Returns force applied to string
    bool isInContact() const { return in_contact_; }
    
    // State queries
    double getPosition() const { return position_; }
    double getVelocity() const { return velocity_; }
    double getContactForce() const { return contact_force_; }
    
private:
    // Hammer physical properties
    int note_number_;
    double mass_;
    double stiffness_;
    double damping_;
    double felt_hardness_;
    double strike_position_;
    
    // Simulation state
    double sample_rate_;
    double dt_;
    
    // Hammer motion state
    double position_;
    double velocity_;
    double acceleration_;
    double contact_force_;
    bool in_contact_;
    
    // Contact model parameters
    double contact_stiffness_;
    double contact_damping_;
    double contact_compression_;
    double max_compression_;
    
    // Strike history for modeling felt compression
    std::vector<double> compression_history_;
    int history_length_;
    
    // Internal methods
    void updateHammerMotion();
    double calculateContactForce(double compression, double compression_velocity);
    double calculateFeltStiffness(double compression);
    void updateCompressionHistory(double compression);
    
    // Utility functions
    double getHammerMass(int note_number);
    double getHammerStiffness(int note_number);
};

} // namespace Physics
} // namespace PianoSynth
