#include <gtest/gtest.h>
#include "../core/synthesis/piano_synthesizer.h"
#include "../core/abstraction/note_event.h"
#include "../core/utils/config_manager.h"
#include "../core/utils/constants.h"
#include <memory>
#include <chrono>
#include <cmath>

using namespace PianoSynth;
using namespace PianoSynth::Synthesis;
using namespace PianoSynth::Abstraction;
using namespace PianoSynth::Utils;

class PianoSynthesizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager = std::make_unique<ConfigManager>();
        piano_synthesizer = std::make_unique<PianoSynthesizer>();
        
        // Initialize with test configuration
        ASSERT_TRUE(piano_synthesizer->initialize(config_manager.get()));
    }
    
    void TearDown() override {
        piano_synthesizer->shutdown();
    }
    
    NoteEvent createNoteOnEvent(int note_number, float velocity = 0.5f) {
        NoteEvent event;
        event.type = NoteEvent::NOTE_ON;
        event.note_number = note_number;
        event.velocity = velocity;
        event.hammer_velocity = velocity * 3.0f;
        event.string_excitation = velocity * velocity * 2.0f;
        event.damper_position = 1.0f;
        event.sustain_pedal = false;
        event.soft_pedal = false;
        event.press_time = std::chrono::high_resolution_clock::now();
        return event;
    }
    
    NoteEvent createNoteOffEvent(int note_number, float release_velocity = 0.5f) {
        NoteEvent event;
        event.type = NoteEvent::NOTE_OFF;
        event.note_number = note_number;
        event.release_velocity = release_velocity;
        event.damper_position = 0.0f;
        event.sustain_pedal = false;
        event.soft_pedal = false;
        event.release_time = std::chrono::high_resolution_clock::now();
        return event;
    }
    
    NoteEvent createPedalEvent(bool sustain = true, bool soft = false) {
        NoteEvent event;
        event.type = NoteEvent::PEDAL_CHANGE;
        event.sustain_pedal = sustain;
        event.soft_pedal = soft;
        return event;
    }
    
    std::unique_ptr<ConfigManager> config_manager;
    std::unique_ptr<PianoSynthesizer> piano_synthesizer;
    
    static constexpr double EPSILON = 1e-6;
    static constexpr int TEST_BUFFER_SIZE = 512;
};

// Test basic initialization
TEST_F(PianoSynthesizerTest, BasicInitialization) {
    // Synthesizer should be initialized
    EXPECT_NE(piano_synthesizer.get(), nullptr);
    
    // Should be able to generate empty audio buffer
    auto buffer = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    EXPECT_EQ(buffer.size(), TEST_BUFFER_SIZE * Constants::CHANNELS);
    
    // Buffer should be silent initially
    for (float sample : buffer) {
        EXPECT_NEAR(sample, 0.0f, EPSILON);
    }
}

// Test single note on/off
TEST_F(PianoSynthesizerTest, SingleNoteOnOff) {
    const int test_note = 60; // Middle C
    
    // Play note
    auto note_on = createNoteOnEvent(test_note, 0.7f);
    piano_synthesizer->processNoteEvent(note_on);
    
    // Generate some audio
    auto buffer = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Should have some audio output
    bool has_audio = false;
    for (float sample : buffer) {
        if (std::abs(sample) > EPSILON) {
            has_audio = true;
            break;
        }
    }
    EXPECT_TRUE(has_audio);
    
    // Stop note
    auto note_off = createNoteOffEvent(test_note);
    piano_synthesizer->processNoteEvent(note_off);
    
    // Generate more audio buffers to let note fade
    for (int i = 0; i < 10; ++i) {
        buffer = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    }
    
    // Eventually should become silent
    bool is_silent = true;
    for (float sample : buffer) {
        if (std::abs(sample) > 0.01f) {
            is_silent = false;
            break;
        }
    }
    EXPECT_TRUE(is_silent);
}

// Test polyphonic playback
TEST_F(PianoSynthesizerTest, PolyphonicPlayback) {
    const std::vector<int> chord_notes = {60, 64, 67}; // C major chord
    
    // Play chord
    for (int note : chord_notes) {
        auto note_on = createNoteOnEvent(note, 0.6f);
        piano_synthesizer->processNoteEvent(note_on);
    }
    
    // Generate audio
    auto buffer = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Should have audio output
    bool has_audio = false;
    float max_amplitude = 0.0f;
    for (float sample : buffer) {
        float abs_sample = std::abs(sample);
        if (abs_sample > EPSILON) {
            has_audio = true;
        }
        max_amplitude = std::max(max_amplitude, abs_sample);
    }
    
    EXPECT_TRUE(has_audio);
    EXPECT_GT(max_amplitude, 0.0f);
    
    // Stop all notes
    for (int note : chord_notes) {
        auto note_off = createNoteOffEvent(note);
        piano_synthesizer->processNoteEvent(note_off);
    }
}

// Test velocity sensitivity
TEST_F(PianoSynthesizerTest, VelocitySensitivity) {
    const int test_note = 60;
    
    // Play note with low velocity
    auto note_on_soft = createNoteOnEvent(test_note, 0.2f);
    piano_synthesizer->processNoteEvent(note_on_soft);
    
    auto buffer_soft = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Stop note
    auto note_off = createNoteOffEvent(test_note);
    piano_synthesizer->processNoteEvent(note_off);
    
    // Wait for silence
    for (int i = 0; i < 20; ++i) {
        piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    }
    
    // Play note with high velocity
    auto note_on_loud = createNoteOnEvent(test_note, 0.9f);
    piano_synthesizer->processNoteEvent(note_on_loud);
    
    auto buffer_loud = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Calculate RMS levels
    float rms_soft = 0.0f, rms_loud = 0.0f;
    for (size_t i = 0; i < buffer_soft.size(); ++i) {
        rms_soft += buffer_soft[i] * buffer_soft[i];
        rms_loud += buffer_loud[i] * buffer_loud[i];
    }
    rms_soft = sqrt(rms_soft / buffer_soft.size());
    rms_loud = sqrt(rms_loud / buffer_loud.size());
    
    // Loud note should have higher RMS
    EXPECT_GT(rms_loud, rms_soft);
    
    piano_synthesizer->processNoteEvent(createNoteOffEvent(test_note));
}

// Test sustain pedal functionality
TEST_F(PianoSynthesizerTest, SustainPedal) {
    const int test_note = 60;
    
    // Press sustain pedal
    auto pedal_on = createPedalEvent(true, false);
    piano_synthesizer->processNoteEvent(pedal_on);
    
    // Play and release note quickly
    auto note_on = createNoteOnEvent(test_note, 0.6f);
    piano_synthesizer->processNoteEvent(note_on);
    
    // Generate a few buffers
    for (int i = 0; i < 5; ++i) {
        piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    }
    
    auto note_off = createNoteOffEvent(test_note);
    piano_synthesizer->processNoteEvent(note_off);
    
    // Note should continue playing due to sustain pedal
    auto buffer_sustained = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    bool still_playing = false;
    for (float sample : buffer_sustained) {
        if (std::abs(sample) > 0.01f) {
            still_playing = true;
            break;
        }
    }
    EXPECT_TRUE(still_playing);
    
    // Release sustain pedal
    auto pedal_off = createPedalEvent(false, false);
    piano_synthesizer->processNoteEvent(pedal_off);
    
    // Note should fade out
    for (int i = 0; i < 20; ++i) {
        piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    }
    
    auto buffer_released = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    bool is_silent = true;
    for (float sample : buffer_released) {
        if (std::abs(sample) > 0.01f) {
            is_silent = false;
            break;
        }
    }
    EXPECT_TRUE(is_silent);
}

// Test voice management and voice stealing
TEST_F(PianoSynthesizerTest, VoiceManagement) {
    const int max_voices = config_manager->getInt("synthesis.max_voices", Constants::MAX_VOICES);
    
    // Play more notes than available voices
    std::vector<int> many_notes;
    for (int i = 0; i < max_voices + 10; ++i) {
        int note = 21 + i; // Start from lowest piano note
        many_notes.push_back(note);
        
        auto note_on = createNoteOnEvent(note, 0.5f);
        piano_synthesizer->processNoteEvent(note_on);
    }
    
    // Generate audio
    auto buffer = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Should still produce audio (voice stealing should work)
    bool has_audio = false;
    for (float sample : buffer) {
        if (std::abs(sample) > EPSILON) {
            has_audio = true;
            break;
        }
    }
    EXPECT_TRUE(has_audio);
    
    // Stop all notes
    for (int note : many_notes) {
        auto note_off = createNoteOffEvent(note);
        piano_synthesizer->processNoteEvent(note_off);
    }
}

// Test parameter changes
TEST_F(PianoSynthesizerTest, ParameterChanges) {
    const int test_note = 60;
    
    // Test pedal damping
    piano_synthesizer->setPedalDamping(0.5f);
    
    auto note_on = createNoteOnEvent(test_note, 0.6f);
    piano_synthesizer->processNoteEvent(note_on);
    
    auto buffer1 = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Change string tension
    piano_synthesizer->setStringTension(1.5f);
    
    auto buffer2 = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Should still produce audio
    bool has_audio = false;
    for (float sample : buffer2) {
        if (std::abs(sample) > EPSILON) {
            has_audio = true;
            break;
        }
    }
    EXPECT_TRUE(has_audio);
    
    // Test master tuning
    piano_synthesizer->setMasterTuning(50.0f); // +50 cents
    
    auto buffer3 = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Should still produce audio
    has_audio = false;
    for (float sample : buffer3) {
        if (std::abs(sample) > EPSILON) {
            has_audio = true;
            break;
        }
    }
    EXPECT_TRUE(has_audio);
    
    auto note_off = createNoteOffEvent(test_note);
    piano_synthesizer->processNoteEvent(note_off);
}

// Test buffer size variations
TEST_F(PianoSynthesizerTest, BufferSizeVariations) {
    const int test_note = 60;
    
    auto note_on = createNoteOnEvent(test_note, 0.6f);
    piano_synthesizer->processNoteEvent(note_on);
    
    // Test different buffer sizes
    std::vector<int> buffer_sizes = {64, 128, 256, 512, 1024, 2048};
    
    for (int buffer_size : buffer_sizes) {
        auto buffer = piano_synthesizer->generateAudioBuffer(buffer_size);
        
        // Check correct buffer size
        EXPECT_EQ(buffer.size(), buffer_size * Constants::CHANNELS);
        
        // Should have audio content
        bool has_audio = false;
        for (float sample : buffer) {
            if (std::abs(sample) > EPSILON) {
                has_audio = true;
                break;
            }
        }
        EXPECT_TRUE(has_audio);
    }
    
    auto note_off = createNoteOffEvent(test_note);
    piano_synthesizer->processNoteEvent(note_off);
}

// Test note range coverage
TEST_F(PianoSynthesizerTest, NoteRangeCoverage) {
    // Test notes across entire piano range
    std::vector<int> test_notes = {21, 36, 48, 60, 72, 84, 96, 108}; // Various octaves
    
    for (int note : test_notes) {
        auto note_on = createNoteOnEvent(note, 0.6f);
        piano_synthesizer->processNoteEvent(note_on);
        
        auto buffer = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
        
        // Should produce audio for all notes in range
        bool has_audio = false;
        for (float sample : buffer) {
            if (std::abs(sample) > EPSILON) {
                has_audio = true;
                break;
            }
        }
        EXPECT_TRUE(has_audio) << "Note " << note << " should produce audio";
        
        auto note_off = createNoteOffEvent(note);
        piano_synthesizer->processNoteEvent(note_off);
        
        // Let note fade
        for (int i = 0; i < 10; ++i) {
            piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
        }
    }
}

// Test audio output quality
TEST_F(PianoSynthesizerTest, AudioOutputQuality) {
    const int test_note = 60;
    
    auto note_on = createNoteOnEvent(test_note, 0.7f);
    piano_synthesizer->processNoteEvent(note_on);
    
    auto buffer = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Check for clipping (should not exceed [-1.0, 1.0])
    for (float sample : buffer) {
        EXPECT_GE(sample, -1.0f);
        EXPECT_LE(sample, 1.0f);
    }
    
    // Check for NaN or infinite values
    for (float sample : buffer) {
        EXPECT_FALSE(std::isnan(sample));
        EXPECT_FALSE(std::isinf(sample));
    }
    
    // Check stereo output (should have left and right channels)
    bool left_has_audio = false, right_has_audio = false;
    for (size_t i = 0; i < buffer.size(); i += 2) {
        if (std::abs(buffer[i]) > EPSILON) left_has_audio = true;
        if (i + 1 < buffer.size() && std::abs(buffer[i + 1]) > EPSILON) right_has_audio = true;
    }
    EXPECT_TRUE(left_has_audio);
    EXPECT_TRUE(right_has_audio);
    
    auto note_off = createNoteOffEvent(test_note);
    piano_synthesizer->processNoteEvent(note_off);
}

// Test rapid note events
TEST_F(PianoSynthesizerTest, RapidNoteEvents) {
    const int test_note = 60;
    
    // Rapid note on/off events
    for (int i = 0; i < 10; ++i) {
        auto note_on = createNoteOnEvent(test_note, 0.6f);
        piano_synthesizer->processNoteEvent(note_on);
        
        piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
        
        auto note_off = createNoteOffEvent(test_note);
        piano_synthesizer->processNoteEvent(note_off);
        
        piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    }
    
    // Should handle rapid events without crashing
    auto final_buffer = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Check for stability
    for (float sample : final_buffer) {
        EXPECT_FALSE(std::isnan(sample));
        EXPECT_FALSE(std::isinf(sample));
        EXPECT_GE(sample, -1.0f);
        EXPECT_LE(sample, 1.0f);
    }
}

// Test soft pedal functionality
TEST_F(PianoSynthesizerTest, SoftPedal) {
    const int test_note = 60;
    
    // Play note without soft pedal
    auto note_on1 = createNoteOnEvent(test_note, 0.7f);
    piano_synthesizer->processNoteEvent(note_on1);
    
    auto buffer1 = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    auto note_off1 = createNoteOffEvent(test_note);
    piano_synthesizer->processNoteEvent(note_off1);
    
    // Wait for silence
    for (int i = 0; i < 20; ++i) {
        piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    }
    
    // Play note with soft pedal
    auto pedal_event = createPedalEvent(false, true); // soft pedal on
    piano_synthesizer->processNoteEvent(pedal_event);
    
    auto note_on2 = createNoteOnEvent(test_note, 0.7f);
    note_on2.soft_pedal = true;
    piano_synthesizer->processNoteEvent(note_on2);
    
    auto buffer2 = piano_synthesizer->generateAudioBuffer(TEST_BUFFER_SIZE);
    
    // Calculate RMS for comparison
    float rms1 = 0.0f, rms2 = 0.0f;
    for (size_t i = 0; i < buffer1.size(); ++i) {
        rms1 += buffer1[i] * buffer1[i];
        rms2 += buffer2[i] * buffer2[i];
    }
    rms1 = sqrt(rms1 / buffer1.size());
    rms2 = sqrt(rms2 / buffer2.size());
    
    // Soft pedal should generally reduce the brightness/harshness
    // Both should still produce audio
    EXPECT_GT(rms1, EPSILON);
    EXPECT_GT(rms2, EPSILON);
    
    auto note_off2 = createNoteOffEvent(test_note);
    piano_synthesizer->processNoteEvent(note_off2);
}