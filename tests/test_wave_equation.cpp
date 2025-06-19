#include <gtest/gtest.h>
#include "physics/wave_equation_solver.h"
#include "utils/constants.h"
#include <cmath>

using namespace PianoSynth::Physics;
using namespace PianoSynth::Constants;

class WaveEquationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test parameters for a typical piano string
        length_ = 1.0;           // 1 meter
        wave_speed_ = 400.0;     // 400 m/s
        stiffness_ = 1e-5;       // Small stiffness
        damping_ = 0.001;        // Light damping
        sample_rate_ = 44100.0;  // Standard audio sample rate
        spatial_points_ = 100;   // 100 spatial discretization points
        
        solver_ = std::make_unique<WaveEquationSolver>();
        solver_->initialize(length_, wave_speed_, stiffness_, damping_, sample_rate_, spatial_points_);
    }

    void TearDown() override {
        solver_.reset();
    }

    std::unique_ptr<WaveEquationSolver> solver_;
    double length_;
    double wave_speed_;
    double stiffness_;
    double damping_;
    double sample_rate_;
    int spatial_points_;
    const double EPSILON = 1e-6;
};

// Test wave equation solver initialization
TEST_F(WaveEquationTest, Initialization) {
    // After initialization, displacement should be zero everywhere
    for (int i = 0; i <= spatial_points_; ++i) {
        double position = (i * length_) / spatial_points_;
        EXPECT_NEAR(solver_->getDisplacement(position), 0.0, EPSILON);
        EXPECT_NEAR(solver_->getVelocity(position), 0.0, EPSILON);
    }
}

// Test excitation and wave propagation
TEST_F(WaveEquationTest, ExcitationAndPropagation) {
    // Add excitation at center of string
    double excitation_position = length_ / 2.0;
    double excitation_force = 1.0;
    
    solver_->addExcitation(excitation_position, excitation_force);
    
    // Take one simulation step
    solver_->step();
    
    // There should be non-zero displacement near the excitation point
    double displacement_at_excitation = solver_->getDisplacement(excitation_position);
    EXPECT_GT(std::abs(displacement_at_excitation), EPSILON);
    
    // Run several steps to see wave propagation
    std::vector<double> center_displacements;
    for (int i = 0; i < 100; ++i) {
        solver_->step();
        center_displacements.push_back(solver_->getDisplacement(excitation_position));
    }
    
    // Should have oscillatory behavior
    bool has_positive = false;
    bool has_negative = false;
    for (double disp : center_displacements) {
        if (disp > EPSILON) has_positive = true;
        if (disp < -EPSILON) has_negative = true;
    }
    
    EXPECT_TRUE(has_positive);
    EXPECT_TRUE(has_negative);
}

// Test boundary conditions
TEST_F(WaveEquationTest, BoundaryConditions) {
    // Set fixed boundary conditions (default)
    solver_->setBoundaryConditions(WaveEquationSolver::FIXED, WaveEquationSolver::FIXED);
    
    // Add excitation and run simulation
    solver_->addExcitation(length_ / 2.0, 1.0);
    
    for (int i = 0; i < 50; ++i) {
        solver_->step();
        
        // Fixed boundaries should always have zero displacement
        EXPECT_NEAR(solver_->getDisplacement(0.0), 0.0, EPSILON);
        EXPECT_NEAR(solver_->getDisplacement(length_), 0.0, EPSILON);
    }
}

// Test wave reflection at boundaries
TEST_F(WaveEquationTest, WaveReflection) {
    // Excite near one end to create a traveling wave
    double excitation_position = length_ * 0.1;
    solver_->addExcitation(excitation_position, 1.0);
    
    // Record displacement at quarter point over time
    double quarter_point = length_ * 0.25;
    std::vector<double> displacements;
    
    for (int i = 0; i < 1000; ++i) {
        solver_->step();
        displacements.push_back(solver_->getDisplacement(quarter_point));
    }
    
    // Should see wave activity (non-zero displacements)
    double max_displacement = 0.0;
    for (double disp : displacements) {
        max_displacement = std::max(max_displacement, std::abs(disp));
    }
    
    EXPECT_GT(max_displacement, EPSILON);
}

// Test frequency response
TEST_F(WaveEquationTest, FrequencyResponse) {
    // The fundamental frequency should be approximately wave_speed / (2 * length)
    double expected_frequency = wave_speed_ / (2.0 * length_);
    
    // Excite string with impulse
    solver_->addExcitation(length_ / 8.0, 1.0); // Excite at 1/8 point for good harmonic content
    
    // Collect samples
    const int num_samples = 4096;
    std::vector<double> samples;
    
    for (int i = 0; i < num_samples; ++i) {
        solver_->step();
        samples.push_back(solver_->getDisplacement(length_ / 4.0)); // Sample at 1/4 point
    }
    
    // Simple frequency estimation using zero crossings
    int zero_crossings = 0;
    for (size_t i = 1; i < samples.size(); ++i) {
        if ((samples[i-1] >= 0 && samples[i] < 0) || 
            (samples[i-1] < 0 && samples[i] >= 0)) {
            zero_crossings++;
        }
    }
    
    double estimated_frequency = (zero_crossings / 2.0) * sample_rate_ / num_samples;
    
    // Should be within 10% of expected frequency
    EXPECT_NEAR(estimated_frequency, expected_frequency, expected_frequency * 0.1);
}

// Test damping effects
TEST_F(WaveEquationTest, DampingEffects) {
    // Test with high damping
    auto high_damping_solver = std::make_unique<WaveEquationSolver>();
    high_damping_solver->initialize(length_, wave_speed_, stiffness_, 0.1, sample_rate_, spatial_points_);
    
    // Test with low damping
    auto low_damping_solver = std::make_unique<WaveEquationSolver>();
    low_damping_solver->initialize(length_, wave_speed_, stiffness_, 0.001, sample_rate_, spatial_points_);
    
    // Excite both
    high_damping_solver->addExcitation(length_ / 2.0, 1.0);
    low_damping_solver->addExcitation(length_ / 2.0, 1.0);
    
    // Run for many steps and measure decay
    double high_damping_amplitude = 0.0;
    double low_damping_amplitude = 0.0;
    
    for (int i = 0; i < 2000; ++i) {
        high_damping_solver->step();
        low_damping_solver->step();
        
        if (i > 1500) { // Measure amplitude in later part
            high_damping_amplitude = std::max(high_damping_amplitude, 
                std::abs(high_damping_solver->getDisplacement(length_ / 2.0)));
            low_damping_amplitude = std::max(low_damping_amplitude, 
                std::abs(low_damping_solver->getDisplacement(length_ / 2.0)));
        }
    }
    
    // Low damping should have higher sustained amplitude
    EXPECT_GT(low_damping_amplitude, high_damping_amplitude);
}

// Test stiffness effects
TEST_F(WaveEquationTest, StiffnessEffects) {
    // Stiffness should affect higher harmonics more than fundamental
    // This is a complex test, so we'll just verify that stiffness changes behavior
    
    auto no_stiffness_solver = std::make_unique<WaveEquationSolver>();
    no_stiffness_solver->initialize(length_, wave_speed_, 0.0, damping_, sample_rate_, spatial_points_);
    
    auto with_stiffness_solver = std::make_unique<WaveEquationSolver>();
    with_stiffness_solver->initialize(length_, wave_speed_, 1e-4, damping_, sample_rate_, spatial_points_);
    
    // Excite both at same position
    double excitation_pos = length_ / 8.0;
    no_stiffness_solver->addExcitation(excitation_pos, 1.0);
    with_stiffness_solver->addExcitation(excitation_pos, 1.0);
    
    // Collect samples from both
    std::vector<double> no_stiffness_samples;
    std::vector<double> with_stiffness_samples;
    
    for (int i = 0; i < 1000; ++i) {
        no_stiffness_solver->step();
        with_stiffness_solver->step();
        
        no_stiffness_samples.push_back(no_stiffness_solver->getDisplacement(length_ / 4.0));
        with_stiffness_samples.push_back(with_stiffness_solver->getDisplacement(length_ / 4.0));
    }
    
    // The samples should be different (stiffness changes the behavior)
    double difference = 0.0;
    for (size_t i = 0; i < no_stiffness_samples.size(); ++i) {
        difference += std::abs(no_stiffness_samples[i] - with_stiffness_samples[i]);
    }
    
    EXPECT_GT(difference, EPSILON);
}

// Test reset functionality
TEST_F(WaveEquationTest, ResetFunctionality) {
    // Excite and run simulation
    solver_->addExcitation(length_ / 2.0, 1.0);
    
    for (int i = 0; i < 100; ++i) {
        solver_->step();
    }
    
    // Should have non-zero displacement somewhere
    bool has_displacement = false;
    for (int i = 0; i <= spatial_points_; ++i) {
        double position = (i * length_) / spatial_points_;
        if (std::abs(solver_->getDisplacement(position)) > EPSILON) {
            has_displacement = true;
            break;
        }
    }
    EXPECT_TRUE(has_displacement);
    
    // Reset
    solver_->reset();
    
    // All displacements should be zero again
    for (int i = 0; i <= spatial_points_; ++i) {
        double position = (i * length_) / spatial_points_;
        EXPECT_NEAR(solver_->getDisplacement(position), 0.0, EPSILON);
        EXPECT_NEAR(solver_->getVelocity(position), 0.0, EPSILON);
    }
}

// Test distributed excitation
TEST_F(WaveEquationTest, DistributedExcitation) {
    // Add distributed excitation over a region
    double start_pos = length_ * 0.3;
    double end_pos = length_ * 0.7;
    double force = 1.0;
    
    solver_->addDistributedExcitation(start_pos, end_pos, force);
    solver_->step();
    
    // Should have excitation in the specified region
    bool has_excitation_in_region = false;
    for (double pos = start_pos; pos <= end_pos; pos += length_ / 50.0) {
        if (std::abs(solver_->getDisplacement(pos)) > EPSILON) {
            has_excitation_in_region = true;
            break;
        }
    }
    
    EXPECT_TRUE(has_excitation_in_region);
}
