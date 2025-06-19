#pragma once

#include "../utils/constants.h"
#include <vector>

namespace PianoSynth {
namespace Physics {

/**
 * Solves the wave equation for string vibration using finite difference methods
 * Implements the full wave equation with stiffness and damping terms
 */
class WaveEquationSolver {
public:
    WaveEquationSolver();
    ~WaveEquationSolver();

    // Initialization
    void initialize(double length, double wave_speed, double stiffness, 
                   double damping, double sample_rate, int spatial_points);
    void reset();
    
    // Boundary conditions
    enum BoundaryType {
        FIXED,      // Fixed end (displacement = 0)
        FREE,       // Free end (force = 0)
        DAMPED      // Damped end (impedance boundary)
    };
    
    void setBoundaryConditions(BoundaryType left, BoundaryType right);
    void setLeftBoundaryImpedance(double impedance);
    void setRightBoundaryImpedance(double impedance);
    
    // Excitation
    void addExcitation(double position, double force);
    void addDistributedExcitation(double start_pos, double end_pos, double force);
    
    // Simulation step
    void step();
    
    // State access
    double getDisplacement(double position) const;
    double getVelocity(double position) const;
    std::vector<double> getDisplacementArray() const { return u_; }
    std::vector<double> getVelocityArray() const { return v_; }
    
    // Physical parameters
    void setWaveSpeed(double speed) { wave_speed_ = speed; updateCoefficients(); }
    void setStiffness(double stiffness) { stiffness_ = stiffness; updateCoefficients(); }
    void setDamping(double damping) { damping_ = damping; updateCoefficients(); }
    
private:
    // Physical parameters
    double length_;
    double wave_speed_;
    double stiffness_;
    double damping_;
    double sample_rate_;
    double dt_;
    double dx_;
    int num_points_;
    
    // Numerical coefficients
    double c1_, c2_, c3_; // Wave equation coefficients
    double d1_, d2_;      // Damping coefficients
    double s1_, s2_;      // Stiffness coefficients
    
    // State arrays
    std::vector<double> u_;     // Current displacement
    std::vector<double> u_1_;   // Previous displacement
    std::vector<double> u_2_;   // Two steps ago displacement
    std::vector<double> v_;     // Current velocity
    std::vector<double> f_;     // Current external forces
    
    // Boundary conditions
    BoundaryType left_boundary_;
    BoundaryType right_boundary_;
    double left_impedance_;
    double right_impedance_;
    
    // Internal methods
    void updateCoefficients();
    void solveWaveEquation();
    void applyBoundaryConditions();
    void updateVelocities();
    void clearForces();
    
    // Boundary condition implementations
    void applyFixedBoundary(int index);
    void applyFreeBoundary(int index);
    void applyDampedBoundary(int index, double impedance);
    
    // Utility functions
    int positionToIndex(double position) const;
    double indexToPosition(int index) const;
    double interpolateArray(const std::vector<double>& array, double position) const;
    bool isValidPosition(double position) const;
    
    // Stability check
    bool isStable() const;
};

} // namespace Physics
} // namespace PianoSynth
