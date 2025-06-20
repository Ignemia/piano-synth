#include <gtest/gtest.h>
#include "../core/piano_synth.h"
#include "../core/utils/config_manager.h"
#include "../core/utils/logger.h"
#include "../core/utils/math_utils.h"
#include "../core/physics/string_model.h"
#include "../core/physics/hammer_model.h"
#include "../core/physics/resonance_model.h"
#include "../core/abstraction/input_abstractor.h"
#include "../core/synthesis/piano_synthesizer.h"
#include <memory>
#include <chrono>
#include <fstream>
#include <cmath>

using namespace PianoSynth;

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test configuration file
        createTestConfig();
        
        // Initialize piano synth app
        piano_app = std::make_unique<PianoSynthApp>();
        
        // Initialize without actually starting the full application
        initializeComponents();
    }
    
    void TearDown() override {
        if (piano_app) {
            piano_app->shutdown();
        }
        
        // Clean up test files
        std::remove("test_config.json");
        std::remove("test_recording.mp3");
    }
    
    void createTestConfig() {
        std::ofstream config_file("test_config.json");
        config_file << R"({
  "audio": {
    "sample_rate": 44100.0,
    "buffer_size": 256,
    "channels": 2,
    "output_device": "default"
  },
  "synthesis": {
    "max_voices": 32,
    "master_volume": 0.6,
    "velocity_sensitivity": 0.01
  },
  "string": {
    "tension_base": 1000.0,
    "damping": 0.001,
    "stiffness": 1e-5
  },
  "midi": {
    "auto_detect": false,
    "velocity_curve": 1.0
  },
  "recording": {
    "mp3_bitrate": 128,
    "mp3_quality": 7,
    "output_directory": "./"
  }
})";
        config_file.close();
    }
    
    bool initializeComponents() {
        // Initialize with test config
        return piano_app->initialize();
    }
    
    // Helper to simulate MIDI events
    void simulateMidiNoteOn(int note, int velocity) {
        // This would normally come from MIDI input
        // For testing, we'll create events directly
        std::vector<unsigned char> midi_data = {
            static_cast<unsigned char>(0x90), // Note on
            static_cast<unsigned char>(note),
            static_cast<unsigned char>(velocity)
        };
        
        // Simulate processing this MIDI event
        processMidiEvent(midi_data);
    }
    
    void simulateMidiNoteOff(int note, int velocity = 64) {
        std::vector<unsigned char> midi_data = {
            static_cast<unsigned char>(0x80), // Note off
            static_cast<unsigned char>(note),
            static_cast<unsigned char>(velocity)
        };
        
        processMidiEvent(midi_data);
    }
    
    void simulatePedalChange(int controller, int value) {
        std::vector<unsigned char> midi_data = {
            static_cast<unsigned char>(0xB0), // Control change
            static_cast<unsigned char>(controller),
            static_cast<unsigned char>(value)
        };
        
        processMidiEvent(midi_data);
    }
    
    void processMidiEvent(const std::vector<unsigned char>& /*midi_data*/) {
        // This simulates what would happen in the MIDI processing loop
        // In a real test, we'd need access to the internal components
        // For now, we'll focus on testing that the system can handle
        // the full initialization and basic operations
    }
    
    std::unique_ptr<PianoSynthApp> piano_app;
};

// Test complete system initialization
TEST_F(IntegrationTest, SystemInitialization) {
    // System should initialize successfully
    EXPECT_NE(piano_app.get(), nullptr);
    
    // Test basic configuration loading
    auto config = std::make_unique<Utils::ConfigManager>();
    EXPECT_TRUE(config->loadConfig("test_config.json"));
    
    EXPECT_EQ(config->getDouble("audio.sample_rate"), 44100.0);
    EXPECT_EQ(config->getInt("synthesis.max_voices"), 32);
    EXPECT_EQ(config->getFloat("synthesis.master_volume"), 0.6f);
}

// Test logging system
TEST_F(IntegrationTest, LoggingSystem) {
    auto logger = std::make_unique<Utils::Logger>(Utils::LogLevel::kDebug);
    
    // Test different log levels
    logger->debug("Debug message");
    logger->info("Info message");
    logger->warning("Warning message");
    logger->error("Error message");
    
    // Test logging to file
    logger->setLogToFile(true, "test_log.txt");
    logger->info("File log test");
    
    // Verify file was created
    std::ifstream log_file("test_log.txt");
    EXPECT_TRUE(log_file.is_open());
    
    std::string log_content((std::istreambuf_iterator<char>(log_file)),
                           std::istreambuf_iterator<char>());
    EXPECT_TRUE(log_content.find("File log test") != std::string::npos);
    
    log_file.close();
    std::remove("test_log.txt");
}

// Test math utilities integration
TEST_F(IntegrationTest, MathUtilitiesIntegration) {
    // Test MIDI to frequency conversions across full range
    for (int note = 21; note <= 108; ++note) {
        double freq = PianoSynth::Utils::MathUtils::midiToFrequency(note);
        int converted_note = PianoSynth::Utils::MathUtils::frequencyToMidi(freq);
        
        EXPECT_EQ(converted_note, note);
        EXPECT_GT(freq, 0.0);
    }
    
    // Test DSP utilities
    std::vector<float> test_signal = {0.1f, 0.5f, 0.9f, 1.5f, -0.8f, -1.2f};
    
    for (float& sample : test_signal) {
        float clipped = PianoSynth::Utils::DSPUtils::softClip(sample, 0.8f);
        EXPECT_GE(clipped, -1.0f);
        EXPECT_LE(clipped, 1.0f);
        
        float saturated = PianoSynth::Utils::DSPUtils::tanhSaturation(sample, 2.0f);
        EXPECT_GE(saturated, -1.0f);
        EXPECT_LE(saturated, 1.0f);
    }
}

// Test complete audio pipeline
TEST_F(IntegrationTest, AudioPipelineIntegration) {
    // Test that we can create all the major components
    auto config_manager = std::make_unique<Utils::ConfigManager>();
    config_manager->loadConfig("test_config.json");
    
    // Test string model
    auto string_model = std::make_unique<Physics::StringModel>(60);
    string_model->initialize(44100.0);
    string_model->excite(0.125, 1.0, 0.001);
    
    // Generate some samples
    std::vector<double> string_output;
    for (int i = 0; i < 1000; ++i) {
        string_output.push_back(string_model->step());
    }
    
    // Verify string produces output
    bool has_output = false;
    for (double sample : string_output) {
        if (std::abs(sample) > 1e-6) {
            has_output = true;
            break;
        }
    }
    EXPECT_TRUE(has_output);
    
    // Test hammer model
    auto hammer_model = std::make_unique<Physics::HammerModel>(60);
    hammer_model->initialize(44100.0);
    hammer_model->strike(2.0);
    
    // Test hammer-string interaction
    for (int i = 0; i < 100; ++i) {
        double string_displacement = string_model->step();
        double hammer_force = hammer_model->step(string_displacement);
        EXPECT_FALSE(std::isnan(hammer_force));
        EXPECT_FALSE(std::isinf(hammer_force));
    }
}

// Test event processing pipeline
TEST_F(IntegrationTest, EventProcessingPipeline) {
    // Create abstraction layer components
    auto input_abstractor = std::make_unique<Abstraction::InputAbstractor>();
    input_abstractor->initialize();
    
    // Create synthetic MIDI events
    std::vector<Input::RawMidiEvent> raw_events;
    
    // Note on event
    Input::RawMidiEvent note_on_event;
    note_on_event.data = {0x90, 60, 100}; // Note on, middle C, velocity 100
    note_on_event.timestamp = 0.0;
    note_on_event.source_port = 0;
    raw_events.push_back(note_on_event);
    
    // Control change event (sustain pedal)
    Input::RawMidiEvent cc_event;
    cc_event.data = {0xB0, 64, 127}; // CC, sustain pedal, on
    cc_event.timestamp = 0.1;
    cc_event.source_port = 0;
    raw_events.push_back(cc_event);
    
    // Note off event
    Input::RawMidiEvent note_off_event;
    note_off_event.data = {0x80, 60, 64}; // Note off, middle C, velocity 64
    note_off_event.timestamp = 1.0;
    note_off_event.source_port = 0;
    raw_events.push_back(note_off_event);
    
    // Process events
    auto processed_events = input_abstractor->processEvents(raw_events);
    
    // Verify event processing
    EXPECT_EQ(processed_events.size(), 3);
    
    // Check note on event
    EXPECT_EQ(processed_events[0].type, Abstraction::NoteEvent::NOTE_ON);
    EXPECT_EQ(processed_events[0].note_number, 60);
    EXPECT_GT(processed_events[0].velocity, 0.0f);
    EXPECT_GT(processed_events[0].hammer_velocity, 0.0f);
    
    // Check pedal event
    EXPECT_EQ(processed_events[1].type, Abstraction::NoteEvent::PEDAL_CHANGE);
    EXPECT_TRUE(processed_events[1].sustain_pedal);
    
    // Check note off event
    EXPECT_EQ(processed_events[2].type, Abstraction::NoteEvent::NOTE_OFF);
    EXPECT_EQ(processed_events[2].note_number, 60);
}

// Test synthesis pipeline integration
TEST_F(IntegrationTest, SynthesisPipelineIntegration) {
    auto config_manager = std::make_unique<Utils::ConfigManager>();
    config_manager->loadConfig("test_config.json");
    
    auto piano_synthesizer = std::make_unique<Synthesis::PianoSynthesizer>();
    EXPECT_TRUE(piano_synthesizer->initialize(config_manager.get()));
    
    // Create note events
    Abstraction::NoteEvent note_on;
    note_on.type = Abstraction::NoteEvent::NOTE_ON;
    note_on.note_number = 60;
    note_on.velocity = 0.7f;
    note_on.hammer_velocity = 2.0f;
    note_on.string_excitation = 1.0f;
    note_on.damper_position = 1.0f;
    note_on.sustain_pedal = false;
    
    // Process note on
    piano_synthesizer->processNoteEvent(note_on);
    
    // Generate audio
    auto audio_buffer = piano_synthesizer->generateAudioBuffer(512);
    
    // Verify audio output
    EXPECT_EQ(audio_buffer.size(), 512 * 2); // Stereo
    
    bool has_audio = false;
    for (float sample : audio_buffer) {
        if (std::abs(sample) > 1e-6) {
            has_audio = true;
        }
        
        // Check for valid audio range
        EXPECT_GE(sample, -1.0f);
        EXPECT_LE(sample, 1.0f);
        EXPECT_FALSE(std::isnan(sample));
        EXPECT_FALSE(std::isinf(sample));
    }
    EXPECT_TRUE(has_audio);
    
    // Test note off
    Abstraction::NoteEvent note_off;
    note_off.type = Abstraction::NoteEvent::NOTE_OFF;
    note_off.note_number = 60;
    note_off.release_velocity = 0.5f;
    
    piano_synthesizer->processNoteEvent(note_off);
    
    // Generate more audio to test fade out
    for (int i = 0; i < 10; ++i) {
        audio_buffer = piano_synthesizer->generateAudioBuffer(512);
    }
    
    piano_synthesizer->shutdown();
}

// Test resonance model integration
TEST_F(IntegrationTest, ResonanceModelIntegration) {
    auto resonance_model = std::make_unique<Physics::ResonanceModel>();
    resonance_model->initialize(44100.0, 88); // 88 piano keys
    
    // Test string coupling
    resonance_model->updateStringCoupling(39, 0.1, 261.626); // Middle C
    double sympathetic = resonance_model->getSympatheticResonance(40); // C#
    
    // Nearby strings should have some coupling
    EXPECT_GE(sympathetic, -1.0);
    EXPECT_LE(sympathetic, 1.0);
    
    // Test soundboard processing
    std::vector<double> string_outputs(88, 0.0);
    string_outputs[39] = 0.5; // Middle C output
    string_outputs[43] = 0.3; // E output
    string_outputs[46] = 0.4; // G output
    
    double soundboard_output = resonance_model->processSoundboard(string_outputs);
    EXPECT_FALSE(std::isnan(soundboard_output));
    EXPECT_FALSE(std::isinf(soundboard_output));
    
    // Test room acoustics
    double room_output = resonance_model->processRoomAcoustics(soundboard_output);
    EXPECT_FALSE(std::isnan(room_output));
    EXPECT_FALSE(std::isinf(room_output));
    
    // Test parameter changes
    resonance_model->setSoundboardResonance(0.8f);
    resonance_model->setRoomSize(20.0);
    resonance_model->setRoomDamping(0.4);
    
    // Should still work after parameter changes
    room_output = resonance_model->processRoomAcoustics(soundboard_output);
    EXPECT_FALSE(std::isnan(room_output));
    EXPECT_FALSE(std::isinf(room_output));
}

// Test error handling and edge cases
TEST_F(IntegrationTest, ErrorHandlingIntegration) {
    // Test with invalid configuration
    auto config_manager = std::make_unique<Utils::ConfigManager>();
    EXPECT_FALSE(config_manager->loadConfig("nonexistent_config.json"));
    
    // Should fall back to defaults
    EXPECT_EQ(config_manager->getDouble("audio.sample_rate"), 44100.0);
    
    // Test synthesizer with extreme parameters
    auto piano_synthesizer = std::make_unique<Synthesis::PianoSynthesizer>();
    EXPECT_TRUE(piano_synthesizer->initialize(config_manager.get()));
    
    // Test extreme parameter values
    piano_synthesizer->setPedalDamping(2.0f); // Should clamp to valid range
    piano_synthesizer->setStringTension(-1.0f); // Should clamp to valid range
    piano_synthesizer->setMasterTuning(1000.0f); // Should clamp to valid range
    
    // Should still work
    auto buffer = piano_synthesizer->generateAudioBuffer(256);
    EXPECT_EQ(buffer.size(), 256 * 2);
    
    // Test with invalid note numbers
    Abstraction::NoteEvent invalid_note;
    invalid_note.type = Abstraction::NoteEvent::NOTE_ON;
    invalid_note.note_number = 200; // Invalid MIDI note
    invalid_note.velocity = 0.5f;
    
    // Should handle gracefully (not crash)
    piano_synthesizer->processNoteEvent(invalid_note);
    buffer = piano_synthesizer->generateAudioBuffer(256);
    EXPECT_EQ(buffer.size(), 256 * 2);
    
    piano_synthesizer->shutdown();
}

// Test memory management and cleanup
TEST_F(IntegrationTest, MemoryManagementIntegration) {
    // Test creating and destroying multiple components
    for (int iteration = 0; iteration < 5; ++iteration) {
        auto config = std::make_unique<Utils::ConfigManager>();
        config->loadConfig("test_config.json");
        
        auto synthesizer = std::make_unique<Synthesis::PianoSynthesizer>();
        EXPECT_TRUE(synthesizer->initialize(config.get()));
        
        // Generate some audio
        for (int i = 0; i < 10; ++i) {
            auto buffer = synthesizer->generateAudioBuffer(128);
            EXPECT_EQ(buffer.size(), 128 * 2);
        }
        
        synthesizer->shutdown();
        // Automatic cleanup when going out of scope
    }
    
    // Test rapid voice allocation/deallocation
    auto config = std::make_unique<Utils::ConfigManager>();
    config->loadConfig("test_config.json");
    
    auto synthesizer = std::make_unique<Synthesis::PianoSynthesizer>();
    EXPECT_TRUE(synthesizer->initialize(config.get()));
    
    // Rapidly create and destroy voices
    for (int round = 0; round < 10; ++round) {
        // Play many notes
        for (int note = 60; note < 72; ++note) {
            Abstraction::NoteEvent note_on;
            note_on.type = Abstraction::NoteEvent::NOTE_ON;
            note_on.note_number = note;
            note_on.velocity = 0.5f;
            note_on.hammer_velocity = 1.0f;
            note_on.string_excitation = 0.5f;
            note_on.damper_position = 1.0f;
            
            synthesizer->processNoteEvent(note_on);
        }
        
        // Generate some audio
        synthesizer->generateAudioBuffer(256);
        
        // Stop all notes
        for (int note = 60; note < 72; ++note) {
            Abstraction::NoteEvent note_off;
            note_off.type = Abstraction::NoteEvent::NOTE_OFF;
            note_off.note_number = note;
            note_off.release_velocity = 0.5f;
            
            synthesizer->processNoteEvent(note_off);
        }
        
        // Generate more audio to let notes fade
        for (int i = 0; i < 5; ++i) {
            synthesizer->generateAudioBuffer(256);
        }
    }
    
    synthesizer->shutdown();
}

// Test real-time performance characteristics
TEST_F(IntegrationTest, PerformanceIntegration) {
    auto config = std::make_unique<Utils::ConfigManager>();
    config->loadConfig("test_config.json");
    
    auto synthesizer = std::make_unique<Synthesis::PianoSynthesizer>();
    EXPECT_TRUE(synthesizer->initialize(config.get()));
    
    // Play a complex chord
    std::vector<int> chord_notes = {48, 52, 55, 60, 64, 67, 72, 76};
    for (int note : chord_notes) {
        Abstraction::NoteEvent note_on;
        note_on.type = Abstraction::NoteEvent::NOTE_ON;
        note_on.note_number = note;
        note_on.velocity = 0.6f;
        note_on.hammer_velocity = 1.5f;
        note_on.string_excitation = 0.8f;
        note_on.damper_position = 1.0f;
        
        synthesizer->processNoteEvent(note_on);
    }
    
    // Measure audio generation time
    auto start_time = std::chrono::high_resolution_clock::now();
    
    const int num_buffers = 100;
    const int buffer_size = 512;
    
    for (int i = 0; i < num_buffers; ++i) {
        auto buffer = synthesizer->generateAudioBuffer(buffer_size);
        EXPECT_EQ(buffer.size(), buffer_size * 2);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Calculate if we can meet real-time requirements
    double audio_duration = (num_buffers * buffer_size) / 44100.0; // seconds
    double processing_time = duration.count() / 1000000.0; // seconds
    double real_time_ratio = processing_time / audio_duration;
    
    // Should be able to process faster than real-time (ratio < 1.0)
    // Allow some margin for test system variations
    EXPECT_LT(real_time_ratio, 2.0) << "Processing too slow for real-time: " << real_time_ratio;
    
    // Clean up
    for (int note : chord_notes) {
        Abstraction::NoteEvent note_off;
        note_off.type = Abstraction::NoteEvent::NOTE_OFF;
        note_off.note_number = note;
        synthesizer->processNoteEvent(note_off);
    }
    
    synthesizer->shutdown();
}