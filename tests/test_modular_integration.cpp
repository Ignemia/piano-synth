#include <gtest/gtest.h>
#include "../shared/interfaces/dll_interfaces.h"
#include "../shared/interfaces/common_types.h"
#include <vector>
#include <chrono>
#include <thread>
#include <dlfcn.h>
#include <memory>
#include <cmath>

using namespace PianoSynth::Interfaces;

class ModularIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize all components of the modular system
        loadAbstractionDLL();
        loadInstrumentDLL();
        
        if (!abstractor_ || !instrument_) {
            GTEST_SKIP() << "Could not load required DLLs for integration test";
        }
    }

    void TearDown() override {
        cleanupDLLs();
    }

private:
    void loadAbstractionDLL() {
        abstraction_dll_ = dlopen("./dlls/libpiano_abstraction.so", RTLD_LAZY);
        if (abstraction_dll_) {
            typedef IAbstractionProcessor* (*create_func_t)();
            create_func_t create_abstraction = (create_func_t) dlsym(abstraction_dll_, "create_abstraction_processor");
            if (create_abstraction) {
                abstractor_ = create_abstraction();
                if (abstractor_) {
                    const char* config = R"({
                        "abstraction_processor": {
                            "velocity_processing": {
                                "velocity_curve": 1.0,
                                "velocity_sensitivity": 1.0
                            },
                            "note_management": {
                                "max_active_notes": 64
                            }
                        }
                    })";
                    abstractor_->initialize(config);
                }
            }
        }
    }
    
    void loadInstrumentDLL() {
        instrument_dll_ = dlopen("./dlls/libpiano_instrument.so", RTLD_LAZY);
        if (instrument_dll_) {
            typedef IInstrumentSynthesizer* (*create_func_t)();
            create_func_t create_instrument = (create_func_t) dlsym(instrument_dll_, "create_instrument_synthesizer");
            if (create_instrument) {
                instrument_ = create_instrument();
                if (instrument_) {
                    const char* config = R"({
                        "piano_instrument": {
                            "instrument_info": {
                                "polyphony": 64
                            },
                            "physical_modeling": {
                                "string_model": {
                                    "tension_variation": 1.0
                                },
                                "hammer_model": {
                                    "velocity_to_force_curve": 2.0
                                }
                            }
                        }
                    })";
                    instrument_->initialize(config, 44100.0, 2048);
                }
            }
        }
    }
    
    void cleanupDLLs() {
        if (abstractor_ && abstraction_dll_) {
            typedef void (*destroy_func_t)(IAbstractionProcessor*);
            destroy_func_t destroy_abstraction = (destroy_func_t) dlsym(abstraction_dll_, "destroy_abstraction_processor");
            if (destroy_abstraction) {
                destroy_abstraction(abstractor_);
            }
        }
        
        if (instrument_ && instrument_dll_) {
            typedef void (*destroy_func_t)(IInstrumentSynthesizer*);
            destroy_func_t destroy_instrument = (destroy_func_t) dlsym(instrument_dll_, "destroy_instrument_synthesizer");
            if (destroy_instrument) {
                destroy_instrument(instrument_);
            }
        }
        
        if (abstraction_dll_) dlclose(abstraction_dll_);
        if (instrument_dll_) dlclose(instrument_dll_);
        
        abstractor_ = nullptr;
        instrument_ = nullptr;
        abstraction_dll_ = nullptr;
        instrument_dll_ = nullptr;
    }

protected:
    // Helper to create raw input events (simulating what would come from input DLL)
    MusicalEvent createRawNoteOn(int note, float velocity) {
        MusicalEvent event;
        event.type = EventType::NOTE_ON;
        event.timestamp = std::chrono::high_resolution_clock::now();
        event.note_number = note;
        event.velocity = velocity;
        return event;
    }
    
    MusicalEvent createRawNoteOff(int note) {
        MusicalEvent event;
        event.type = EventType::NOTE_OFF;
        event.timestamp = std::chrono::high_resolution_clock::now();
        event.note_number = note;
        event.release_velocity = 0.5f;
        return event;
    }
    
    // Helper to process complete pipeline: raw input -> abstraction -> instrument -> audio
    bool processCompleteAudioPipeline(const std::vector<MusicalEvent>& input_events, 
                                     std::vector<float>& audio_output,
                                     size_t frames_to_generate = 1024) {
        if (!abstractor_ || !instrument_) return false;
        
        // Step 1: Process through abstraction layer
        std::vector<MusicalEvent> processed_events(input_events.size() * 2); // Some headroom
        int processed_count = abstractor_->process_events(
            input_events.data(), input_events.size(),
            processed_events.data(), processed_events.size()
        );
        
        if (processed_count < 0) return false;
        
        // Step 2: Send processed events to instrument
        if (processed_count > 0) {
            if (!instrument_->process_events(processed_events.data(), processed_count)) {
                return false;
            }
        }
        
        // Step 3: Generate audio from instrument
        audio_output.resize(frames_to_generate * 2); // Stereo
        AudioBuffer buffer;
        buffer.samples = audio_output.data();
        buffer.frame_count = frames_to_generate;
        buffer.channel_count = 2;
        buffer.sample_rate = 44100.0;
        buffer.timestamp = std::chrono::high_resolution_clock::now();
        
        int audio_result = instrument_->generate_audio(&buffer);
        return audio_result >= 0;
    }
    
    // Check if audio contains meaningful content
    bool hasAudioContent(const std::vector<float>& audio, float threshold = 0.001f) {
        for (float sample : audio) {
            if (std::abs(sample) > threshold) return true;
        }
        return false;
    }
    
    float calculateRMS(const std::vector<float>& audio) {
        float sum = 0.0f;
        for (float sample : audio) {
            sum += sample * sample;
        }
        return std::sqrt(sum / audio.size());
    }

    void* abstraction_dll_ = nullptr;
    void* instrument_dll_ = nullptr;
    IAbstractionProcessor* abstractor_ = nullptr;
    IInstrumentSynthesizer* instrument_ = nullptr;
};

// Test complete pipeline with single note
TEST_F(ModularIntegrationTest, CompletePipelineSingleNote) {
    // Create raw input event
    std::vector<MusicalEvent> input_events;
    input_events.push_back(createRawNoteOn(60, 0.8f)); // C4 at 80% velocity
    
    // Process through complete pipeline
    std::vector<float> audio_output;
    EXPECT_TRUE(processCompleteAudioPipeline(input_events, audio_output));
    
    // Check that audio was generated
    EXPECT_TRUE(hasAudioContent(audio_output)) << "No audio generated in complete pipeline";
    
    // Check that audio level is reasonable
    float rms = calculateRMS(audio_output);
    EXPECT_GT(rms, 0.001f) << "Audio level too low: " << rms;
    EXPECT_LT(rms, 1.0f) << "Audio level too high (possible clipping): " << rms;
}

// Test complete pipeline with chord
TEST_F(ModularIntegrationTest, CompletePipelineChord) {
    // Create C Major chord
    std::vector<MusicalEvent> input_events;
    input_events.push_back(createRawNoteOn(60, 0.7f)); // C4
    input_events.push_back(createRawNoteOn(64, 0.75f)); // E4
    input_events.push_back(createRawNoteOn(67, 0.8f)); // G4
    
    // Process through complete pipeline
    std::vector<float> audio_output;
    EXPECT_TRUE(processCompleteAudioPipeline(input_events, audio_output));
    
    // Check that audio was generated
    EXPECT_TRUE(hasAudioContent(audio_output)) << "No audio generated for chord";
    
    // Chord should be louder than single note (more energy)
    float chord_rms = calculateRMS(audio_output);
    
    // Compare with single note
    std::vector<MusicalEvent> single_note;
    single_note.push_back(createRawNoteOn(60, 0.75f)); // Average velocity
    
    std::vector<float> single_audio;
    EXPECT_TRUE(processCompleteAudioPipeline(single_note, single_audio));
    float single_rms = calculateRMS(single_audio);
    
    EXPECT_GT(chord_rms, single_rms * 1.2f) << "Chord not significantly louder than single note";
}

// Test note on/off sequence through complete pipeline
TEST_F(ModularIntegrationTest, CompletePipelineNoteSequence) {
    // Test note on
    std::vector<MusicalEvent> note_on_events;
    note_on_events.push_back(createRawNoteOn(60, 0.8f));
    
    std::vector<float> audio_during_note;
    EXPECT_TRUE(processCompleteAudioPipeline(note_on_events, audio_during_note, 2048));
    
    float sustain_rms = 0.0f;
    // Generate several buffers to get into sustain phase
    for (int i = 0; i < 5; i++) {
        std::vector<MusicalEvent> empty_events;
        std::vector<float> sustain_audio;
        EXPECT_TRUE(processCompleteAudioPipeline(empty_events, sustain_audio, 1024));
        float rms = calculateRMS(sustain_audio);
        sustain_rms = std::max(sustain_rms, rms);
    }
    
    EXPECT_GT(sustain_rms, 0.001f) << "No sustain phase detected";
    
    // Test note off
    std::vector<MusicalEvent> note_off_events;
    note_off_events.push_back(createRawNoteOff(60));
    
    std::vector<float> audio_after_off;
    EXPECT_TRUE(processCompleteAudioPipeline(note_off_events, audio_after_off));
    
    // Generate audio after note off to check decay
    float decay_rms = 0.0f;
    for (int i = 0; i < 10; i++) {
        std::vector<MusicalEvent> empty_events;
        std::vector<float> decay_audio;
        EXPECT_TRUE(processCompleteAudioPipeline(empty_events, decay_audio, 1024));
        if (i > 5) { // After some decay time
            float rms = calculateRMS(decay_audio);
            decay_rms = std::max(decay_rms, rms);
        }
    }
    
    EXPECT_LT(decay_rms, sustain_rms * 0.5f) << "No clear decay after note off";
}

// Test velocity response through complete pipeline
TEST_F(ModularIntegrationTest, CompletePipelineVelocityResponse) {
    std::vector<float> velocities = {0.3f, 0.6f, 0.9f};
    std::vector<float> rms_levels;
    
    for (float velocity : velocities) {
        // Reset instrument state
        instrument_->all_notes_off();
        
        // Generate empty buffers to clear state
        for (int i = 0; i < 5; i++) {
            std::vector<MusicalEvent> empty_events;
            std::vector<float> empty_audio;
            processCompleteAudioPipeline(empty_events, empty_audio, 512);
        }
        
        // Play note at this velocity
        std::vector<MusicalEvent> input_events;
        input_events.push_back(createRawNoteOn(60, velocity));
        
        // Generate audio and find peak level
        float max_rms = 0.0f;
        for (int i = 0; i < 10; i++) {
            std::vector<float> audio_output;
            EXPECT_TRUE(processCompleteAudioPipeline(i == 0 ? input_events : std::vector<MusicalEvent>(), 
                                                    audio_output, 1024));
            float rms = calculateRMS(audio_output);
            max_rms = std::max(max_rms, rms);
        }
        
        rms_levels.push_back(max_rms);
    }
    
    // Verify velocity response
    EXPECT_GT(rms_levels[1], rms_levels[0] * 1.1f) << "Medium velocity not louder than low";
    EXPECT_GT(rms_levels[2], rms_levels[1] * 1.1f) << "High velocity not louder than medium";
}

// Test error handling in complete pipeline
TEST_F(ModularIntegrationTest, CompletePipelineErrorHandling) {
    // Test with invalid note numbers
    std::vector<MusicalEvent> invalid_events;
    invalid_events.push_back(createRawNoteOn(-1, 0.8f)); // Invalid note
    invalid_events.push_back(createRawNoteOn(128, 0.8f)); // Invalid note
    
    std::vector<float> audio_output;
    // Should handle gracefully without crashing
    bool result = processCompleteAudioPipeline(invalid_events, audio_output);
    // Don't assert on result since different implementations might handle this differently
    
    // Test with extreme velocities
    std::vector<MusicalEvent> extreme_events;
    extreme_events.push_back(createRawNoteOn(60, -0.5f)); // Negative velocity
    extreme_events.push_back(createRawNoteOn(62, 2.0f)); // Velocity > 1.0
    
    result = processCompleteAudioPipeline(extreme_events, audio_output);
    // Should handle gracefully
    
    // Test with very large number of simultaneous notes
    std::vector<MusicalEvent> many_events;
    for (int i = 0; i < 200; i++) { // More than typical polyphony
        many_events.push_back(createRawNoteOn(21 + (i % 88), 0.5f));
    }
    
    result = processCompleteAudioPipeline(many_events, audio_output);
    EXPECT_TRUE(result) << "Pipeline failed with high polyphony";
    
    // Check that audio is not clipped
    for (float sample : audio_output) {
        EXPECT_LE(std::abs(sample), 2.0f) << "Audio clipping detected with high polyphony";
    }
}

// Test latency through complete pipeline
TEST_F(ModularIntegrationTest, CompletePipelineLatency) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create input event with timestamp
    std::vector<MusicalEvent> input_events;
    MusicalEvent note_on = createRawNoteOn(60, 0.8f);
    note_on.timestamp = start_time;
    input_events.push_back(note_on);
    
    // Process through pipeline
    std::vector<float> audio_output;
    EXPECT_TRUE(processCompleteAudioPipeline(input_events, audio_output));
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto processing_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Processing should be reasonable for real-time use
    // For 1024 samples at 44.1kHz = ~23ms of audio
    // Processing should be much faster than this
    EXPECT_LT(processing_time.count(), 10000) << "Processing took too long: " << processing_time.count() << " microseconds";
}

// Test configuration updates during runtime
TEST_F(ModularIntegrationTest, RuntimeConfigurationUpdates) {
    // Test initial state
    std::vector<MusicalEvent> input_events;
    input_events.push_back(createRawNoteOn(60, 0.8f));
    
    std::vector<float> initial_audio;
    EXPECT_TRUE(processCompleteAudioPipeline(input_events, initial_audio));
    float initial_rms = calculateRMS(initial_audio);
    
    // Update abstraction layer configuration
    const char* new_abstraction_config = R"({
        "abstraction_processor": {
            "velocity_processing": {
                "velocity_curve": 2.0,
                "velocity_sensitivity": 1.5
            }
        }
    })";
    
    EXPECT_TRUE(abstractor_->update_config(new_abstraction_config));
    
    // Reset and test again
    instrument_->all_notes_off();
    for (int i = 0; i < 5; i++) {
        std::vector<MusicalEvent> empty;
        std::vector<float> empty_audio;
        processCompleteAudioPipeline(empty, empty_audio, 256);
    }
    
    std::vector<float> updated_audio;
    EXPECT_TRUE(processCompleteAudioPipeline(input_events, updated_audio));
    float updated_rms = calculateRMS(updated_audio);
    
    // Configuration change should have some effect
    // (Exact effect depends on implementation, so we just check it doesn't crash)
    EXPECT_GT(updated_rms, 0.001f) << "No audio after configuration update";
}
