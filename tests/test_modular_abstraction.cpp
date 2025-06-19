#include <gtest/gtest.h>
#include "../shared/interfaces/dll_interfaces.h"
#include "../shared/interfaces/common_types.h"
#include <vector>
#include <chrono>
#include <thread>
#include <dlfcn.h>
#include <algorithm>

using namespace PianoSynth::Interfaces;

class ModularAbstractionLayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Load abstraction DLL
        abstraction_dll_ = dlopen("./dlls/libpiano_abstraction.so", RTLD_LAZY);
        if (!abstraction_dll_) {
            GTEST_SKIP() << "Could not load abstraction DLL: " << dlerror();
        }
        
        // Get the creation function
        typedef IAbstractionProcessor* (*create_func_t)();
        create_func_t create_abstraction = (create_func_t) dlsym(abstraction_dll_, "create_abstraction_processor");
        
        if (!create_abstraction) {
            GTEST_SKIP() << "Could not find create_abstraction_processor function";
        }
        
        // Create the abstraction processor
        abstractor_ = create_abstraction();
        if (!abstractor_) {
            GTEST_SKIP() << "Could not create abstraction processor";
        }
        
        // Initialize with test configuration
        const char* test_config = R"({
            "abstraction_processor": {
                "velocity_processing": {
                    "velocity_curve": 1.0,
                    "velocity_sensitivity": 1.0,
                    "min_velocity": 0.01,
                    "max_velocity": 1.0
                },
                "timing": {
                    "enable_quantization": false,
                    "max_note_duration_ms": 30000
                },
                "pedal_processing": {
                    "sustain_sensitivity": 1.0,
                    "sustain_threshold": 0.5
                },
                "note_management": {
                    "max_active_notes": 256,
                    "note_priority": "velocity"
                }
            }
        })";
        
        if (!abstractor_->initialize(test_config)) {
            GTEST_SKIP() << "Could not initialize abstraction processor";
        }
    }

    void TearDown() override {
        if (abstractor_) {
            typedef void (*destroy_func_t)(IAbstractionProcessor*);
            destroy_func_t destroy_abstraction = (destroy_func_t) dlsym(abstraction_dll_, "destroy_abstraction_processor");
            if (destroy_abstraction) {
                destroy_abstraction(abstractor_);
            }
            abstractor_ = nullptr;
        }
        
        if (abstraction_dll_) {
            dlclose(abstraction_dll_);
            abstraction_dll_ = nullptr;
        }
    }

    // Helper function to create a test musical event
    MusicalEvent createNoteOnEvent(int note, float velocity, int channel = 0) {
        MusicalEvent event;
        event.type = EventType::NOTE_ON;
        event.timestamp = std::chrono::high_resolution_clock::now();
        event.note_number = note;
        event.velocity = velocity;
        event.channel = channel;
        return event;
    }
    
    MusicalEvent createNoteOffEvent(int note, float velocity, int channel = 0) {
        MusicalEvent event;
        event.type = EventType::NOTE_OFF;
        event.timestamp = std::chrono::high_resolution_clock::now();
        event.note_number = note;
        event.release_velocity = velocity;
        event.channel = channel;
        return event;
    }
    
    MusicalEvent createPedalEvent(bool pressed) {
        MusicalEvent event;
        event.type = EventType::PEDAL_CHANGE;
        event.timestamp = std::chrono::high_resolution_clock::now();
        event.pedals.sustain = pressed;
        event.pedals.sustain_position = pressed ? 1.0f : 0.0f;
        return event;
    }

    void* abstraction_dll_ = nullptr;
    IAbstractionProcessor* abstractor_ = nullptr;
};

// Test single note processing through the DLL interface
TEST_F(ModularAbstractionLayerTest, SingleNoteProcessing) {
    if (!abstractor_) GTEST_SKIP();
    
    // Create input events
    std::vector<MusicalEvent> input_events;
    input_events.push_back(createNoteOnEvent(60, 0.8f)); // C4 with 80% velocity
    
    // Process events
    std::vector<MusicalEvent> output_events(10);
    int processed_count = abstractor_->process_events(
        input_events.data(), 
        input_events.size(),
        output_events.data(), 
        output_events.size()
    );
    
    // Verify results
    ASSERT_GE(processed_count, 1);
    
    const auto& processed_event = output_events[0];
    EXPECT_EQ(processed_event.type, EventType::NOTE_ON);
    EXPECT_EQ(processed_event.note_number, 60);
    EXPECT_FLOAT_EQ(processed_event.velocity, 0.8f);
    EXPECT_GT(processed_event.timestamp.time_since_epoch().count(), 0);
}

// Test chord processing (multiple simultaneous notes)
TEST_F(ModularAbstractionLayerTest, ChordProcessing) {
    if (!abstractor_) GTEST_SKIP();
    
    // Create C Major chord: C4, E4, G4
    std::vector<MusicalEvent> input_events;
    input_events.push_back(createNoteOnEvent(60, 0.7f)); // C4
    input_events.push_back(createNoteOnEvent(64, 0.75f)); // E4
    input_events.push_back(createNoteOnEvent(67, 0.8f)); // G4
    
    // Process events
    std::vector<MusicalEvent> output_events(10);
    int processed_count = abstractor_->process_events(
        input_events.data(), 
        input_events.size(),
        output_events.data(), 
        output_events.size()
    );
    
    // Should get all three notes processed
    ASSERT_EQ(processed_count, 3);
    
    // Verify all notes are present (order might vary)
    std::vector<int> processed_notes;
    for (int i = 0; i < processed_count; i++) {
        EXPECT_EQ(output_events[i].type, EventType::NOTE_ON);
        processed_notes.push_back(output_events[i].note_number);
    }
    
    std::sort(processed_notes.begin(), processed_notes.end());
    std::vector<int> expected_notes = {60, 64, 67};
    EXPECT_EQ(processed_notes, expected_notes);
}

// Test velocity curve processing
TEST_F(ModularAbstractionLayerTest, VelocityCurveProcessing) {
    if (!abstractor_) GTEST_SKIP();
    
    // Test different velocity curves
    std::vector<float> test_curves = {0.5f, 1.0f, 2.0f};
    
    for (float curve : test_curves) {
        // Set velocity curve
        EXPECT_TRUE(abstractor_->set_velocity_curve(curve));
        
        // Test with multiple velocities
        std::vector<float> input_velocities = {0.2f, 0.5f, 0.8f, 1.0f};
        std::vector<float> output_velocities;
        
        for (float input_vel : input_velocities) {
            std::vector<MusicalEvent> input_events;
            input_events.push_back(createNoteOnEvent(60, input_vel));
            
            std::vector<MusicalEvent> output_events(2);
            int processed_count = abstractor_->process_events(
                input_events.data(), 1,
                output_events.data(), output_events.size()
            );
            
            ASSERT_GE(processed_count, 1);
            output_velocities.push_back(output_events[0].velocity);
            
            // Reset state for next test
            abstractor_->reset_state();
        }
        
        // Verify velocities are monotonically increasing
        for (size_t i = 1; i < output_velocities.size(); i++) {
            EXPECT_GE(output_velocities[i], output_velocities[i-1])
                << "Velocity curve " << curve << " failed monotonicity test";
        }
    }
}

// Test sustain pedal processing
TEST_F(ModularAbstractionLayerTest, SustainPedalProcessing) {
    if (!abstractor_) GTEST_SKIP();
    
    // Press sustain pedal
    std::vector<MusicalEvent> input_events;
    input_events.push_back(createPedalEvent(true));
    
    std::vector<MusicalEvent> output_events(2);
    int processed_count = abstractor_->process_events(
        input_events.data(), 1,
        output_events.data(), output_events.size()
    );
    
    ASSERT_GE(processed_count, 1);
    EXPECT_EQ(output_events[0].type, EventType::PEDAL_CHANGE);
    EXPECT_TRUE(output_events[0].pedals.sustain);
    
    // Now play a note with sustain active
    input_events.clear();
    input_events.push_back(createNoteOnEvent(60, 0.8f));
    
    processed_count = abstractor_->process_events(
        input_events.data(), 1,
        output_events.data(), output_events.size()
    );
    
    ASSERT_GE(processed_count, 1);
    EXPECT_EQ(output_events[0].type, EventType::NOTE_ON);
    EXPECT_TRUE(output_events[0].pedals.sustain);
    
    // Release sustain pedal
    input_events.clear();
    input_events.push_back(createPedalEvent(false));
    
    processed_count = abstractor_->process_events(
        input_events.data(), 1,
        output_events.data(), output_events.size()
    );
    
    ASSERT_GE(processed_count, 1);
    EXPECT_EQ(output_events[0].type, EventType::PEDAL_CHANGE);
    EXPECT_FALSE(output_events[0].pedals.sustain);
}

// Test note on/off pairing
TEST_F(ModularAbstractionLayerTest, NoteOnOffSequence) {
    if (!abstractor_) GTEST_SKIP();
    
    // Play note on
    std::vector<MusicalEvent> input_events;
    input_events.push_back(createNoteOnEvent(60, 0.8f));
    
    std::vector<MusicalEvent> output_events(2);
    int processed_count = abstractor_->process_events(
        input_events.data(), 1,
        output_events.data(), output_events.size()
    );
    
    ASSERT_GE(processed_count, 1);
    EXPECT_EQ(output_events[0].type, EventType::NOTE_ON);
    EXPECT_EQ(output_events[0].note_number, 60);
    
    // Small delay to simulate realistic timing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Play note off
    input_events.clear();
    input_events.push_back(createNoteOffEvent(60, 0.5f));
    
    processed_count = abstractor_->process_events(
        input_events.data(), 1,
        output_events.data(), output_events.size()
    );
    
    ASSERT_GE(processed_count, 1);
    EXPECT_EQ(output_events[0].type, EventType::NOTE_OFF);
    EXPECT_EQ(output_events[0].note_number, 60);
    EXPECT_FLOAT_EQ(output_events[0].release_velocity, 0.5f);
}

// Test configuration updates
TEST_F(ModularAbstractionLayerTest, ConfigurationUpdates) {
    if (!abstractor_) GTEST_SKIP();
    
    // Test pedal sensitivity update
    EXPECT_TRUE(abstractor_->set_pedal_sensitivity(0.5f));
    
    // Test timing quantization update  
    EXPECT_TRUE(abstractor_->set_timing_quantization(0.1f));
    
    // Test velocity curve update
    EXPECT_TRUE(abstractor_->set_velocity_curve(1.5f));
    
    // Test full configuration update
    const char* new_config = R"({
        "abstraction_processor": {
            "velocity_processing": {
                "velocity_curve": 2.0,
                "velocity_sensitivity": 0.8
            },
            "pedal_processing": {
                "sustain_sensitivity": 0.7
            }
        }
    })";
    
    EXPECT_TRUE(abstractor_->update_config(new_config));
}

// Test error handling and edge cases
TEST_F(ModularAbstractionLayerTest, ErrorHandling) {
    if (!abstractor_) GTEST_SKIP();
    
    // Test with invalid configuration
    EXPECT_FALSE(abstractor_->update_config("invalid json"));
    
    // Test with null pointers
    EXPECT_EQ(abstractor_->process_events(nullptr, 0, nullptr, 0), 0);
    
    // Test with oversized input
    std::vector<MusicalEvent> many_events(1000);
    for (size_t i = 0; i < many_events.size(); i++) {
        many_events[i] = createNoteOnEvent(60 + (i % 12), 0.8f);
    }
    
    std::vector<MusicalEvent> output_events(10);
    int processed_count = abstractor_->process_events(
        many_events.data(), many_events.size(),
        output_events.data(), output_events.size()
    );
    
    // Should process up to the output buffer limit
    EXPECT_LE(processed_count, static_cast<int>(output_events.size()));
    EXPECT_GE(processed_count, 0);
}
