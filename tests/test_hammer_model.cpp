#include <gtest/gtest.h>
#include "physics/hammer_model.h"
#include "utils/constants.h"
#include <cmath>
#include <algorithm>

using namespace PianoSynth::Physics;
using namespace PianoSynth::Constants;

class HammerModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test with middle C (MIDI 60)
        hammer_model_ = std::make_unique<HammerModel>(60);
        hammer_model_->initialize(SAMPLE_RATE);
    }

    void TearDown() override {
        hammer_model_.reset();
    }

    std::unique_ptr<HammerModel> hammer_model_;
    const double EPSILON = 1e-6;
};

// Test hammer model initialization
TEST_F(HammerModelTest, Initialization) {
    EXPECT_FALSE(hammer_model_->isInContact());
    EXPECT_NEAR(hammer_model_->getPosition(), 0.0, EPSILON);
    EXPECT_NEAR(hammer_model_->getVelocity(), 0.0, EPSILON);
    EXPECT_NEAR(hammer_model_->getContactForce(), 0.0, EPSILON);
}

// Test hammer strike
TEST_F(HammerModelTest, HammerStrike) {
    double strike_velocity = 2.0; // m/s
    double strike_position = 0.125; // 1/8 from string end
    
    hammer_model_->strike(strike_velocity, strike_position);
    
    // After strike, hammer should have velocity
    EXPECT_NEAR(hammer_model_->getVelocity(), strike_velocity, EPSILON);
    EXPECT_FALSE(hammer_model_->isInContact()); // Not in contact initially
}

// Test hammer-string contact
TEST_F(HammerModelTest, HammerStringContact) {
    double strike_velocity = 1.0;
    hammer_model_->strike(strike_velocity);
    
    // Simulate string at rest (displacement = 0)
    double string_displacement = 0.0;
    
    // Step simulation until contact
    double max_force = 0.0;
    bool made_contact = false;
    
    for (int i = 0; i < 1000; ++i) {
        double force = hammer_model_->step(string_displacement);
        
        if (hammer_model_->isInContact()) {
            made_contact = true;
            max_force = std::max(max_force, force);
        }
        
        // If velocity becomes negative, hammer is moving away
        if (hammer_model_->getVelocity() < 0.0) {
            break;
        }
    }
    
    EXPECT_TRUE(made_contact);
    EXPECT_GT(max_force, 0.0);
}

// Test velocity scaling
TEST_F(HammerModelTest, VelocityScaling) {
    std::vector<double> test_velocities = {0.5, 1.0, 2.0, 3.0};
    std::vector<double> max_forces;
    
    for (double velocity : test_velocities) {
        hammer_model_->reset();
        hammer_model_->strike(velocity);
        
        double max_force = 0.0;
        
        // Simulate contact with string at rest
        for (int i = 0; i < 500; ++i) {
            double force = hammer_model_->step(0.0);
            max_force = std::max(max_force, force);
            
            if (hammer_model_->getVelocity() < 0.0) {
                break;
            }
        }
        
        max_forces.push_back(max_force);
    }
    
    // Higher velocities should produce higher forces
    for (size_t i = 1; i < max_forces.size(); ++i) {
        EXPECT_GT(max_forces[i], max_forces[i-1]) 
            << "Force didn't increase with velocity";
    }
}

// Test felt hardness effects
TEST_F(HammerModelTest, FeltHardnessEffects) {
    std::vector<double> hardness_values = {0.2, 0.5, 0.8};
    std::vector<double> max_forces;
    
    double strike_velocity = 2.0;
    
    for (double hardness : hardness_values) {
        hammer_model_->reset();
        hammer_model_->setFeltHardness(hardness);
        hammer_model_->strike(strike_velocity);
        
        double max_force = 0.0;
        
        for (int i = 0; i < 500; ++i) {
            double force = hammer_model_->step(0.0);
            max_force = std::max(max_force, force);
            
            if (hammer_model_->getVelocity() < 0.0) {
                break;
            }
        }
        
        max_forces.push_back(max_force);
    }
    
    // Harder felt should produce higher forces
    for (size_t i = 1; i < max_forces.size(); ++i) {
        EXPECT_GT(max_forces[i], max_forces[i-1]) 
            << "Force didn't increase with felt hardness";
    }
}

// Test contact duration
TEST_F(HammerModelTest, ContactDuration) {
    double strike_velocity = 1.5;
    hammer_model_->strike(strike_velocity);
    
    int contact_steps = 0;
    bool was_in_contact = false;
    
    for (int i = 0; i < 1000; ++i) {
        hammer_model_->step(0.0);
        
        if (hammer_model_->isInContact()) {
            contact_steps++;
            was_in_contact = true;
        } else if (was_in_contact) {
            // Contact ended
            break;
        }
    }
    
    EXPECT_TRUE(was_in_contact);
    EXPECT_GT(contact_steps, 0);
    
    // Contact duration should be reasonable (a few milliseconds)
    double contact_duration = contact_steps / SAMPLE_RATE;
    EXPECT_GT(contact_duration, 0.0001); // > 0.1ms
    EXPECT_LT(contact_duration, 0.01);   // < 10ms
}

// Test different note ranges
TEST_F(HammerModelTest, NoteRangeEffects) {
    std::vector<int> test_notes = {21, 36, 48, 60, 72, 84, 96, 108};
    std::vector<double> max_forces;
    
    double strike_velocity = 2.0;
    
    for (int note : test_notes) {
        HammerModel hammer(note);
        hammer.initialize(SAMPLE_RATE);
        hammer.strike(strike_velocity);
        
        double max_force = 0.0;
        
        for (int i = 0; i < 500; ++i) {
            double force = hammer.step(0.0);
            max_force = std::max(max_force, force);
            
            if (hammer.getVelocity() < 0.0) {
                break;
            }
        }
        
        max_forces.push_back(max_force);
        EXPECT_GT(max_force, 0.0) << "Note " << note << " produced no force";
    }
    
    // Should have variation across note range
    double min_force = *std::min_element(max_forces.begin(), max_forces.end());
    double max_force = *std::max_element(max_forces.begin(), max_forces.end());
    EXPECT_GT(max_force / min_force, 1.1); // At least 10% variation
}

// Test mass and stiffness effects
TEST_F(HammerModelTest, MassAndStiffnessEffects) {
    // Test mass effects
    hammer_model_->setMass(0.005); // Light hammer
    hammer_model_->strike(2.0);
    
    double light_hammer_max_force = 0.0;
    for (int i = 0; i < 500; ++i) {
        double force = hammer_model_->step(0.0);
        light_hammer_max_force = std::max(light_hammer_max_force, force);
        if (hammer_model_->getVelocity() < 0.0) break;
    }
    
    hammer_model_->reset();
    hammer_model_->setMass(0.020); // Heavy hammer
    hammer_model_->strike(2.0);
    
    double heavy_hammer_max_force = 0.0;
    for (int i = 0; i < 500; ++i) {
        double force = hammer_model_->step(0.0);
        heavy_hammer_max_force = std::max(heavy_hammer_max_force, force);
        if (hammer_model_->getVelocity() < 0.0) break;
    }
    
    // Heavy hammer should produce different force characteristics
    EXPECT_NE(light_hammer_max_force, heavy_hammer_max_force);
}

// Test reset functionality
TEST_F(HammerModelTest, ResetFunctionality) {
    // Strike hammer and run simulation
    hammer_model_->strike(2.0);
    
    for (int i = 0; i < 100; ++i) {
        hammer_model_->step(0.0);
    }
    
    // State should be non-zero
    EXPECT_NE(hammer_model_->getPosition(), 0.0);
    
    // Reset
    hammer_model_->reset();
    
    // State should be zero after reset
    EXPECT_NEAR(hammer_model_->getPosition(), 0.0, EPSILON);
    EXPECT_NEAR(hammer_model_->getVelocity(), 0.0, EPSILON);
    EXPECT_NEAR(hammer_model_->getContactForce(), 0.0, EPSILON);
    EXPECT_FALSE(hammer_model_->isInContact());
}

// Test parameter bounds
TEST_F(HammerModelTest, ParameterBounds) {
    // Test mass bounds
    hammer_model_->setMass(-1.0); // Negative mass
    EXPECT_GT(hammer_model_->getVelocity(), -1000.0); // Should not explode
    
    hammer_model_->setMass(0.0); // Zero mass
    EXPECT_GT(hammer_model_->getVelocity(), -1000.0); // Should not explode
    
    // Test felt hardness bounds
    hammer_model_->setFeltHardness(-1.0); // Should clamp to valid range
    hammer_model_->setFeltHardness(2.0);  // Should clamp to valid range
    
    // Should still function normally
    hammer_model_->strike(1.0);
    double force = hammer_model_->step(0.0);
    EXPECT_GE(force, 0.0);
}
