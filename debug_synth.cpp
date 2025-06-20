#include "core/synthesis/piano_synthesizer.h"
#include "core/abstraction/note_event.h"
#include "core/utils/config_manager.h"
#include "core/utils/logger.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>

using namespace PianoSynth;

int main() {
    std::cout << "🔍 Piano Synthesizer Debug - Testing Audio Generation" << std::endl;
    std::cout << "=====================================================" << std::endl;
    
    // Create logger
    auto logger = std::make_unique<Utils::Logger>(Utils::LogLevel::kDebug);
    logger->setLogToConsole(true);
    
    // Create config manager with minimal settings
    auto config = std::make_unique<Utils::ConfigManager>();
    config->setDouble("audio.sample_rate", 44100.0);
    config->setInt("synthesis.max_voices", 4);
    config->setFloat("synthesis.master_volume", 1.0f);
    
    // Simplified string physics for debugging
    config->setDouble("string.tension_base", 500.0);
    config->setDouble("string.damping", 0.01);
    config->setDouble("string.stiffness", 1e-6);
    config->setInt("resonance.max_harmonics", 8);
    
    std::cout << "📋 Configuration loaded" << std::endl;
    
    // Create synthesizer
    auto synthesizer = std::make_unique<Synthesis::PianoSynthesizer>();
    if (!synthesizer->initialize(config.get())) {
        std::cout << "❌ Failed to initialize synthesizer" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Synthesizer initialized" << std::endl;
    
    // Create a simple note event (Middle C)
    Abstraction::NoteEvent note_on;
    note_on.type = Abstraction::NoteEvent::NOTE_ON;
    note_on.note_number = 60; // Middle C
    note_on.velocity = 0.8f;
    note_on.hammer_velocity = 2.0f;
    note_on.string_excitation = 1.0f;
    note_on.damper_position = 1.0f;
    note_on.sustain_pedal = false;
    note_on.soft_pedal = false;
    note_on.press_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "🎵 Playing Middle C (MIDI 60) with velocity 0.8" << std::endl;
    
    // Process note on
    synthesizer->processNoteEvent(note_on);
    
    // Generate some audio buffers and analyze
    const int buffer_size = 512;
    const int num_buffers = 10;
    
    std::vector<float> all_samples;
    
    for (int i = 0; i < num_buffers; ++i) {
        auto buffer = synthesizer->generateAudioBuffer(buffer_size);
        
        // Analyze this buffer
        float min_val = *std::min_element(buffer.begin(), buffer.end());
        float max_val = *std::max_element(buffer.begin(), buffer.end());
        float peak = std::max(std::abs(min_val), std::abs(max_val));
        
        double rms = 0.0;
        for (float sample : buffer) {
            rms += sample * sample;
        }
        rms = std::sqrt(rms / buffer.size());
        
        std::cout << "Buffer " << (i+1) << ": Peak=" << peak << ", RMS=" << rms;
        
        // Check for problems
        bool has_nan = false, has_inf = false;
        for (float sample : buffer) {
            if (std::isnan(sample)) has_nan = true;
            if (std::isinf(sample)) has_inf = true;
        }
        
        if (has_nan) std::cout << " [NaN detected!]";
        if (has_inf) std::cout << " [Inf detected!]";
        if (peak > 1.0f) std::cout << " [Clipping!]";
        if (peak == 0.0f) std::cout << " [Silent]";
        
        std::cout << std::endl;
        
        // Save first few samples for inspection
        if (i == 0) {
            std::cout << "First 10 samples: ";
            for (int j = 0; j < std::min(10, (int)buffer.size()); ++j) {
                std::cout << buffer[j] << " ";
            }
            std::cout << std::endl;
        }
        
        all_samples.insert(all_samples.end(), buffer.begin(), buffer.end());
    }
    
    // Overall statistics
    float overall_peak = *std::max_element(all_samples.begin(), all_samples.end(), 
                                          [](float a, float b) { return std::abs(a) < std::abs(b); });
    overall_peak = std::abs(overall_peak);
    
    double overall_rms = 0.0;
    for (float sample : all_samples) {
        overall_rms += sample * sample;
    }
    overall_rms = std::sqrt(overall_rms / all_samples.size());
    
    std::cout << "\n📊 Overall Statistics:" << std::endl;
    std::cout << "   Total samples: " << all_samples.size() << std::endl;
    std::cout << "   Peak level: " << overall_peak << std::endl;
    std::cout << "   RMS level: " << overall_rms << std::endl;
    
    if (overall_peak > 0.0f) {
        std::cout << "   Peak dB: " << (20.0 * std::log10(overall_peak)) << " dB" << std::endl;
        std::cout << "   RMS dB: " << (20.0 * std::log10(overall_rms + 1e-10)) << " dB" << std::endl;
        std::cout << "✅ Audio generation working!" << std::endl;
    } else {
        std::cout << "❌ No audio output detected" << std::endl;
    }
    
    // Test note off
    std::cout << "\n🎵 Releasing note..." << std::endl;
    Abstraction::NoteEvent note_off;
    note_off.type = Abstraction::NoteEvent::NOTE_OFF;
    note_off.note_number = 60;
    note_off.release_velocity = 0.5f;
    note_off.release_time = std::chrono::high_resolution_clock::now();
    
    synthesizer->processNoteEvent(note_off);
    
    // Generate a few more buffers to see decay
    for (int i = 0; i < 5; ++i) {
        auto buffer = synthesizer->generateAudioBuffer(buffer_size);
        float peak = 0.0f;
        for (float sample : buffer) {
            peak = std::max(peak, std::abs(sample));
        }
        std::cout << "Release buffer " << (i+1) << ": Peak=" << peak << std::endl;
    }
    
    std::cout << "\n🎉 Debug complete!" << std::endl;
    
    return 0;
}