#include "wave_equation_solver.h"
#include "../utils/math_utils.h"
#include <algorithm>
#include <cmath>

namespace PianoSynth {
namespace Physics {

WaveEquationSolver::WaveEquationSolver() 
    : length_(0.0), wave_speed_(0.0), stiffness_(0.0), damping_(0.0),
      sample_rate_(0.0), dt_(0.0), dx_(0.0), num_points_(0),
      left_boundary_(FIXED), right_boundary_(FIXED),
      left_impedance_(0.0), right_impedance_(0.0) {
}

WaveEquationSolver::~WaveEquationSolver() = default;

void WaveEquationSolver::initialize(double length, double wave_speed, double stiffness,
                                   double damping, double sample_rate, int spatial_points) {
    length_ = length;
    wave_speed_ = wave_speed;
    stiffness_ = stiffness;
    damping_ = damping;
    sample_rate_ = sample_rate;
    dt_ = 1.0 / sample_rate;
    num_points_ = spatial_points;
    dx_ = length / (num_points_ - 1);
    
    // Initialize state arrays
    u_.resize(num_points_, 0.0);
    u_1_.resize(num_points_, 0.0);
    u_2_.resize(num_points_, 0.0);
    v_.resize(num_points_, 0.0);
    f_.resize(num_points_, 0.0);
    
    // Set default boundary conditions
    setBoundaryConditions(FIXED, FIXED);
    
    updateCoefficients();
    reset();
}

void WaveEquationSolver::reset() {
    std::fill(u_.begin(), u_.end(), 0.0);
    std::fill(u_1_.begin(), u_1_.end(), 0.0);
    std::fill(u_2_.begin(), u_2_.end(), 0.0);
    std::fill(v_.begin(), v_.end(), 0.0);
    std::fill(f_.begin(), f_.end(), 0.0);
}

void WaveEquationSolver::setBoundaryConditions(BoundaryType left, BoundaryType right) {
    left_boundary_ = left;
    right_boundary_ = right;
}

void WaveEquationSolver::setLeftBoundaryImpedance(double impedance) {
    left_impedance_ = impedance;
}

void WaveEquationSolver::setRightBoundaryImpedance(double impedance) {
    right_impedance_ = impedance;
}

void WaveEquationSolver::addExcitation(double position, double force) {
    if (!isValidPosition(position)) return;
    
    int index = positionToIndex(position);
    if (index >= 0 && index < num_points_) {
        f_[index] += force;
    }
}

void WaveEquationSolver::addDistributedExcitation(double start_pos, double end_pos, double force) {
    if (!isValidPosition(start_pos) || !isValidPosition(end_pos)) return;
    
    int start_index = positionToIndex(start_pos);
    int end_index = positionToIndex(end_pos);
    
    if (start_index > end_index) {
        std::swap(start_index, end_index);
    }
    
    double distributed_force = force / (end_index - start_index + 1);
    
    for (int i = start_index; i <= end_index; ++i) {
        if (i >= 0 && i < num_points_) {
            f_[i] += distributed_force;
        }
    }
}

void WaveEquationSolver::step() {
    solveWaveEquation();
    applyBoundaryConditions();
    updateVelocities();
    clearForces();
}

double WaveEquationSolver::getDisplacement(double position) const {
    return interpolateArray(u_, position);
}

double WaveEquationSolver::getVelocity(double position) const {
    return interpolateArray(v_, position);
}

void WaveEquationSolver::updateCoefficients() {
    if (dt_ <= 0.0 || dx_ <= 0.0) return;
    
    double r = (wave_speed_ * dt_) / dx_;
    double r2 = r * r;
    
    // Standard wave equation coefficients
    c1_ = 2.0 - 2.0 * r2;
    c2_ = r2;
    c3_ = -1.0;
    
    // Damping coefficients
    d1_ = damping_ * dt_;
    d2_ = 1.0 / (1.0 + d1_);
    
    // Stiffness coefficients (for fourth-order derivative)
    double stiffness_factor = stiffness_ * dt_ * dt_ / (dx_ * dx_ * dx_ * dx_);
    s1_ = stiffness_factor;
    s2_ = -4.0 * stiffness_factor;
}

void WaveEquationSolver::solveWaveEquation() {
    for (int i = 1; i < num_points_ - 1; ++i) {
        // Standard wave equation term
        double wave_term = c2_ * (u_1_[i+1] + u_1_[i-1]) + c1_ * u_1_[i] + c3_ * u_2_[i];
        
        // Stiffness term (fourth derivative approximation)
        double stiffness_term = 0.0;
        if (i >= 2 && i < num_points_ - 2) {
            stiffness_term = s1_ * (u_1_[i+2] + u_1_[i-2]) + s2_ * (u_1_[i+1] + u_1_[i-1]) + 
                           6.0 * s1_ * u_1_[i];
        }
        
        // External force term
        double force_term = f_[i] * dt_ * dt_;
        
        // Update displacement
        u_[i] = d2_ * (wave_term + stiffness_term + force_term);
    }
}

void WaveEquationSolver::applyBoundaryConditions() {
    // Left boundary
    switch (left_boundary_) {
        case FIXED:
            applyFixedBoundary(0);
            break;
        case FREE:
            applyFreeBoundary(0);
            break;
        case DAMPED:
            applyDampedBoundary(0, left_impedance_);
            break;
    }
    
    // Right boundary
    switch (right_boundary_) {
        case FIXED:
            applyFixedBoundary(num_points_ - 1);
            break;
        case FREE:
            applyFreeBoundary(num_points_ - 1);
            break;
        case DAMPED:
            applyDampedBoundary(num_points_ - 1, right_impedance_);
            break;
    }
}

void WaveEquationSolver::updateVelocities() {
    for (int i = 0; i < num_points_; ++i) {
        v_[i] = (u_[i] - u_1_[i]) / dt_;
    }
    
    // Update history
    u_2_ = u_1_;
    u_1_ = u_;
}

void WaveEquationSolver::clearForces() {
    std::fill(f_.begin(), f_.end(), 0.0);
}

void WaveEquationSolver::applyFixedBoundary(int index) {
    u_[index] = 0.0;
}

void WaveEquationSolver::applyFreeBoundary(int index) {
    // Free boundary: du/dx = 0 at boundary
    if (index == 0) {
        u_[0] = u_[1];
    } else if (index == num_points_ - 1) {
        u_[num_points_ - 1] = u_[num_points_ - 2];
    }
}

void WaveEquationSolver::applyDampedBoundary(int index, double impedance) {
    // Damped boundary with specified impedance
    double impedance_factor = impedance * dt_ / dx_;
    
    if (index == 0) {
        u_[0] = (u_[1] - impedance_factor * v_[0]) / (1.0 + impedance_factor);
    } else if (index == num_points_ - 1) {
        u_[num_points_ - 1] = (u_[num_points_ - 2] - impedance_factor * v_[num_points_ - 1]) / 
                             (1.0 + impedance_factor);
    }
}

int WaveEquationSolver::positionToIndex(double position) const {
    return static_cast<int>(position / dx_);
}

double WaveEquationSolver::indexToPosition(int index) const {
    return index * dx_;
}

double WaveEquationSolver::interpolateArray(const std::vector<double>& array, double position) const {
    if (!isValidPosition(position) || array.empty()) {
        return 0.0;
    }
    
    double index_f = position / dx_;
    int index = static_cast<int>(index_f);
    double frac = index_f - index;
    
    if (index >= 0 && index < static_cast<int>(array.size()) - 1) {
        return Utils::MathUtils::linearInterpolate(array[index], array[index + 1], frac);
    } else if (index == static_cast<int>(array.size()) - 1) {
        return array[index];
    }
    
    return 0.0;
}

bool WaveEquationSolver::isValidPosition(double position) const {
    return position >= 0.0 && position <= length_;
}

bool WaveEquationSolver::isStable() const {
    // CFL condition for stability
    double cfl = wave_speed_ * dt_ / dx_;
    return cfl <= 1.0;
}

} // namespace Physics
} // namespace PianoSynth
