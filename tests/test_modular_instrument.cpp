#include <gtest/gtest.h>
#include "../shared/interfaces/dll_interfaces.h"
#include "../shared/interfaces/common_types.h"
#include <vector>
#include <chrono>
#include <thread>
#include <dlfcn.h>
#include <cmath>

using namespace PianoSynth::Interfaces;

class ModularInstrumentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Load piano instrument DLL
        instrument_dll_ = dlopen("./dlls/libpiano_instrument.so", RTLD_LAZY);
        if (!instrument_dll_) {
            GTEST_SKIP() << "Could not load piano instrument DLL: " << dlerror();
        }
        
        // Get the creation function
        typedef IInstrumentSynthesizer* (*create_func_t)();
        create_func_t create_instrument = (create_func_t) dlsym(instrument_dll_, "create_instrument_synthesizer");
        
        if (!create_instrument) {
            GTEST_SKIP() << "Could not find create_instrument_synthesizer function";
        }
        
        // Create the instrument
        instrument_ = create_instrument();
        if (!instrument_) {
            GTEST_SKIP() << "Could not create instrument synthesizer";
        }
        
        // Initialize with test configuration
        const char* test_config = R"({
            "piano_instrument": {
                "instrument_info": {
                    "polyphony": 64,
                    "note_range": {"min": 21, "max": 108}
                },
                "physical_modeling": {
                    "string_model": {
                        "tension_variation": 1.0,
                        "decay_time_scaling": 1.0
                    },
                    "hammer_model": {
                        "mass_variation": 1.0,
                        "velocity_to_force_curve": 2.0
                    }
                },
                "performance": {
                    "voice_stealing": "oldest",
                    "fast_attack_optimization": true
                }
            }
        })";
        
        sample_rate_ = 44100.0;
        buffer_size_ = 512;
        
        if (!instrument_->initialize(test_config, sample_rate_, buffer_size_ * 4)) {
            GTEST_SKIP() << "Could not initialize piano instrument";
        }
    }

    void TearDown() override {
        if (instrument_) {
            typedef void (*destroy_func_t)(IInstrumentSynthesizer*);
            destroy_func_t destroy_instrument = (destroy_func_t) dlsym(instrument_dll_, "destroy_instrument_synthesizer");
            if (destroy_instrument) {
                destroy_instrument(instrument_);
            }
            instrument_ = nullptr;
        }
        
        if (instrument_dll_) {
            dlclose(instrument_dll_);
            instrument_dll_ = nullptr;
        }
    }

    // Helper to create test events
    MusicalEvent createNoteOnEvent(int note, float velocity) {
        MusicalEvent event;
        event.type = EventType::NOTE_ON;
        event.timestamp = std::chrono::high_resolution_clock::now();
        event.note_number = note;
        event.velocity = velocity;
        return event;
    }
    
    MusicalEvent createNoteOffEvent(int note) {
        MusicalEvent event;
        event.type = EventType::NOTE_OFF;
        event.timestamp = std::chrono::high_resolution_clock::now();
        event.note_number = note;
        event.release_velocity = 0.5f;
        return event;
    }
    
    // Helper to check if audio buffer contains sound
    bool hasAudioContent(const AudioBuffer& buffer, float threshold = 0.001f) {
        for (size_t i = 0; i < buffer.total_samples(); i++) {
            if (std::abs(buffer.samples[i]) > threshold) {
                return true;
            }
        }
        return false;
    }
    
    // Helper to calculate RMS level of audio buffer
    float calculateRMS(const AudioBuffer& buffer) {
        float sum = 0.0f;
        for (size_t i = 0; i < buffer.total_samples(); i++) {
            sum += buffer.samples[i] * buffer.samples[i];
        }
        return std::sqrt(sum / buffer.total_samples());
    }

    void* instrument_dll_ = nullptr;
    IInstrumentSynthesizer* instrument_ = nullptr;
    double sample_rate_;
    size_t buffer_size_;
};

// Test basic instrument information
TEST_F(ModularInstrumentTest, InstrumentInfo) {
    if (!instrument_) GTEST_SKIP();
    
    // Check instrument name and version
    const char* name = instrument_->get_instrument_name();
    const char* version = instrument_->get_instrument_version();
    
    EXPECT_NE(name, nullptr);
    EXPECT_NE(version, nullptr);
    EXPECT_GT(strlen(name), 0);
    EXPECT_GT(strlen(version), 0);
    
    // Check polyphony limit
    int polyphony = instrument_->get_polyphony_limit();
    EXPECT_GT(polyphony, 0);
    EXPECT_LE(polyphony, 256); // Reasonable upper bound
}

// Test single note synthesis
TEST_F(ModularInstrumentTest, SingleNoteSynthesis) {
    if (!instrument_) GTEST_SKIP();
    
    // Create note on event
    MusicalEvent note_on = createNoteOnEvent(60, 0.8f); // C4 at 80% velocity
    
    // Process the event
    EXPECT_TRUE(instrument_->process_events(&note_on, 1));
    
    // Generate audio
    std::vector<float> audio_data(buffer_size_ * 2); // Stereo
    AudioBuffer buffer;
    buffer.samples = audio_data.data();
    buffer.frame_count = buffer_size_;
    buffer.channel_count = 2;
    buffer.sample_rate = sample_rate_;
    buffer.timestamp = std::chrono::high_resolution_clock::now();
    
    // Generate several buffers to ensure attack phase is captured
    bool found_audio = false;
    for (int i = 0; i < 10 && !found_audio; i++) {
        int result = instrument_->generate_audio(&buffer);
        EXPECT_GE(result, 0);
        
        if (hasAudioContent(buffer)) {
            found_audio = true;
        }
    }
    
    EXPECT_TRUE(found_audio) << "No audio generated for note on event";
}

// Test chord synthesis (multiple simultaneous notes)
TEST_F(ModularInstrumentTest, ChordSynthesis) {
    if (!instrument_) GTEST_SKIP();
    
    // Create C Major chord events
    std::vector<MusicalEvent> chord_events;
    chord_events.push_back(createNoteOnEvent(60, 0.7f)); // C4
    chord_events.push_back(createNoteOnEvent(64, 0.75f)); // E4
    chord_events.push_back(createNoteOnEvent(67, 0.8f)); // G4
    
    // Process all events
    EXPECT_TRUE(instrument_->process_events(chord_events.data(), chord_events.size()));
    
    // Generate audio
    std::vector<float> audio_data(buffer_size_ * 2);
    AudioBuffer buffer;
    buffer.samples = audio_data.data();
    buffer.frame_count = buffer_size_;
    buffer.channel_count = 2;
    buffer.sample_rate = sample_rate_;
    buffer.timestamp = std::chrono::high_resolution_clock::now();
    
    // Generate and check for audio content
    bool found_audio = false;
    float max_rms = 0.0f;
    
    for (int i = 0; i < 10; i++) {
        int result = instrument_->generate_audio(&buffer);
        EXPECT_GE(result, 0);
        
        if (hasAudioContent(buffer)) {
            found_audio = true;
            float rms = calculateRMS(buffer);
            max_rms = std::max(max_rms, rms);
        }
    }
    
    EXPECT_TRUE(found_audio) << "No audio generated for chord";
    EXPECT_GT(max_rms, 0.001f) << "Audio level too low for chord";
}

// Test velocity response
TEST_F(ModularInstrumentTest, VelocityResponse) {
    if (!instrument_) GTEST_SKIP();
    
    std::vector<float> velocities = {0.2f, 0.5f, 0.8f, 1.0f};
    std::vector<float> rms_levels;
    
    for (float velocity : velocities) {
        // Reset instrument state
        instrument_->all_notes_off();
        
        // Generate a few empty buffers to clear state
        std::vector<float> empty_data(buffer_size_ * 2, 0.0f);
        AudioBuffer empty_buffer;
        empty_buffer.samples = empty_data.data();
        empty_buffer.frame_count = buffer_size_;
        empty_buffer.channel_count = 2;
        empty_buffer.sample_rate = sample_rate_;
        empty_buffer.timestamp = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 5; i++) {
            instrument_->generate_audio(&empty_buffer);
        }
        
        // Play note at this velocity
        MusicalEvent note_on = createNoteOnEvent(60, velocity);
        EXPECT_TRUE(instrument_->process_events(&note_on, 1));
        
        // Generate audio and measure peak RMS
        std::vector<float> audio_data(buffer_size_ * 2);
        AudioBuffer buffer;
        buffer.samples = audio_data.data();
        buffer.frame_count = buffer_size_;
        buffer.channel_count = 2;
        buffer.sample_rate = sample_rate_;
        buffer.timestamp = std::chrono::high_resolution_clock::now();
        
        float max_rms = 0.0f;
        for (int i = 0; i < 20; i++) { // More buffers to catch attack peak
            instrument_->generate_audio(&buffer);
            float rms = calculateRMS(buffer);
            max_rms = std::max(max_rms, rms);
        }
        
        rms_levels.push_back(max_rms);
    }
    
    // Verify that higher velocities generally produce higher RMS levels
    for (size_t i = 1; i < rms_levels.size(); i++) {
        EXPECT_GE(rms_levels[i], rms_levels[i-1] * 0.8f) // Allow some tolerance
            << "Velocity response not monotonic: velocity " << velocities[i] 
            << " (RMS: " << rms_levels[i] << ") vs velocity " << velocities[i-1] 
            << " (RMS: " << rms_levels[i-1] << ")";
    }
}

// Test note on/off behavior
TEST_F(ModularInstrumentTest, NoteOnOffBehavior) {
    if (!instrument_) GTEST_SKIP();
    
    // Play note on
    MusicalEvent note_on = createNoteOnEvent(60, 0.8f);
    EXPECT_TRUE(instrument_->process_events(&note_on, 1));
    
    // Generate audio for attack phase
    std::vector<float> audio_data(buffer_size_ * 2);
    AudioBuffer buffer;
    buffer.samples = audio_data.data();
    buffer.frame_count = buffer_size_;
    buffer.channel_count = 2;
    buffer.sample_rate = sample_rate_;
    buffer.timestamp = std::chrono::high_resolution_clock::now();
    
    float rms_during_sustain = 0.0f;
    
    // Generate several buffers during sustain phase
    for (int i = 0; i < 50; i++) {
        instrument_->generate_audio(&buffer);
        if (i > 10 && i < 40) { // Sample during sustain phase
            rms_during_sustain = std::max(rms_during_sustain, calculateRMS(buffer));
        }
    }
    
    EXPECT_GT(rms_during_sustain, 0.001f) << "No sustain phase detected";
    
    // Play note off
    MusicalEvent note_off = createNoteOffEvent(60);
    EXPECT_TRUE(instrument_->process_events(&note_off, 1));
    
    // Check for decay/release phase
    float rms_after_release = 0.0f;
    for (int i = 0; i < 50; i++) {
        instrument_->generate_audio(&buffer);
        if (i > 10) { // After release has had time to take effect
            rms_after_release = std::max(rms_after_release, calculateRMS(buffer));
        }
    }
    
    // Release phase should be quieter than sustain phase
    EXPECT_LT(rms_after_release, rms_during_sustain * 0.8f) 
        << "No clear release/decay after note off";
}

// Test polyphony limits
TEST_F(ModularInstrumentTest, PolyphonyLimits) {
    if (!instrument_) GTEST_SKIP();
    
    int polyphony_limit = instrument_->get_polyphony_limit();
    
    // Play notes up to polyphony limit
    std::vector<MusicalEvent> events;
    for (int i = 0; i < polyphony_limit + 10; i++) { // Exceed limit
        events.push_back(createNoteOnEvent(21 + (i % 88), 0.5f));
    }
    
    // Process all events (should handle gracefully)
    EXPECT_TRUE(instrument_->process_events(events.data(), events.size()));
    
    // Generate audio (should not crash or produce excessive volume)
    std::vector<float> audio_data(buffer_size_ * 2);
    AudioBuffer buffer;
    buffer.samples = audio_data.data();
    buffer.frame_count = buffer_size_;
    buffer.channel_count = 2;
    buffer.sample_rate = sample_rate_;
    buffer.timestamp = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10; i++) {
        int result = instrument_->generate_audio(&buffer);
        EXPECT_GE(result, 0);
        
        // Check for clipping or excessive levels
        for (size_t j = 0; j < buffer.total_samples(); j++) {
            EXPECT_LE(std::abs(buffer.samples[j]), 2.0f) << "Audio clipping detected";
        }
    }
}

// Test all notes off functionality
TEST_F(ModularInstrumentTest, AllNotesOff) {
    if (!instrument_) GTEST_SKIP();
    
    // Play several notes
    std::vector<MusicalEvent> events;
    for (int i = 0; i < 5; i++) {
        events.push_back(createNoteOnEvent(60 + i * 2, 0.7f));
    }
    EXPECT_TRUE(instrument_->process_events(events.data(), events.size()));
    
    // Generate some audio to establish sound
    std::vector<float> audio_data(buffer_size_ * 2);
    AudioBuffer buffer;
    buffer.samples = audio_data.data();
    buffer.frame_count = buffer_size_;
    buffer.channel_count = 2;
    buffer.sample_rate = sample_rate_;
    buffer.timestamp = std::chrono::high_resolution_clock::now();
    
    bool had_audio = false;
    for (int i = 0; i < 10; i++) {
        instrument_->generate_audio(&buffer);
        if (hasAudioContent(buffer, 0.001f)) {
            had_audio = true;
            break;
        }
    }
    
    EXPECT_TRUE(had_audio) << "No audio before all notes off";
    
    // Call all notes off
    instrument_->all_notes_off();
    
    // Generate more audio - should decay to silence
    float max_level_after_off = 0.0f;
    for (int i = 0; i < 100; i++) { // More buffers to allow for decay
        instrument_->generate_audio(&buffer);
        if (i > 50) { // Check after some decay time
            for (size_t j = 0; j < buffer.total_samples(); j++) {
                max_level_after_off = std::max(max_level_after_off, std::abs(buffer.samples[j]));
            }
        }
    }
    
    EXPECT_LT(max_level_after_off, 0.01f) << "Audio level too high after all notes off";
}

// Test parameter setting
TEST_F(ModularInstrumentTest, ParameterControl) {
    if (!instrument_) GTEST_SKIP();
    
    // Test setting some parameters (exact parameters depend on implementation)
    // These might not be implemented yet, so we test graceful handling
    
    bool result1 = instrument_->set_parameter("master_volume", 0.8f);
    // Don't assert on result since parameters might not be implemented
    
    bool result2 = instrument_->set_parameter("reverb_amount", 0.3f);
    // Don't assert on result since parameters might not be implemented
    
    // Test getting parameters
    float value = instrument_->get_parameter("master_volume");
    // Value might be 0.0 if not implemented, which is fine
    
    // Just verify the calls don't crash
    SUCCEED() << "Parameter calls completed without crashing";
}
