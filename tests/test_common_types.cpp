#include <gtest/gtest.h>
#include "../shared/interfaces/common_types.h"
#include "../shared/interfaces/dll_interfaces.h"
#include <chrono>
#include <thread>

using namespace PianoSynth::Interfaces;

class CommonTypesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test MusicalEvent structure
TEST_F(CommonTypesTest, MusicalEventBasics) {
    MusicalEvent event;
    
    // Test default initialization
    EXPECT_EQ(event.type, EventType::NOTE_ON);
    EXPECT_EQ(event.note_number, 60);
    EXPECT_FLOAT_EQ(event.velocity, 0.0f);
    EXPECT_FLOAT_EQ(event.release_velocity, 0.0f);
    EXPECT_FLOAT_EQ(event.pressure, 0.0f);
    EXPECT_FLOAT_EQ(event.pitch_bend, 0.0f);
    EXPECT_EQ(event.channel, 0);
    EXPECT_EQ(event.source_device_id, 0);
    
    // Test assignment
    event.type = EventType::NOTE_OFF;
    event.note_number = 72;
    event.velocity = 0.8f;
    event.channel = 5;
    
    EXPECT_EQ(event.type, EventType::NOTE_OFF);
    EXPECT_EQ(event.note_number, 72);
    EXPECT_FLOAT_EQ(event.velocity, 0.8f);
    EXPECT_EQ(event.channel, 5);
}

// Test PedalState structure
TEST_F(CommonTypesTest, PedalStateBasics) {
    PedalState pedals;
    
    // Test default initialization
    EXPECT_FALSE(pedals.sustain);
    EXPECT_FALSE(pedals.soft);
    EXPECT_FALSE(pedals.sostenuto);
    EXPECT_FLOAT_EQ(pedals.sustain_position, 0.0f);
    EXPECT_FLOAT_EQ(pedals.soft_position, 0.0f);
    EXPECT_FLOAT_EQ(pedals.sostenuto_position, 0.0f);
    
    // Test assignment
    pedals.sustain = true;
    pedals.sustain_position = 0.7f;
    pedals.soft = true;
    pedals.soft_position = 0.3f;
    
    EXPECT_TRUE(pedals.sustain);
    EXPECT_FLOAT_EQ(pedals.sustain_position, 0.7f);
    EXPECT_TRUE(pedals.soft);
    EXPECT_FLOAT_EQ(pedals.soft_position, 0.3f);
    EXPECT_FALSE(pedals.sostenuto); // Should remain false
}

// Test AudioBuffer structure
TEST_F(CommonTypesTest, AudioBufferBasics) {
    AudioBuffer buffer;
    
    // Test default initialization
    EXPECT_EQ(buffer.samples, nullptr);
    EXPECT_EQ(buffer.frame_count, 0);
    EXPECT_EQ(buffer.channel_count, 2); // Default stereo
    EXPECT_DOUBLE_EQ(buffer.sample_rate, 44100.0);
    
    // Test helper methods
    EXPECT_EQ(buffer.total_samples(), 0);
    EXPECT_EQ(buffer.size_bytes(), 0);
    
    // Test with actual data
    float test_data[1024];
    buffer.samples = test_data;
    buffer.frame_count = 256;
    buffer.channel_count = 2;
    
    EXPECT_EQ(buffer.total_samples(), 512); // 256 frames * 2 channels
    EXPECT_EQ(buffer.size_bytes(), 512 * sizeof(float));
    
    // Test with mono
    buffer.channel_count = 1;
    EXPECT_EQ(buffer.total_samples(), 256);
    EXPECT_EQ(buffer.size_bytes(), 256 * sizeof(float));
}

// Test EventType enum
TEST_F(CommonTypesTest, EventTypeEnum) {
    // Test all event types are distinct
    EXPECT_NE(EventType::NOTE_ON, EventType::NOTE_OFF);
    EXPECT_NE(EventType::NOTE_ON, EventType::PEDAL_CHANGE);
    EXPECT_NE(EventType::NOTE_ON, EventType::PITCH_BEND);
    EXPECT_NE(EventType::NOTE_ON, EventType::CONTROL_CHANGE);
    EXPECT_NE(EventType::NOTE_ON, EventType::AFTERTOUCH);
    EXPECT_NE(EventType::NOTE_ON, EventType::SYSTEM_RESET);
    
    // Test assignment and comparison
    EventType type = EventType::NOTE_ON;
    EXPECT_EQ(type, EventType::NOTE_ON);
    
    type = EventType::PEDAL_CHANGE;
    EXPECT_EQ(type, EventType::PEDAL_CHANGE);
    EXPECT_NE(type, EventType::NOTE_ON);
}

// Test timestamp functionality
TEST_F(CommonTypesTest, TimestampHandling) {
    MusicalEvent event1, event2;
    
    // Set different timestamps
    event1.timestamp = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    event2.timestamp = std::chrono::high_resolution_clock::now();
    
    // event2 should be later than event1
    EXPECT_LT(event1.timestamp, event2.timestamp);
    
    // Test duration calculation
    auto duration = event2.timestamp - event1.timestamp;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    EXPECT_GT(microseconds.count(), 50); // Should be at least 50 microseconds
}

// Test event construction patterns
TEST_F(CommonTypesTest, EventConstructionPatterns) {
    // Test note on event
    MusicalEvent note_on;
    note_on.type = EventType::NOTE_ON;
    note_on.note_number = 60; // C4
    note_on.velocity = 0.8f;
    note_on.channel = 0;
    note_on.timestamp = std::chrono::high_resolution_clock::now();
    
    EXPECT_EQ(note_on.type, EventType::NOTE_ON);
    EXPECT_EQ(note_on.note_number, 60);
    EXPECT_FLOAT_EQ(note_on.velocity, 0.8f);
    
    // Test note off event
    MusicalEvent note_off;
    note_off.type = EventType::NOTE_OFF;
    note_off.note_number = 60;
    note_off.release_velocity = 0.5f;
    note_off.channel = 0;
    note_off.timestamp = std::chrono::high_resolution_clock::now();
    
    EXPECT_EQ(note_off.type, EventType::NOTE_OFF);
    EXPECT_EQ(note_off.note_number, 60);
    EXPECT_FLOAT_EQ(note_off.release_velocity, 0.5f);
    
    // Test pedal event
    MusicalEvent pedal_event;
    pedal_event.type = EventType::PEDAL_CHANGE;
    pedal_event.pedals.sustain = true;
    pedal_event.pedals.sustain_position = 1.0f;
    pedal_event.timestamp = std::chrono::high_resolution_clock::now();
    
    EXPECT_EQ(pedal_event.type, EventType::PEDAL_CHANGE);
    EXPECT_TRUE(pedal_event.pedals.sustain);
    EXPECT_FLOAT_EQ(pedal_event.pedals.sustain_position, 1.0f);
    
    // Test control change event
    MusicalEvent cc_event;
    cc_event.type = EventType::CONTROL_CHANGE;
    cc_event.controller_number = 7; // Volume
    cc_event.controller_value = 0.75f;
    cc_event.channel = 2;
    cc_event.timestamp = std::chrono::high_resolution_clock::now();
    
    EXPECT_EQ(cc_event.type, EventType::CONTROL_CHANGE);
    EXPECT_EQ(cc_event.controller_number, 7);
    EXPECT_FLOAT_EQ(cc_event.controller_value, 0.75f);
    EXPECT_EQ(cc_event.channel, 2);
}

// Test audio buffer with real data
TEST_F(CommonTypesTest, AudioBufferWithData) {
    // Create test audio data
    const size_t frames = 128;
    const size_t channels = 2;
    std::vector<float> audio_data(frames * channels);
    
    // Fill with test pattern
    for (size_t i = 0; i < audio_data.size(); i++) {
        audio_data[i] = static_cast<float>(i) / audio_data.size();
    }
    
    // Create buffer
    AudioBuffer buffer;
    buffer.samples = audio_data.data();
    buffer.frame_count = frames;
    buffer.channel_count = channels;
    buffer.sample_rate = 48000.0;
    buffer.timestamp = std::chrono::high_resolution_clock::now();
    
    // Test accessors
    EXPECT_EQ(buffer.total_samples(), frames * channels);
    EXPECT_EQ(buffer.size_bytes(), frames * channels * sizeof(float));
    
    // Test data integrity
    EXPECT_FLOAT_EQ(buffer.samples[0], 0.0f);
    EXPECT_FLOAT_EQ(buffer.samples[1], 1.0f / (frames * channels));
    EXPECT_FLOAT_EQ(buffer.samples[audio_data.size() - 1], 
                   static_cast<float>(audio_data.size() - 1) / audio_data.size());
}

// Test range validation helpers
TEST_F(CommonTypesTest, RangeValidation) {
    MusicalEvent event;
    
    // Test valid MIDI note range
    for (int note = 0; note <= 127; note++) {
        event.note_number = note;
        EXPECT_GE(event.note_number, 0);
        EXPECT_LE(event.note_number, 127);
    }
    
    // Test valid velocity range (0.0 - 1.0)
    event.velocity = 0.0f;
    EXPECT_FLOAT_EQ(event.velocity, 0.0f);
    
    event.velocity = 1.0f;
    EXPECT_FLOAT_EQ(event.velocity, 1.0f);
    
    event.velocity = 0.5f;
    EXPECT_FLOAT_EQ(event.velocity, 0.5f);
    
    // Test valid channel range (0-15 for MIDI)
    for (int channel = 0; channel < 16; channel++) {
        event.channel = channel;
        EXPECT_GE(event.channel, 0);
        EXPECT_LT(event.channel, 16);
    }
    
    // Test pitch bend range (-1.0 to +1.0)
    event.pitch_bend = -1.0f;
    EXPECT_FLOAT_EQ(event.pitch_bend, -1.0f);
    
    event.pitch_bend = 1.0f;
    EXPECT_FLOAT_EQ(event.pitch_bend, 1.0f);
    
    event.pitch_bend = 0.0f;
    EXPECT_FLOAT_EQ(event.pitch_bend, 0.0f);
}
