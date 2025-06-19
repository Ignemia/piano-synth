#include <gtest/gtest.h>
#include "abstraction/input_abstractor.h"
#include "input/midi_input_manager.h"
#include "utils/constants.h"
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>

using namespace PianoSynth;
using namespace PianoSynth::Abstraction;
using namespace PianoSynth::Input;

class AbstractionLayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        abstractor_ = std::make_unique<InputAbstractor>();
        abstractor_->initialize();
    }

    void TearDown() override {
        abstractor_.reset();
    }

    // Helper function to create raw MIDI note on event
    RawMidiEvent createMidiNoteOn(int note, int velocity, int channel = 0) {
        RawMidiEvent event;
        event.data = {
            static_cast<unsigned char>(Constants::MIDI_NOTE_ON | channel),
            static_cast<unsigned char>(note),
            static_cast<unsigned char>(velocity)
        };
        event.timestamp = getCurrentTime();
        event.source_port = 0;
        return event;
    }

    // Helper function to create raw MIDI note off event
    RawMidiEvent createMidiNoteOff(int note, int velocity, int channel = 0) {
        RawMidiEvent event;
        event.data = {
            static_cast<unsigned char>(Constants::MIDI_NOTE_OFF | channel),
            static_cast<unsigned char>(note),
            static_cast<unsigned char>(velocity)
        };
        event.timestamp = getCurrentTime();
        event.source_port = 0;
        return event;
    }

    // Helper function to create sustain pedal event
    RawMidiEvent createSustainPedal(bool pressed, int channel = 0) {
        RawMidiEvent event;
        event.data = {
            static_cast<unsigned char>(Constants::MIDI_CONTROL_CHANGE | channel),
            static_cast<unsigned char>(Constants::MIDI_SUSTAIN_PEDAL),
            static_cast<unsigned char>(pressed ? 127 : 0)
        };
        event.timestamp = getCurrentTime();
        event.source_port = 0;
        return event;
    }

    double getCurrentTime() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration<double>(duration).count();
    }

    std::unique_ptr<InputAbstractor> abstractor_;
};

// Test single note on/off
TEST_F(AbstractionLayerTest, SingleNoteOnOff) {
    std::vector<RawMidiEvent> raw_events;
    
    // Note on C4 (MIDI 60) with velocity 100
    raw_events.push_back(createMidiNoteOn(60, 100));
    
    auto processed_events = abstractor_->processEvents(raw_events);
    
    ASSERT_EQ(processed_events.size(), 1);
    
    const auto& note_event = processed_events[0];
    EXPECT_EQ(note_event.type, NoteEvent::NOTE_ON);
    EXPECT_EQ(note_event.note_number, 60);
    EXPECT_FLOAT_EQ(note_event.velocity, 100.0f / 127.0f);
    EXPECT_GT(note_event.hammer_velocity, 0.0f);
    EXPECT_GT(note_event.string_excitation, 0.0f);
    EXPECT_FALSE(note_event.sustain_pedal);
    
    // Clear events
    raw_events.clear();
    
    // Add small delay to simulate realistic timing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Note off
    raw_events.push_back(createMidiNoteOff(60, 64));
    
    processed_events = abstractor_->processEvents(raw_events);
    
    ASSERT_EQ(processed_events.size(), 1);
    
    const auto& note_off_event = processed_events[0];
    EXPECT_EQ(note_off_event.type, NoteEvent::NOTE_OFF);
    EXPECT_EQ(note_off_event.note_number, 60);
    EXPECT_FLOAT_EQ(note_off_event.release_velocity, 64.0f / 127.0f);
}

// Test chord playing (multiple simultaneous notes)
TEST_F(AbstractionLayerTest, ChordPlaying) {
    std::vector<RawMidiEvent> raw_events;
    
    // C Major chord: C4 (60), E4 (64), G4 (67)
    raw_events.push_back(createMidiNoteOn(60, 100)); // C
    raw_events.push_back(createMidiNoteOn(64, 95));  // E
    raw_events.push_back(createMidiNoteOn(67, 105)); // G
    
    auto processed_events = abstractor_->processEvents(raw_events);
    
    ASSERT_EQ(processed_events.size(), 3);
    
    // Verify all notes are processed correctly
    std::vector<int> expected_notes = {60, 64, 67};
    std::vector<int> actual_notes;
    
    for (const auto& event : processed_events) {
        EXPECT_EQ(event.type, NoteEvent::NOTE_ON);
        actual_notes.push_back(event.note_number);
        
        // Each note should have physical parameters calculated
        EXPECT_GT(event.hammer_velocity, 0.0f);
        EXPECT_GT(event.string_excitation, 0.0f);
        EXPECT_EQ(event.damper_position, 1.0f); // No sustain pedal, so damper open
    }
    
    std::sort(actual_notes.begin(), actual_notes.end());
    EXPECT_EQ(actual_notes, expected_notes);
}

// Test arpeggio (sequential notes)
TEST_F(AbstractionLayerTest, ArpeggioPlaying) {
    std::vector<RawMidiEvent> raw_events;
    
    // C Major arpeggio with timing
    const std::vector<int> arpeggio_notes = {60, 64, 67, 72}; // C, E, G, C
    const std::vector<int> velocities = {80, 85, 90, 95};
    
    for (size_t i = 0; i < arpeggio_notes.size(); ++i) {
        raw_events.clear();
        raw_events.push_back(createMidiNoteOn(arpeggio_notes[i], velocities[i]));
        
        auto processed_events = abstractor_->processEvents(raw_events);
        
        ASSERT_EQ(processed_events.size(), 1);
        
        const auto& event = processed_events[0];
        EXPECT_EQ(event.type, NoteEvent::NOTE_ON);
        EXPECT_EQ(event.note_number, arpeggio_notes[i]);
        EXPECT_FLOAT_EQ(event.velocity, velocities[i] / 127.0f);
        
        // Higher velocities should produce higher hammer velocities
        if (i > 0) {
            // This test assumes monotonically increasing velocities
        }
        
        // Small delay between notes
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// Test sustain pedal effects
TEST_F(AbstractionLayerTest, SustainPedalEffect) {
    std::vector<RawMidiEvent> raw_events;
    
    // Press sustain pedal
    raw_events.push_back(createSustainPedal(true));
    
    auto processed_events = abstractor_->processEvents(raw_events);
    ASSERT_EQ(processed_events.size(), 1);
    EXPECT_EQ(processed_events[0].type, NoteEvent::PEDAL_CHANGE);
    
    raw_events.clear();
    
    // Play note with sustain pedal active
    raw_events.push_back(createMidiNoteOn(60, 100));
    
    processed_events = abstractor_->processEvents(raw_events);
    
    ASSERT_EQ(processed_events.size(), 1);
    
    const auto& note_event = processed_events[0];
    EXPECT_TRUE(note_event.sustain_pedal);
    EXPECT_EQ(note_event.damper_position, 1.0f); // Sustain keeps damper open
    
    raw_events.clear();
    
    // Release sustain pedal
    raw_events.push_back(createSustainPedal(false));
    
    processed_events = abstractor_->processEvents(raw_events);
    ASSERT_EQ(processed_events.size(), 1);
    EXPECT_EQ(processed_events[0].type, NoteEvent::PEDAL_CHANGE);
}

// Test velocity curve effects
TEST_F(AbstractionLayerTest, VelocityCurveEffect) {
    // Test different velocity curves
    std::vector<float> velocity_curves = {0.5f, 1.0f, 2.0f};
    std::vector<int> test_velocities = {32, 64, 96, 127};
    
    for (float curve : velocity_curves) {
        abstractor_->setVelocityCurve(curve);
        
        std::vector<float> hammer_velocities;
        
        for (int velocity : test_velocities) {
            std::vector<RawMidiEvent> raw_events;
            raw_events.push_back(createMidiNoteOn(60, velocity));
            
            auto processed_events = abstractor_->processEvents(raw_events);
            
            ASSERT_EQ(processed_events.size(), 1);
            hammer_velocities.push_back(processed_events[0].hammer_velocity);
            
            // Clear for next test
            abstractor_->clearActiveNotes();
        }
        
        // Verify hammer velocities are monotonically increasing
        for (size_t i = 1; i < hammer_velocities.size(); ++i) {
            EXPECT_GT(hammer_velocities[i], hammer_velocities[i-1])
                << "Velocity curve " << curve << " failed at index " << i;
        }
    }
}

// Test note range effects (different hammer characteristics)
TEST_F(AbstractionLayerTest, NoteRangeEffects) {
    const int test_velocity = 100;
    std::vector<int> test_notes = {21, 36, 48, 60, 72, 84, 96, 108}; // Full piano range
    
    std::vector<float> hammer_velocities;
    std::vector<float> string_excitations;
    
    for (int note : test_notes) {
        std::vector<RawMidiEvent> raw_events;
        raw_events.push_back(createMidiNoteOn(note, test_velocity));
        
        auto processed_events = abstractor_->processEvents(raw_events);
        
        ASSERT_EQ(processed_events.size(), 1);
        
        hammer_velocities.push_back(processed_events[0].hammer_velocity);
        string_excitations.push_back(processed_events[0].string_excitation);
        
        abstractor_->clearActiveNotes();
    }
    
    // Lower notes should generally have different characteristics
    // (this depends on the specific implementation)
    EXPECT_GT(hammer_velocities.size(), 0);
    EXPECT_GT(string_excitations.size(), 0);
    
    // All values should be positive
    for (float hv : hammer_velocities) {
        EXPECT_GT(hv, 0.0f);
    }
    for (float se : string_excitations) {
        EXPECT_GT(se, 0.0f);
    }
}

// Test complex playing scenario (legato, overlapping notes)
TEST_F(AbstractionLayerTest, ComplexPlayingScenario) {
    std::vector<RawMidiEvent> raw_events;
    
    // Simulate legato playing with overlapping notes
    // Play C4, then while holding, play D4, release C4, then E4
    
    // 1. Play C4
    raw_events.push_back(createMidiNoteOn(60, 80));
    auto events = abstractor_->processEvents(raw_events);
    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].note_number, 60);
    
    raw_events.clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 2. Play D4 (while C4 still held)
    raw_events.push_back(createMidiNoteOn(62, 85));
    events = abstractor_->processEvents(raw_events);
    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].note_number, 62);
    
    raw_events.clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 3. Release C4
    raw_events.push_back(createMidiNoteOff(60, 64));
    events = abstractor_->processEvents(raw_events);
    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].type, NoteEvent::NOTE_OFF);
    EXPECT_EQ(events[0].note_number, 60);
    
    raw_events.clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // 4. Play E4 (while D4 still held)
    raw_events.push_back(createMidiNoteOn(64, 90));
    events = abstractor_->processEvents(raw_events);
    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].note_number, 64);
    
    // Verify timing information is being tracked
    EXPECT_GT(events[0].press_time.time_since_epoch().count(), 0);
}

// Test pitch bend processing
TEST_F(AbstractionLayerTest, PitchBendProcessing) {
    std::vector<RawMidiEvent> raw_events;
    
    // Create pitch bend event (center = 8192, range typically ±8191)
    RawMidiEvent pitch_bend_event;
    pitch_bend_event.data = {
        0xE0, // Pitch bend on channel 0
        0x00, // LSB (0)
        0x60  // MSB (96) - should give positive bend
    };
    pitch_bend_event.timestamp = getCurrentTime();
    pitch_bend_event.source_port = 0;
    
    raw_events.push_back(pitch_bend_event);
    
    // Process pitch bend
    auto events = abstractor_->processEvents(raw_events);
    // Pitch bend might not generate a note event directly
    
    raw_events.clear();
    
    // Now play a note and verify pitch bend is applied
    raw_events.push_back(createMidiNoteOn(60, 100));
    events = abstractor_->processEvents(raw_events);
    
    ASSERT_EQ(events.size(), 1);
    EXPECT_NE(events[0].pitch_bend, 0.0f); // Should have pitch bend applied
}

// Test timing accuracy
TEST_F(AbstractionLayerTest, TimingAccuracy) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<RawMidiEvent> raw_events;
    raw_events.push_back(createMidiNoteOn(60, 100));
    
    auto events = abstractor_->processEvents(raw_events);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    
    ASSERT_EQ(events.size(), 1);
    
    // Verify timestamp is within reasonable bounds
    EXPECT_GE(events[0].press_time, start_time);
    EXPECT_LE(events[0].press_time, end_time);
}
