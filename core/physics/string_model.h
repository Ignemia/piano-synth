#pragma once
// [AI GENERATED]

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
    /**
     * \brief [AI GENERATED] Set the number of harmonics to generate.
     */
    void setNumHarmonics(int num);

    /**
     * \brief [AI GENERATED] Explicitly set the inharmonicity coefficient B.
     */
    void setInharmonicityCoefficient(double B);
    
    // Excitation and damping
    void excite(double position, double force, double duration);
    void setDamperPosition(double position); // 0.0 = fully damped, 1.0 = open
    
    // String coupling control
    void setCouplingStrength(double strength);
    
    // Simulation
    /**
     * [AI GENERATED] Advance the string model by one sample and return
     * the current displacement at the pickup position.
     */
    double step();

    /**
     * [AI GENERATED] Precompute harmonic frequencies and amplitudes for
     * additive synthesis. Called on initialization and whenever the
     * fundamental changes.
     */
    void updateHarmonics();
    
    // String properties (read-only)
    double getFundamentalFrequency() const { return fundamental_frequency_; }
    double getLength() const { return length_; }
    double getTension() const { return tension_; }
    /**
     * \brief [AI GENERATED] Retrieve the internal damping coefficient.
     */
    double getDamping() const { return damping_coefficient_; }
    double getCurrentAmplitude() const;

    /**
     * [AI GENERATED] Get the inharmonicity coefficient B for this string.
     */
    double getInharmonicityCoefficient() const { return inharmonicity_coefficient_; }

    /**
     * [AI GENERATED] Return the frequency of a specific harmonic including
     * inharmonicity effects.
     */
    double getHarmonicFrequency(size_t index) const {
        return (index < harmonic_frequencies_.size()) ? harmonic_frequencies_[index] : 0.0;
    }

    /**
     * [AI GENERATED] Get the number of harmonics currently modeled.
     */
    size_t getNumHarmonics() const { return harmonic_frequencies_.size(); }

    /**
     * [AI GENERATED] Get the amplitude of the specified harmonic.
     */
    double getHarmonicAmplitude(size_t index) const {
        return (index < harmonic_amplitudes_.size()) ? harmonic_amplitudes_[index] : 0.0;
    }
    
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
    // [AI GENERATED] Previous output sample for output low-pass filtering
    double prev_wave_output_;
    
    // Harmonic content
    std::vector<double> harmonic_amplitudes_;
    std::vector<double> harmonic_phases_;
    std::vector<double> harmonic_frequencies_;
    
    // Internal methods
    void calculatePhysicalProperties();
    /**
     * [AI GENERATED] Update the internal wave equation using a finite
     * difference scheme. This advances the displacement arrays by one
     * time step.
     */
    void updateWaveEquation();

    /**
     * [AI GENERATED] Apply the boundary conditions at both ends of the
     * string. The left end is fixed while the right end is damped by the
     * current damper position.
     */
    void applyBoundaryConditions();

    /**
     * [AI GENERATED] Inject excitation force into the discretized string
     * during the striking phase of the hammer.
     */
    void applyExcitation();

    /**
     * [AI GENERATED] Apply global damping to the string displacement to
     * emulate energy loss to the surroundings and the damper.
     */
    void applyDamping();
    void applyStiffness();
    double calculateStiffnessEffect(int point);
    double interpolateDisplacement(double position);
    
    // Utility functions
    double noteToFrequency(int note_number);
    double calculateStringLength(double frequency, double tension, double linear_density);

    // Inharmonicity coefficient
    double inharmonicity_coefficient_;
    // [AI GENERATED] Maximum number of harmonics to model
    int num_harmonics_;
};

} // namespace Physics
} // namespace PianoSynth
