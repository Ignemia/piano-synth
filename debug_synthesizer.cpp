#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <memory>
#include "core/synthesis/piano_synthesizer.h"
#include "core/abstraction/note_event.h"
#include "core/utils/config_manager.h"


using namespace PianoSynth;

int main() {
    std::cout << "🔍 Synthesizer Pipeline Debug Analysis" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // Create minimal config
    auto config = std::make_unique<Utils::ConfigManager>();
    config->setDouble("audio.sample_rate", 44100.0);
    config->setInt("synthesis.max_voices", 4);
    
    // Create synthesizer
    auto synthesizer = std::make_unique<Synthesis::PianoSynthesizer>();
    
    std::cout << "Initializing synthesizer..." << std::endl;
    if (!synthesizer->initialize(config.get())) {
        std::cout << "❌ Failed to initialize synthesizer" << std::endl;
        return 1;
    }
    
    // Create a note event
    Abstraction::NoteEvent note_on;
    note_on.type = Abstraction::NoteEvent::NOTE_ON;
    note_on.note_number = 60; // Middle C
    note_on.velocity = 0.8f;
    note_on.hammer_velocity = 5.0f;
    note_on.string_excitation = 3.0f;
    note_on.damper_position = 1.0f;
    note_on.sustain_pedal = false;
    note_on.soft_pedal = false;
    note_on.press_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "Processing note on event..." << std::endl;
    synthesizer->processNoteEvent(note_on);
    
    // Generate audio buffers and analyze each step
    const int buffer_size = 256;
    const int num_buffers = 5;
    
    std::vector<float> all_samples;
    
    for (int buf = 0; buf < num_buffers; ++buf) {
        std::cout << "\n--- Buffer " << (buf + 1) << " ---" << std::endl;
        
        auto buffer = synthesizer->generateAudioBuffer(buffer_size);
        
        // Analyze this buffer
        if (buffer.empty()) {
            std::cout << "❌ PROBLEM: Empty buffer returned!" << std::endl;
            continue;
        }
        
        float min_val = *std::min_element(buffer.begin(), buffer.end());
        float max_val = *std::max_element(buffer.begin(), buffer.end());
        float peak = std::max(std::abs(min_val), std::abs(max_val));
        
        double rms = 0.0;
        int zero_count = 0;
        int nan_count = 0;
        int inf_count = 0;
        
        for (float sample : buffer) {
            rms += sample * sample;
            if (sample == 0.0f) zero_count++;
            if (std::isnan(sample)) nan_count++;
            if (std::isinf(sample)) inf_count++;
        }
        rms = std::sqrt(rms / buffer.size());
        
        std::cout << "Buffer size: " << buffer.size() << " samples" << std::endl;
        std::cout << "Peak: " << peak << std::endl;
        std::cout << "RMS: " << rms << std::endl;
        std::cout << "Zero samples: " << zero_count << "/" << buffer.size() << std::endl;
        std::cout << "NaN samples: " << nan_count << std::endl;
        std::cout << "Inf samples: " << inf_count << std::endl;
        
        // Print first few samples to see patterns (stereo format)
        std::cout << "First 10 samples (L/R pairs): ";
        for (int i = 0; i < std::min(10, (int)buffer.size()); i += 2) {
            std::cout << "L:" << buffer[i];
            if (i + 1 < buffer.size()) {
                std::cout << "/R:" << buffer[i + 1] << " ";
            }
        }
        std::cout << std::endl;
        
        // Check if left and right channels are identical (bad)
        bool channels_identical = true;
        for (int i = 0; i < buffer.size() - 1; i += 2) {
            if (std::abs(buffer[i] - buffer[i + 1]) > 1e-6f) {
                channels_identical = false;
                break;
            }
        }
        
        if (channels_identical) {
            std::cout << "⚠️ WARNING: Left and right channels are identical!" << std::endl;
        }
        
        // Check for common problems
        if (peak == 0.0f) {
            std::cout << "❌ PROBLEM: Silent buffer!" << std::endl;
        } else if (peak < 1e-6f) {
            std::cout << "⚠️ WARNING: Very quiet audio" << std::endl;
        } else if (peak > 10.0f) {
            std::cout << "⚠️ WARNING: Very loud audio (possible overflow)" << std::endl;
        } else {
            std::cout << "✅ Audio level seems reasonable" << std::endl;
        }
        
        if (nan_count > 0 || inf_count > 0) {
            std::cout << "❌ CRITICAL: Invalid floating point values!" << std::endl;
        }
        
        if (zero_count == buffer.size()) {
            std::cout << "❌ PROBLEM: All samples are zero!" << std::endl;
        } else if (zero_count > buffer.size() * 0.9) {
            std::cout << "⚠️ WARNING: >90% samples are zero" << std::endl;
        }
        
        // Check for static/repeating patterns
        bool all_same = true;
        for (size_t i = 1; i < buffer.size(); ++i) {
            if (std::abs(buffer[i] - buffer[0]) > 1e-9f) {
                all_same = false;
                break;
            }
        }
        
        if (all_same && buffer[0] != 0.0f) {
            std::cout << "❌ PROBLEM: All samples identical (static DC)!" << std::endl;
        }
        
        // Check for obvious digital artifacts
        int clipped_samples = 0;
        for (float sample : buffer) {
            if (std::abs(sample) >= 0.99f) clipped_samples++;
        }
        
        if (clipped_samples > 0) {
            std::cout << "⚠️ WARNING: " << clipped_samples << " clipped samples" << std::endl;
        }
        
        all_samples.insert(all_samples.end(), buffer.begin(), buffer.end());
    }
    
    // Overall analysis
    std::cout << "\n📊 OVERALL ANALYSIS" << std::endl;
    std::cout << "===================" << std::endl;
    
    if (all_samples.empty()) {
        std::cout << "❌ CRITICAL: No audio generated!" << std::endl;
        return 1;
    }
    
    float overall_peak = 0.0f;
    double overall_rms = 0.0;
    int overall_zeros = 0;
    
    for (float sample : all_samples) {
        overall_peak = std::max(overall_peak, std::abs(sample));
        overall_rms += sample * sample;
        if (sample == 0.0f) overall_zeros++;
    }
    overall_rms = std::sqrt(overall_rms / all_samples.size());
    
    std::cout << "Total samples: " << all_samples.size() << std::endl;
    std::cout << "Overall peak: " << overall_peak << std::endl;
    std::cout << "Overall RMS: " << overall_rms << std::endl;
    std::cout << "Zero percentage: " << (100.0 * overall_zeros / all_samples.size()) << "%" << std::endl;
    
    // Save raw audio for analysis
    std::ofstream debug_file("synthesizer_debug_output.txt");
    debug_file << "# Synthesizer pipeline raw output\n";
    debug_file << "# Sample_number\tAmplitude\n";
    for (size_t i = 0; i < all_samples.size(); ++i) {
        debug_file << i << "\t" << all_samples[i] << "\n";
    }
    debug_file.close();
    
    // Create a simple WAV file for listening
    std::ofstream wav_file("debug_output.wav", std::ios::binary);
    
    // WAV header
    struct {
        char chunk_id[4] = {'R', 'I', 'F', 'F'};
        uint32_t chunk_size;
        char format[4] = {'W', 'A', 'V', 'E'};
        char subchunk1_id[4] = {'f', 'm', 't', ' '};
        uint32_t subchunk1_size = 16;
        uint16_t audio_format = 1; // PCM
        uint16_t num_channels = 1; // Mono for debugging
        uint32_t sample_rate = 44100;
        uint32_t byte_rate = 44100 * 1 * 2;
        uint16_t block_align = 2;
        uint16_t bits_per_sample = 16;
        char subchunk2_id[4] = {'d', 'a', 't', 'a'};
        uint32_t subchunk2_size;
    } header;
    
    header.subchunk2_size = all_samples.size() * 2;
    header.chunk_size = 36 + header.subchunk2_size;
    
    wav_file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    // Write mono audio data (take left channel only)
    for (size_t i = 0; i < all_samples.size(); i += 2) {
        int16_t sample = static_cast<int16_t>(std::clamp(all_samples[i] * 32767.0f, -32767.0f, 32767.0f));
        wav_file.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
    }
    wav_file.close();
    
    std::cout << "\n📁 Debug files created:" << std::endl;
    std::cout << "  - synthesizer_debug_output.txt (raw data)" << std::endl;
    std::cout << "  - debug_output.wav (mono audio for listening)" << std::endl;
    
    // Final diagnosis
    std::cout << "\n🎯 DIAGNOSIS:" << std::endl;
    std::cout << "=============" << std::endl;
    
    if (overall_peak == 0.0f) {
        std::cout << "❌ SILENT OUTPUT: Synthesizer produces no sound" << std::endl;
        std::cout << "   Possible causes:" << std::endl;
        std::cout << "   - Voice not being activated properly" << std::endl;
        std::cout << "   - String model not being excited" << std::endl;
        std::cout << "   - Gain/scaling issues setting output to zero" << std::endl;
    } else if (overall_peak < 0.001f) {
        std::cout << "⚠️ VERY QUIET: Output level extremely low" << std::endl;
        std::cout << "   Possible causes:" << std::endl;
        std::cout << "   - Insufficient excitation force" << std::endl;
        std::cout << "   - Excessive damping" << std::endl;
        std::cout << "   - Low gain in audio pipeline" << std::endl;
    } else if (overall_zeros > all_samples.size() * 0.8) {
        std::cout << "⚠️ MOSTLY SILENT: >80% of samples are zero" << std::endl;
        std::cout << "   This suggests intermittent output or gating issues" << std::endl;
    } else {
        std::cout << "✅ AUDIO PRESENT: Synthesizer is producing sound" << std::endl;
        
        // Check if it sounds like static
        double variance = 0.0;
        double mean = overall_rms * overall_rms; // Use power as proxy for mean
        int sign_changes = 0;
        
        for (size_t i = 1; i < all_samples.size(); ++i) {
            if ((all_samples[i] > 0) != (all_samples[i-1] > 0)) {
                sign_changes++;
            }
        }
        
        double sign_change_rate = double(sign_changes) / all_samples.size();
        
        std::cout << "   Sign change rate: " << sign_change_rate << std::endl;
        
        if (sign_change_rate > 0.4) {
            std::cout << "   ⚠️ HIGH FREQUENCY CONTENT: Might sound harsh/static-like" << std::endl;
            std::cout << "   Possible causes:" << std::endl;
            std::cout << "   - Numerical instability creating noise" << std::endl;
            std::cout << "   - Insufficient anti-aliasing filtering" << std::endl;
            std::cout << "   - Aliasing from high-frequency harmonics" << std::endl;
        } else if (sign_change_rate < 0.05) {
            std::cout << "   ⚠️ LOW FREQUENCY CONTENT: Might sound dull or wrong pitch" << std::endl;
        } else {
            std::cout << "   ✅ Frequency content appears reasonable" << std::endl;
        }
    }
    
    std::cout << "\n🎧 Listen to debug_output.wav to verify sound quality" << std::endl;
    
    return 0;
}