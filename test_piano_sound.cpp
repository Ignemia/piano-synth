#include "core/synthesis/piano_synthesizer.h"
#include "core/abstraction/note_event.h"
#include "core/utils/config_manager.h"
#include "core/utils/logger.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <fstream>

using namespace PianoSynth;

// WAV file header structure
struct WAVHeader {
    char chunk_id[4] = {'R', 'I', 'F', 'F'};
    uint32_t chunk_size;
    char format[4] = {'W', 'A', 'V', 'E'};
    char subchunk1_id[4] = {'f', 'm', 't', ' '};
    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1; // PCM
    uint16_t num_channels = 2; // Stereo
    uint32_t sample_rate = 44100;
    uint32_t byte_rate = 44100 * 2 * 2; // sample_rate * num_channels * bytes_per_sample
    uint16_t block_align = 4; // num_channels * bytes_per_sample
    uint16_t bits_per_sample = 16;
    char subchunk2_id[4] = {'d', 'a', 't', 'a'};
    uint32_t subchunk2_size;
};

void saveWAV(const std::string& filename, const std::vector<float>& audio_data, int sample_rate) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "❌ Could not create WAV file: " << filename << std::endl;
        return;
    }
    
    WAVHeader header;
    header.sample_rate = sample_rate;
    header.byte_rate = sample_rate * 2 * 2;
    header.subchunk2_size = audio_data.size() * 2; // 16-bit samples
    header.chunk_size = 36 + header.subchunk2_size;
    
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    // Convert float samples to 16-bit integers
    for (float sample : audio_data) {
        int16_t int_sample = static_cast<int16_t>(std::clamp(sample * 32767.0f, -32767.0f, 32767.0f));
        file.write(reinterpret_cast<const char*>(&int_sample), sizeof(int_sample));
    }
    
    file.close();
    std::cout << "✅ WAV file saved: " << filename << std::endl;
}

void analyzeAudio(const std::vector<float>& audio_data, const std::string& description) {
    if (audio_data.empty()) {
        std::cout << "❌ " << description << ": No audio data" << std::endl;
        return;
    }
    
    // Calculate statistics
    float min_val = *std::min_element(audio_data.begin(), audio_data.end());
    float max_val = *std::max_element(audio_data.begin(), audio_data.end());
    float peak = std::max(std::abs(min_val), std::abs(max_val));
    
    double rms = 0.0;
    for (float sample : audio_data) {
        rms += sample * sample;
    }
    rms = std::sqrt(rms / audio_data.size());
    
    // Check for problems
    bool has_nan = false, has_inf = false, has_dc_offset = false;
    double dc_sum = 0.0;
    
    for (float sample : audio_data) {
        if (std::isnan(sample)) has_nan = true;
        if (std::isinf(sample)) has_inf = true;
        dc_sum += sample;
    }
    
    double dc_offset = dc_sum / audio_data.size();
    has_dc_offset = std::abs(dc_offset) > 0.01;
    
    std::cout << "\n📊 " << description << " Analysis:" << std::endl;
    std::cout << "   Samples: " << audio_data.size() << std::endl;
    std::cout << "   Peak: " << peak << " (" << (20.0 * std::log10(peak + 1e-10)) << " dB)" << std::endl;
    std::cout << "   RMS: " << rms << " (" << (20.0 * std::log10(rms + 1e-10)) << " dB)" << std::endl;
    std::cout << "   DC Offset: " << dc_offset << std::endl;
    
    if (has_nan) std::cout << "   ⚠️  NaN values detected!" << std::endl;
    if (has_inf) std::cout << "   ⚠️  Infinite values detected!" << std::endl;
    if (has_dc_offset) std::cout << "   ⚠️  Significant DC offset detected!" << std::endl;
    if (peak > 1.0f) std::cout << "   ⚠️  Clipping detected (peak > 1.0)!" << std::endl;
    if (peak == 0.0f) std::cout << "   ⚠️  Silent audio!" << std::endl;
    if (peak < 0.001f) std::cout << "   ⚠️  Very quiet audio!" << std::endl;
    
    if (!has_nan && !has_inf && peak > 0.001f && peak < 0.8f) {
        std::cout << "   ✅ Audio quality looks good!" << std::endl;
    }
}

int main() {
    std::cout << "🎹 Piano Sound Quality Test" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << "Testing improved piano synthesizer for realistic sound..." << std::endl;
    
    // Create logger with debug output
    auto logger = std::make_unique<Utils::Logger>(Utils::LogLevel::INFO);
    logger->setLogToConsole(true);
    
    // Create optimized configuration
    auto config = std::make_unique<Utils::ConfigManager>();
    
    // Audio settings
    config->setDouble("audio.sample_rate", 44100.0);
    config->setInt("audio.buffer_size", 256);
    config->setFloat("audio.master_volume", 0.6f);
    
    // Improved physics settings
    config->setDouble("string.tension_base", 800.0);
    config->setDouble("string.damping", 0.015);
    config->setDouble("string.stiffness", 5e-6);
    config->setDouble("hammer.mass_scale", 0.8);
    config->setDouble("hammer.stiffness_scale", 0.5);
    config->setFloat("hammer.felt_hardness", 0.7f);
    
    // Synthesis settings
    config->setInt("synthesis.max_voices", 8);
    config->setFloat("synthesis.velocity_sensitivity", 0.008f);
    config->setDouble("synthesis.note_off_fade_time", 0.25);
    
    std::cout << "📋 Configuration loaded with improved parameters" << std::endl;
    
    // Create and initialize synthesizer
    auto synthesizer = std::make_unique<Synthesis::PianoSynthesizer>();
    if (!synthesizer->initialize(config.get())) {
        std::cout << "❌ Failed to initialize synthesizer" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Synthesizer initialized successfully" << std::endl;
    
    // Test parameters
    const int sample_rate = 44100;
    const int buffer_size = 256;
    const double note_duration = 3.0; // seconds
    const int total_samples = static_cast<int>(note_duration * sample_rate);
    
    std::vector<float> audio_output;
    audio_output.reserve(total_samples);
    
    // Test different notes for range verification
    std::vector<int> test_notes = {48, 60, 72, 84}; // C3, C4 (middle), C5, C6
    std::vector<float> test_velocities = {0.3f, 0.6f, 0.9f};
    
    for (int note : test_notes) {
        for (float velocity : test_velocities) {
            std::cout << "\n🎵 Testing note " << note << " (velocity " << velocity << ")" << std::endl;
            
            // Reset synthesizer state
            synthesizer->shutdown();
            synthesizer->initialize(config.get());
            
            std::vector<float> note_audio;
            
            // Create note on event
            Abstraction::NoteEvent note_on;
            note_on.type = Abstraction::NoteEvent::NOTE_ON;
            note_on.note_number = note;
            note_on.velocity = velocity;
            note_on.hammer_velocity = velocity * 8.0f + 2.0f;
            note_on.string_excitation = velocity * 5.0f + 1.0f;
            note_on.damper_position = 1.0f;
            note_on.sustain_pedal = false;
            note_on.soft_pedal = false;
            note_on.press_time = std::chrono::high_resolution_clock::now();
            
            // Process note on
            synthesizer->processNoteEvent(note_on);
            
            // Generate audio for 1 second
            int samples_per_note = sample_rate; // 1 second per note
            int buffers_needed = (samples_per_note + buffer_size - 1) / buffer_size;
            
            for (int buffer = 0; buffer < buffers_needed; ++buffer) {
                auto audio_buffer = synthesizer->generateAudioBuffer(buffer_size);
                
                // Add to note audio
                note_audio.insert(note_audio.end(), audio_buffer.begin(), audio_buffer.end());
                
                // Release note at 0.8 seconds
                if (buffer == static_cast<int>(0.8 * buffers_needed) && note_on.type != Abstraction::NoteEvent::NOTE_OFF) {
                    Abstraction::NoteEvent note_off;
                    note_off.type = Abstraction::NoteEvent::NOTE_OFF;
                    note_off.note_number = note;
                    note_off.release_velocity = 0.5f;
                    note_off.release_time = std::chrono::high_resolution_clock::now();
                    synthesizer->processNoteEvent(note_off);
                }
            }
            
            // Analyze this note
            std::string note_desc = "Note " + std::to_string(note) + " vel " + std::to_string(velocity);
            analyzeAudio(note_audio, note_desc);
            
            // Add to main audio output with brief silence between notes
            audio_output.insert(audio_output.end(), note_audio.begin(), note_audio.end());
            
            // Add 0.2 seconds of silence
            int silence_samples = static_cast<int>(0.2 * sample_rate * 2); // Stereo
            audio_output.insert(audio_output.end(), silence_samples, 0.0f);
        }
    }
    
    // Overall analysis
    analyzeAudio(audio_output, "Complete Test Audio");
    
    // Save complete test to WAV file
    saveWAV("piano_sound_test.wav", audio_output, sample_rate);
    
    // Test chord for polyphony
    std::cout << "\n🎼 Testing polyphonic chord (C major)" << std::endl;
    
    synthesizer->shutdown();
    synthesizer->initialize(config.get());
    
    std::vector<int> chord_notes = {60, 64, 67}; // C-E-G
    std::vector<float> chord_audio;
    
    // Play chord
    for (int note : chord_notes) {
        Abstraction::NoteEvent note_on;
        note_on.type = Abstraction::NoteEvent::NOTE_ON;
        note_on.note_number = note;
        note_on.velocity = 0.7f;
        note_on.hammer_velocity = 8.0f;
        note_on.string_excitation = 4.0f;
        note_on.damper_position = 1.0f;
        note_on.sustain_pedal = false;
        note_on.soft_pedal = false;
        note_on.press_time = std::chrono::high_resolution_clock::now();
        
        synthesizer->processNoteEvent(note_on);
    }
    
    // Generate chord audio
    int chord_samples = sample_rate * 2; // 2 seconds
    int chord_buffers = (chord_samples + buffer_size - 1) / buffer_size;
    
    for (int buffer = 0; buffer < chord_buffers; ++buffer) {
        auto audio_buffer = synthesizer->generateAudioBuffer(buffer_size);
        chord_audio.insert(chord_audio.end(), audio_buffer.begin(), audio_buffer.end());
        
        // Release chord at 1.5 seconds
        if (buffer == static_cast<int>(0.75 * chord_buffers)) {
            for (int note : chord_notes) {
                Abstraction::NoteEvent note_off;
                note_off.type = Abstraction::NoteEvent::NOTE_OFF;
                note_off.note_number = note;
                note_off.release_velocity = 0.5f;
                note_off.release_time = std::chrono::high_resolution_clock::now();
                synthesizer->processNoteEvent(note_off);
            }
        }
    }
    
    analyzeAudio(chord_audio, "C Major Chord");
    saveWAV("piano_chord_test.wav", chord_audio, sample_rate);
    
    std::cout << "\n🎉 Piano sound test completed!" << std::endl;
    std::cout << "\n📁 Output files:" << std::endl;
    std::cout << "   • piano_sound_test.wav - Individual notes at different velocities" << std::endl;
    std::cout << "   • piano_chord_test.wav - C major chord demonstration" << std::endl;
    std::cout << "\n💡 Listen to these files to verify the sound quality improvements." << std::endl;
    std::cout << "The piano should now sound musical rather than static-like." << std::endl;
    
    return 0;
}