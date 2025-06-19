#pragma once

#include "../utils/constants.h"
#include <vector>
#include <memory>

namespace PianoSynth {
namespace Physics {

/**
 * Physical model of a piano string using the wave equation
 * with stiffness, damping, and nonlinear effects
 */
class StringModel {
public:
    StringModel(int note_number);
    ~StringModel();

    // Initialization
    void initialize(double sample_rate);
    void reset();
    
    // Physical parameters
    void setTension(double tension);
    void setLength(double length);
    void setDiameter(double diameter);
    void setDensity(double density);
    void setDamping(double damping);
    void setStiffness(double stiffness);
    
    // Excitation and damping
    void excite(double position, double force, double duration);
    void setDamperPosition(double position); // 0.0 = fully damped, 1.0 = open
    
    // String coupling control
    void setCouplingStrength(double strength);
    
    // Simulation
    double step(); // Returns displacement at pickup point
    void updateHarmonics();
    
    // String properties (read-only)
    double getFundamentalFrequency() const { return fundamental_frequency_; }
    double getLength() const { return length_; }
    double getTension() const { return tension_; }
    double getCurrentAmplitude() const;
    
private:
    // String physical properties
    int note_number_;
    double fundamental_frequency_;
    double length_;
    double diameter_;
    double cross_sectional_area_;
    double tension_;
    double linear_density_;
    double wave_speed_;
    double damping_coefficient_;
    double stiffness_coefficient_;
    
    // Simulation parameters
    double sample_rate_;
    double dt_; // Time step
    double dx_; // Spatial step
    int num_points_; // Number of spatial discretization points
    
    // String state arrays (current and previous)
    std::vector<double> displacement_;
    std::vector<double> displacement_prev_;
    std::vector<double> displacement_prev2_;
    std::vector<double> velocity_;
    
    // Excitation state
    double excitation_force_;
    double excitation_position_;
    double excitation_duration_;
    double excitation_time_;
    
    // Damper state
    double damper_position_;
    double damper_force_;
    
    // Simplified oscillator state
    double phase_;
    double amplitude_;
    
    // Harmonic content
    std::vector<double> harmonic_amplitudes_;
    std::vector<double> harmonic_phases_;
    std::vector<double> harmonic_frequencies_;
    
    // Internal methods
    void calculatePhysicalProperties();
    void updateWaveEquation();
    void applyBoundaryConditions();
    void applyExcitation();
    void applyDamping();
    void applyStiffness();
    double calculateStiffnessEffect(int point);
    double interpolateDisplacement(double position);
    
    // Utility functions
    double noteToFrequency(int note_number);
    double calculateStringLength(double frequency, double tension, double linear_density);
};

} // namespace Physics
} // namespace PianoSynth
