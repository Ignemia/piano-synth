#include <gtest/gtest.h>
#include "abstraction/note_event.h"
#include <chrono>
#include <thread>

using namespace PianoSynth::Abstraction;

class NoteEventTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a basic note event
        note_event_ = NoteEvent();
        note_event_.type = NoteEvent::NOTE_ON;
        note_event_.note_number = 60; // C4
        note_event_.velocity = 0.8f;
        note_event_.press_time = std::chrono::high_resolution_clock::now();
    }

    NoteEvent note_event_;
};

// Test note event initialization
TEST_F(NoteEventTest, NoteEventInitialization) {
    NoteEvent event;
    
    // Default values should be reasonable
    EXPECT_EQ(event.type, NoteEvent::NOTE_ON);
    EXPECT_EQ(event.note_number, 60);
    EXPECT_FLOAT_EQ(event.velocity, 0.5f);
    EXPECT_FLOAT_EQ(event.release_velocity, 0.5f);
    EXPECT_DOUBLE_EQ(event.duration_ms, 0.0);
    EXPECT_FLOAT_EQ(event.hammer_velocity, 0.0f);
    EXPECT_FLOAT_EQ(event.string_excitation, 0.0f);
    EXPECT_FLOAT_EQ(event.damper_position, 1.0f);
    EXPECT_FALSE(event.sustain_pedal);
    EXPECT_FALSE(event.soft_pedal);
    EXPECT_FALSE(event.sostenuto_pedal);
    EXPECT_FLOAT_EQ(event.pitch_bend, 0.0f);
    EXPECT_FLOAT_EQ(event.aftertouch, 0.0f);
}

// Test note event types
TEST_F(NoteEventTest, NoteEventTypes) {
    // Test all event types
    std::vector<NoteEvent::Type> types = {
        NoteEvent::NOTE_ON,
        NoteEvent::NOTE_OFF,
        NoteEvent::PEDAL_CHANGE,
        NoteEvent::PITCH_BEND,
        NoteEvent::AFTERTOUCH
    };
    
    for (auto type : types) {
        NoteEvent event;
        event.type = type;
        EXPECT_EQ(event.type, type);
    }
}

// Test timing functionality
TEST_F(NoteEventTest, TimingFunctionality) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Simulate note press
    NoteEvent note_on_event;
    note_on_event.type = NoteEvent::NOTE_ON;
    note_on_event.press_time = std::chrono::high_resolution_clock::now();
    
    // Wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Simulate note release
    NoteEvent note_off_event;
    note_off_event.type = NoteEvent::NOTE_OFF;
    note_off_event.release_time = std::chrono::high_resolution_clock::now();
    
    // Calculate duration
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        note_off_event.release_time - note_on_event.press_time);
    note_off_event.duration_ms = duration.count() / 1000.0;
    
    // Duration should be approximately 100ms
    EXPECT_GT(note_off_event.duration_ms, 90.0);
    EXPECT_LT(note_off_event.duration_ms, 110.0);
    
    // Times should be reasonable
    auto end_time = std::chrono::high_resolution_clock::now();
    EXPECT_GE(note_on_event.press_time, start_time);
    EXPECT_LE(note_on_event.press_time, end_time);
    EXPECT_GE(note_off_event.release_time, note_on_event.press_time);
    EXPECT_LE(note_off_event.release_time, end_time);
}

// Test active note functionality
TEST_F(NoteEventTest, ActiveNoteFunctionality) {
    ActiveNote active_note(note_event_);
    
    // Initial state
    EXPECT_EQ(active_note.initial_event.note_number, 60);
    EXPECT_TRUE(active_note.is_active);
    EXPECT_FLOAT_EQ(active_note.current_amplitude, 0.8f); // Should match velocity
    EXPECT_FLOAT_EQ(active_note.current_frequency, 0.0f);
    EXPECT_FALSE(active_note.pedal_sustained);
    
    // Test state modifications
    active_note.current_amplitude = 0.5f;
    active_note.current_frequency = 261.626f; // C4 frequency
    active_note.pedal_sustained = true;
    
    EXPECT_FLOAT_EQ(active_note.current_amplitude, 0.5f);
    EXPECT_FLOAT_EQ(active_note.current_frequency, 261.626f);
    EXPECT_TRUE(active_note.pedal_sustained);
    
    // Test deactivation
    active_note.is_active = false;
    EXPECT_FALSE(active_note.is_active);
}

// Test physical parameter ranges
TEST_F(NoteEventTest, PhysicalParameterRanges) {
    NoteEvent event;
    
    // Test velocity range
    event.velocity = 0.0f;
    EXPECT_GE(event.velocity, 0.0f);
    EXPECT_LE(event.velocity, 1.0f);
    
    event.velocity = 1.0f;
    EXPECT_GE(event.velocity, 0.0f);
    EXPECT_LE(event.velocity, 1.0f);
    
    // Test pitch bend range
    event.pitch_bend = -1.0f;
    EXPECT_GE(event.pitch_bend, -1.0f);
    EXPECT_LE(event.pitch_bend, 1.0f);
    
    event.pitch_bend = 1.0f;
    EXPECT_GE(event.pitch_bend, -1.0f);
    EXPECT_LE(event.pitch_bend, 1.0f);
    
    // Test damper position range
    event.damper_position = 0.0f; // Fully damped
    EXPECT_GE(event.damper_position, 0.0f);
    EXPECT_LE(event.damper_position, 1.0f);
    
    event.damper_position = 1.0f; // Fully open
    EXPECT_GE(event.damper_position, 0.0f);
    EXPECT_LE(event.damper_position, 1.0f);
}

// Test note number ranges
TEST_F(NoteEventTest, NoteNumberRanges) {
    // Test piano range (MIDI notes 21-108)
    std::vector<int> test_notes = {21, 36, 48, 60, 72, 84, 96, 108};
    
    for (int note : test_notes) {
        NoteEvent event;
        event.note_number = note;
        
        EXPECT_GE(event.note_number, 21);  // Lowest piano key (A0)
        EXPECT_LE(event.note_number, 108); // Highest piano key (C8)
    }
}

// Test pedal state combinations
TEST_F(NoteEventTest, PedalStateCombinations) {
    NoteEvent event;
    
    // Test all combinations of pedal states
    std::vector<bool> pedal_states = {false, true};
    
    for (bool sustain : pedal_states) {
        for (bool soft : pedal_states) {
            for (bool sostenuto : pedal_states) {
                event.sustain_pedal = sustain;
                event.soft_pedal = soft;
                event.sostenuto_pedal = sostenuto;
                
                EXPECT_EQ(event.sustain_pedal, sustain);
                EXPECT_EQ(event.soft_pedal, soft);
                EXPECT_EQ(event.sostenuto_pedal, sostenuto);
            }
        }
    }
}

// Test copy and assignment
TEST_F(NoteEventTest, CopyAndAssignment) {
    // Set up original event with specific values
    note_event_.velocity = 0.75f;
    note_event_.hammer_velocity = 2.5f;
    note_event_.sustain_pedal = true;
    note_event_.pitch_bend = 0.3f;
    
    // Test copy constructor
    NoteEvent copied_event = note_event_;
    
    EXPECT_EQ(copied_event.type, note_event_.type);
    EXPECT_EQ(copied_event.note_number, note_event_.note_number);
    EXPECT_FLOAT_EQ(copied_event.velocity, note_event_.velocity);
    EXPECT_FLOAT_EQ(copied_event.hammer_velocity, note_event_.hammer_velocity);
    EXPECT_EQ(copied_event.sustain_pedal, note_event_.sustain_pedal);
    EXPECT_FLOAT_EQ(copied_event.pitch_bend, note_event_.pitch_bend);
    
    // Test assignment
    NoteEvent assigned_event;
    assigned_event = note_event_;
    
    EXPECT_EQ(assigned_event.type, note_event_.type);
    EXPECT_EQ(assigned_event.note_number, note_event_.note_number);
    EXPECT_FLOAT_EQ(assigned_event.velocity, note_event_.velocity);
    EXPECT_FLOAT_EQ(assigned_event.hammer_velocity, note_event_.hammer_velocity);
    EXPECT_EQ(assigned_event.sustain_pedal, note_event_.sustain_pedal);
    EXPECT_FLOAT_EQ(assigned_event.pitch_bend, note_event_.pitch_bend);
}

// Test event validation
TEST_F(NoteEventTest, EventValidation) {
    NoteEvent event;
    
    // Valid note event
    event.type = NoteEvent::NOTE_ON;
    event.note_number = 60;
    event.velocity = 0.8f;
    
    // All parameters should be in valid ranges
    EXPECT_GE(event.note_number, 0);
    EXPECT_LE(event.note_number, 127);
    EXPECT_GE(event.velocity, 0.0f);
    EXPECT_LE(event.velocity, 1.0f);
    
    // Test edge cases
    event.note_number = 0;
    EXPECT_GE(event.note_number, 0);
    
    event.note_number = 127;
    EXPECT_LE(event.note_number, 127);
    
    event.velocity = 0.0f;
    EXPECT_GE(event.velocity, 0.0f);
    
    event.velocity = 1.0f;
    EXPECT_LE(event.velocity, 1.0f);
}
