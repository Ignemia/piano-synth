#include <gtest/gtest.h>
#include "abstraction/input_abstractor.h"
#include "physics/string_model.h"
#include "physics/hammer_model.h"
#include "utils/constants.h"
#include <vector>
#include <chrono>
#include <thread>

using namespace PianoSynth;
using namespace PianoSynth::Abstraction;
using namespace PianoSynth::Physics;

class SynthesisIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        abstractor_ = std::make_unique<InputAbstractor>();
        abstractor_->initialize();
    }

    void TearDown() override {
        abstractor_.reset();
    }

    // Helper to create a complete note playing scenario
    void simulateNotePress(int note, int velocity, 
                          std::unique_ptr<StringModel>& string,
                          std::unique_ptr<HammerModel>& hammer) {
        
        // Create and process MIDI event
        Input::RawMidiEvent midi_event;
        midi_event.data = {
            static_cast<unsigned char>(Constants::MIDI_NOTE_ON),
            static_cast<unsigned char>(note),
            static_cast<unsigned char>(velocity)
        };
        midi_event.timestamp = 0.0;
        midi_event.source_port = 0;
        
        std::vector<Input::RawMidiEvent> raw_events = {midi_event};
        auto note_events = abstractor_->processEvents(raw_events);
        
        ASSERT_EQ(note_events.size(), 1);
        const auto& note_event = note_events[0];
        
        // Initialize physics models
        string = std::make_unique<StringModel>(note);
        hammer = std::make_unique<HammerModel>(note);
        
        string->initialize(Constants::SAMPLE_RATE);
        hammer->initialize(Constants::SAMPLE_RATE);
        
        // Strike the hammer based on abstracted parameters
        hammer->strike(note_event.hammer_velocity);
    }

    std::unique_ptr<InputAbstractor> abstractor_;
    const double EPSILON = 1e-6;
};

// Test complete note synthesis pipeline
TEST_F(SynthesisIntegrationTest, CompleteNoteSynthesis) {
    std::unique_ptr<StringModel> string;
    std::unique_ptr<HammerModel> hammer;
    
    // Simulate playing middle C with moderate velocity
    simulateNotePress(60, 80, string, hammer);
    
    // Run coupled simulation
    std::vector<double> audio_output;
    const int simulation_steps = 2000;
    
    for (int i = 0; i < simulation_steps; ++i) {
        // Get string displacement
        double string_displacement = string->step();
        
        // Calculate hammer force based on string displacement
        double hammer_force = hammer->step(string_displacement);
        
        // Apply hammer force to string (if in contact)
        if (hammer->isInContact()) {
            // Simplified excitation - in reality this would be more sophisticated
            string->excite(0.125, hammer_force * 0.0001, 1.0 / Constants::SAMPLE_RATE);
        }
        
        audio_output.push_back(string_displacement);
    }
    
    // Verify audio output characteristics
    double max_amplitude = 0.0;
    for (double sample : audio_output) {
        max_amplitude = std::max(max_amplitude, std::abs(sample));
    }
    
    EXPECT_GT(max_amplitude, 0.0); // Should produce audible output
    
    // Check for oscillatory behavior
    int zero_crossings = 0;
    for (size_t i = 1; i < audio_output.size(); ++i) {
        if ((audio_output[i-1] >= 0 && audio_output[i] < 0) ||
            (audio_output[i-1] < 0 && audio_output[i] >= 0)) {
            zero_crossings++;
        }
    }
    
    EXPECT_GT(zero_crossings, 10); // Should have oscillatory content
}

// Test velocity sensitivity across the synthesis chain
TEST_F(SynthesisIntegrationTest, VelocitySensitivity) {
    std::vector<int> test_velocities = {32, 64, 96, 127};
    std::vector<double> max_amplitudes;
    
    for (int velocity : test_velocities) {
        std::unique_ptr<StringModel> string;
        std::unique_ptr<HammerModel> hammer;
        
        simulateNotePress(60, velocity, string, hammer);
        
        double max_amplitude = 0.0;
        
        // Run simulation for enough time to capture the attack
        for (int i = 0; i < 1000; ++i) {
            double string_displacement = string->step();
            double hammer_force = hammer->step(string_displacement);
            
            if (hammer->isInContact()) {
                string->excite(0.125, hammer_force * 0.0001, 1.0 / Constants::SAMPLE_RATE);
            }
            
            max_amplitude = std::max(max_amplitude, std::abs(string_displacement));
        }
        
        max_amplitudes.push_back(max_amplitude);
    }
    
    // Higher velocities should produce higher amplitudes
    for (size_t i = 1; i < max_amplitudes.size(); ++i) {
        EXPECT_GT(max_amplitudes[i], max_amplitudes[i-1])
            << "Amplitude didn't increase with velocity";
    }
}

// Test different notes produce different frequencies
TEST_F(SynthesisIntegrationTest, FrequencyAccuracy) {
    std::vector<int> test_notes = {48, 60, 72}; // C3, C4, C5
    std::vector<double> estimated_frequencies;
    
    for (int note : test_notes) {
        std::unique_ptr<StringModel> string;
        std::unique_ptr<HammerModel> hammer;
        
        simulateNotePress(note, 80, string, hammer);
        
        // Collect audio samples
        std::vector<double> samples;
        const int num_samples = 4096;
        
        // Initial excitation
        for (int i = 0; i < 200; ++i) {
            double string_displacement = string->step();
            double hammer_force = hammer->step(string_displacement);
            
            if (hammer->isInContact()) {
                string->excite(0.125, hammer_force * 0.0001, 1.0 / Constants::SAMPLE_RATE);
            }
        }
        
        // Collect samples after initial transient
        for (int i = 0; i < num_samples; ++i) {
            samples.push_back(string->step());
        }
        
        // Estimate frequency using zero crossings
        int zero_crossings = 0;
        for (size_t i = 1; i < samples.size(); ++i) {
            if ((samples[i-1] >= 0 && samples[i] < 0) ||
                (samples[i-1] < 0 && samples[i] >= 0)) {
                zero_crossings++;
            }
        }
        
        double estimated_freq = (zero_crossings / 2.0) * Constants::SAMPLE_RATE / num_samples;
        estimated_frequencies.push_back(estimated_freq);
    }
    
    // Check that frequencies are in the right order (higher notes = higher frequencies)
    for (size_t i = 1; i < estimated_frequencies.size(); ++i) {
        EXPECT_GT(estimated_frequencies[i], estimated_frequencies[i-1])
            << "Frequency didn't increase with note number";
    }
    
    // Check approximate frequency values
    double expected_c4_freq = 261.626; // Hz
    double tolerance = 50.0; // Allow 50 Hz tolerance due to simple analysis
    
    EXPECT_NEAR(estimated_frequencies[1], expected_c4_freq, tolerance);
}

// Test chord synthesis (multiple simultaneous notes)
TEST_F(SynthesisIntegrationTest, ChordSynthesis) {
    // Create C major chord (C, E, G)
    std::vector<int> chord_notes = {60, 64, 67};
    std::vector<std::unique_ptr<StringModel>> strings;
    std::vector<std::unique_ptr<HammerModel>> hammers;
    
    // Initialize all note models
    for (size_t i = 0; i < chord_notes.size(); ++i) {
        std::unique_ptr<StringModel> string;
        std::unique_ptr<HammerModel> hammer;
        
        simulateNotePress(chord_notes[i], 80, string, hammer);
        
        strings.push_back(std::move(string));
        hammers.push_back(std::move(hammer));
    }
    
    // Run simultaneous synthesis
    std::vector<double> mixed_output;
    const int simulation_steps = 1500;
    
    for (int step = 0; step < simulation_steps; ++step) {
        double mixed_sample = 0.0;
        
        // Process each note
        for (size_t i = 0; i < strings.size(); ++i) {
            double string_displacement = strings[i]->step();
            double hammer_force = hammers[i]->step(string_displacement);
            
            if (hammers[i]->isInContact()) {
                strings[i]->excite(0.125, hammer_force * 0.0001, 1.0 / Constants::SAMPLE_RATE);
            }
            
            mixed_sample += string_displacement;
        }
        
        mixed_output.push_back(mixed_sample);
    }
    
    // Verify mixed output has content
    double max_amplitude = 0.0;
    for (double sample : mixed_output) {
        max_amplitude = std::max(max_amplitude, std::abs(sample));
    }
    
    EXPECT_GT(max_amplitude, 0.0);
    
    // Mixed output should be larger than individual notes due to superposition
    // (though this depends on phase relationships)
}

// Test sustain pedal effects in synthesis
TEST_F(SynthesisIntegrationTest, SustainPedalEffects) {
    std::unique_ptr<StringModel> string_no_sustain;
    std::unique_ptr<HammerModel> hammer_no_sustain;
    
    std::unique_ptr<StringModel> string_with_sustain;
    std::unique_ptr<HammerModel> hammer_with_sustain;
    
    // First, press sustain pedal
    Input::RawMidiEvent sustain_event;
    sustain_event.data = {
        static_cast<unsigned char>(Constants::MIDI_CONTROL_CHANGE),
        static_cast<unsigned char>(Constants::MIDI_SUSTAIN_PEDAL),
        127 // Pressed
    };
    std::vector<Input::RawMidiEvent> pedal_events = {sustain_event};
    abstractor_->processEvents(pedal_events);
    
    // Play note with sustain
    simulateNotePress(60, 80, string_with_sustain, hammer_with_sustain);
    
    // Reset abstractor and play note without sustain
    abstractor_->clearActiveNotes();
    simulateNotePress(60, 80, string_no_sustain, hammer_no_sustain);
    
    // Simulate both scenarios
    const int simulation_steps = 3000;
    double sustain_amplitude_late = 0.0;
    double no_sustain_amplitude_late = 0.0;
    
    for (int step = 0; step < simulation_steps; ++step) {
        // With sustain
        double sustain_displacement = string_with_sustain->step();
        double sustain_hammer_force = hammer_with_sustain->step(sustain_displacement);
        if (hammer_with_sustain->isInContact()) {
            string_with_sustain->excite(0.125, sustain_hammer_force * 0.0001, 1.0 / Constants::SAMPLE_RATE);
        }
        
        // Without sustain (apply damping earlier)
        string_no_sustain->setDamperPosition(0.3); // Partial damping
        double no_sustain_displacement = string_no_sustain->step();
        double no_sustain_hammer_force = hammer_no_sustain->step(no_sustain_displacement);
        if (hammer_no_sustain->isInContact()) {
            string_no_sustain->excite(0.125, no_sustain_hammer_force * 0.0001, 1.0 / Constants::SAMPLE_RATE);
        }
        
        // Measure amplitude in later part of simulation
        if (step > 2000) {
            sustain_amplitude_late = std::max(sustain_amplitude_late, std::abs(sustain_displacement));
            no_sustain_amplitude_late = std::max(no_sustain_amplitude_late, std::abs(no_sustain_displacement));
        }
    }
    
    // Sustain should maintain higher amplitude longer
    EXPECT_GT(sustain_amplitude_late, no_sustain_amplitude_late);
}

// Test abstraction layer parameter mapping
TEST_F(SynthesisIntegrationTest, AbstractionParameterMapping) {
    // Test that abstraction layer parameters affect synthesis appropriately
    
    // Set different velocity curves
    abstractor_->setVelocityCurve(0.5); // Soft curve
    
    std::unique_ptr<StringModel> string_soft;
    std::unique_ptr<HammerModel> hammer_soft;
    simulateNotePress(60, 100, string_soft, hammer_soft);
    
    abstractor_->setVelocityCurve(2.0); // Hard curve
    
    std::unique_ptr<StringModel> string_hard;
    std::unique_ptr<HammerModel> hammer_hard;
    simulateNotePress(60, 100, string_hard, hammer_hard);
    
    // Run both simulations
    double soft_max_force = 0.0;
    double hard_max_force = 0.0;
    
    for (int i = 0; i < 500; ++i) {
        // Soft curve
        double soft_displacement = string_soft->step();
        double soft_force = hammer_soft->step(soft_displacement);
        soft_max_force = std::max(soft_max_force, soft_force);
        
        // Hard curve
        double hard_displacement = string_hard->step();
        double hard_force = hammer_hard->step(hard_displacement);
        hard_max_force = std::max(hard_max_force, hard_force);
    }
    
    // Hard velocity curve should produce different force characteristics
    EXPECT_NE(soft_max_force, hard_max_force);
}
