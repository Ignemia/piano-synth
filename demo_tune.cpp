#include "core/utils/config_manager.h"
#include "core/utils/logger.h"
#include "core/abstraction/input_abstractor.h"
#include "core/synthesis/piano_synthesizer.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <cmath>

using namespace PianoSynth;

// Note definitions (MIDI note numbers)
const int C4 = 60, D4 = 62, E4 = 64, F4 = 65, G4 = 67, A4 = 69, B4 = 71;
const int C5 = 72, D5 = 74, E5 = 76;

// Simple note structure for our tune
struct Note {
    int midi_note;
    float velocity;
    double duration;  // in seconds
    
    Note(int note, float vel = 0.7f, double dur = 0.5) 
        : midi_note(note), velocity(vel), duration(dur) {}
};

// "Mary Had a Little Lamb" melody
std::vector<Note> mary_had_a_little_lamb = {
    Note(E4, 0.8f, 0.4),   // Ma-
    Note(D4, 0.7f, 0.4),   // ry
    Note(C4, 0.8f, 0.4),   // had
    Note(D4, 0.7f, 0.4),   // a
    Note(E4, 0.8f, 0.4),   // lit-
    Note(E4, 0.8f, 0.4),   // tle
    Note(E4, 0.9f, 0.8),   // lamb
    
    Note(D4, 0.7f, 0.4),   // lit-
    Note(D4, 0.7f, 0.4),   // tle
    Note(D4, 0.8f, 0.8),   // lamb
    
    Note(E4, 0.8f, 0.4),   // lit-
    Note(G4, 0.8f, 0.4),   // tle
    Note(G4, 0.9f, 0.8),   // lamb
    
    Note(E4, 0.8f, 0.4),   // Ma-
    Note(D4, 0.7f, 0.4),   // ry
    Note(C4, 0.8f, 0.4),   // had
    Note(D4, 0.7f, 0.4),   // a
    Note(E4, 0.8f, 0.4),   // lit-
    Note(E4, 0.8f, 0.4),   // tle
    Note(E4, 0.8f, 0.4),   // lamb
    Note(E4, 0.7f, 0.4),   // its
    Note(D4, 0.7f, 0.4),   // fleece
    Note(D4, 0.7f, 0.4),   // was
    Note(E4, 0.8f, 0.4),   // white
    Note(D4, 0.7f, 0.4),   // as
    Note(C4, 0.9f, 1.2)    // snow
};

class TunePlayer {
public:
    TunePlayer() : sample_rate_(44100), buffer_size_(512) {}
    
    bool initialize() {
        std::cout << "🎹 Piano Synthesizer Demo - Playing 'Mary Had a Little Lamb'" << std::endl;
        std::cout << "============================================================" << std::endl;
        
        // Create logger
        logger_ = std::make_unique<Utils::Logger>(Utils::LogLevel::INFO);
        logger_->setLogToConsole(true);
        
        // Create and configure config manager
        config_manager_ = std::make_unique<Utils::ConfigManager>();
        setupTestConfiguration();
        
        // Create input abstractor
        input_abstractor_ = std::make_unique<Abstraction::InputAbstractor>();
        input_abstractor_->initialize();
        
        // Create piano synthesizer
        piano_synthesizer_ = std::make_unique<Synthesis::PianoSynthesizer>();
        if (!piano_synthesizer_->initialize(config_manager_.get())) {
            std::cerr << "❌ Failed to initialize piano synthesizer" << std::endl;
            return false;
        }
        
        // Set reasonable synthesis parameters
        piano_synthesizer_->setMasterTuning(0.0f);
        piano_synthesizer_->setVelocitySensitivity(0.02f);
        piano_synthesizer_->setSoundboardResonance(0.7f);
        piano_synthesizer_->setRoomAcoustics(8.0, 0.3);
        
        std::cout << "✅ Piano synthesizer initialized successfully" << std::endl;
        return true;
    }
    
    void playTune(const std::vector<Note>& tune) {
        std::cout << "\n🎵 Starting playback..." << std::endl;
        
        // Prepare audio output file
        std::vector<float> complete_audio;
        
        for (size_t i = 0; i < tune.size(); ++i) {
            const Note& note = tune[i];
            
            std::cout << "♪ Playing note " << (i + 1) << "/" << tune.size() 
                      << " - MIDI " << note.midi_note 
                      << " (" << getMidiNoteName(note.midi_note) << ")"
                      << " velocity=" << note.velocity 
                      << " duration=" << note.duration << "s" << std::endl;
            
            // Create note on event
            auto note_on_event = createNoteOnEvent(note.midi_note, note.velocity);
            piano_synthesizer_->processNoteEvent(note_on_event);
            
            // Generate audio for most of the note duration
            double note_on_time = note.duration * 0.8; // 80% of duration with note on
            double note_off_time = note.duration * 0.2; // 20% of duration for release
            
            // Generate audio while note is on
            int note_on_samples = static_cast<int>(note_on_time * sample_rate_);
            int buffers_needed = (note_on_samples + buffer_size_ - 1) / buffer_size_;
            
            for (int buf = 0; buf < buffers_needed; ++buf) {
                auto audio_buffer = piano_synthesizer_->generateAudioBuffer(buffer_size_);
                complete_audio.insert(complete_audio.end(), audio_buffer.begin(), audio_buffer.end());
            }
            
            // Create note off event
            auto note_off_event = createNoteOffEvent(note.midi_note);
            piano_synthesizer_->processNoteEvent(note_off_event);
            
            // Generate audio for note release
            int note_off_samples = static_cast<int>(note_off_time * sample_rate_);
            int release_buffers = (note_off_samples + buffer_size_ - 1) / buffer_size_;
            
            for (int buf = 0; buf < release_buffers; ++buf) {
                auto audio_buffer = piano_synthesizer_->generateAudioBuffer(buffer_size_);
                complete_audio.insert(complete_audio.end(), audio_buffer.begin(), audio_buffer.end());
            }
            
            // Small pause between notes
            if (i < tune.size() - 1) {
                int pause_samples = static_cast<int>(0.05 * sample_rate_); // 50ms pause
                int pause_buffers = (pause_samples + buffer_size_ - 1) / buffer_size_;
                
                for (int buf = 0; buf < pause_buffers; ++buf) {
                    auto audio_buffer = piano_synthesizer_->generateAudioBuffer(buffer_size_);
                    complete_audio.insert(complete_audio.end(), audio_buffer.begin(), audio_buffer.end());
                }
            }
        }
        
        // Add some final reverb tail
        std::cout << "🎵 Adding reverb tail..." << std::endl;
        for (int i = 0; i < 50; ++i) {
            auto audio_buffer = piano_synthesizer_->generateAudioBuffer(buffer_size_);
            complete_audio.insert(complete_audio.end(), audio_buffer.begin(), audio_buffer.end());
        }
        
        // Save audio to file
        saveAudioToWav(complete_audio, "mary_had_a_little_lamb.wav");
        
        std::cout << "✅ Playback complete!" << std::endl;
        std::cout << "📁 Audio saved to: mary_had_a_little_lamb.wav" << std::endl;
        
        // Print audio statistics
        printAudioStats(complete_audio);
    }
    
private:
    void setupTestConfiguration() {
        // Setup optimized configuration for demo
        config_manager_->setDouble("audio.sample_rate", sample_rate_);
        config_manager_->setInt("audio.buffer_size", buffer_size_);
        config_manager_->setInt("audio.channels", 2);
        
        // Synthesis settings
        config_manager_->setInt("synthesis.max_voices", 16);
        config_manager_->setFloat("synthesis.master_volume", 0.8f);
        config_manager_->setDouble("synthesis.velocity_sensitivity", 0.02);
        
        // String physics - lighter settings for demo
        config_manager_->setDouble("string.tension_base", 800.0);
        config_manager_->setDouble("string.damping", 0.003);
        config_manager_->setDouble("string.stiffness", 5e-6);
        
        // Resonance settings
        config_manager_->setInt("resonance.max_harmonics", 16);
        config_manager_->setDouble("resonance.harmonic_decay", 0.85);
        config_manager_->setDouble("resonance.sympathetic_resonance", 0.05);
        
        // Room acoustics
        config_manager_->setDouble("room.size", 8.0);
        config_manager_->setDouble("room.reverb_time", 1.2);
        config_manager_->setDouble("room.damping", 0.25);
    }
    
    Abstraction::NoteEvent createNoteOnEvent(int note_number, float velocity) {
        Abstraction::NoteEvent event;
        event.type = Abstraction::NoteEvent::NOTE_ON;
        event.note_number = note_number;
        event.velocity = velocity;
        event.hammer_velocity = velocity * 3.5f;
        event.string_excitation = velocity * velocity * 2.5f;
        event.damper_position = 1.0f;
        event.sustain_pedal = false;
        event.soft_pedal = false;
        event.sostenuto_pedal = false;
        event.pitch_bend = 0.0f;
        event.aftertouch = 0.0f;
        event.press_time = std::chrono::high_resolution_clock::now();
        
        return event;
    }
    
    Abstraction::NoteEvent createNoteOffEvent(int note_number) {
        Abstraction::NoteEvent event;
        event.type = Abstraction::NoteEvent::NOTE_OFF;
        event.note_number = note_number;
        event.release_velocity = 0.5f;
        event.damper_position = 0.0f;
        event.sustain_pedal = false;
        event.soft_pedal = false;
        event.sostenuto_pedal = false;
        event.pitch_bend = 0.0f;
        event.release_time = std::chrono::high_resolution_clock::now();
        
        return event;
    }
    
    std::string getMidiNoteName(int midi_note) {
        const char* note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        int octave = (midi_note / 12) - 1;
        int note = midi_note % 12;
        return std::string(note_names[note]) + std::to_string(octave);
    }
    
    void saveAudioToWav(const std::vector<float>& audio_data, const std::string& filename) {
        // Simple WAV file writer
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "❌ Failed to open " << filename << " for writing" << std::endl;
            return;
        }
        
        // WAV header
        int channels = 2;
        int sample_rate = static_cast<int>(sample_rate_);
        int bytes_per_sample = 2; // 16-bit
        int frame_count = audio_data.size() / channels;
        int data_size = frame_count * channels * bytes_per_sample;
        int file_size = 36 + data_size;
        
        // RIFF header
        file.write("RIFF", 4);
        file.write(reinterpret_cast<const char*>(&file_size), 4);
        file.write("WAVE", 4);
        
        // Format chunk
        file.write("fmt ", 4);
        int fmt_chunk_size = 16;
        short audio_format = 1; // PCM
        short num_channels = channels;
        int byte_rate = sample_rate * channels * bytes_per_sample;
        short block_align = channels * bytes_per_sample;
        short bits_per_sample = bytes_per_sample * 8;
        
        file.write(reinterpret_cast<const char*>(&fmt_chunk_size), 4);
        file.write(reinterpret_cast<const char*>(&audio_format), 2);
        file.write(reinterpret_cast<const char*>(&num_channels), 2);
        file.write(reinterpret_cast<const char*>(&sample_rate), 4);
        file.write(reinterpret_cast<const char*>(&byte_rate), 4);
        file.write(reinterpret_cast<const char*>(&block_align), 2);
        file.write(reinterpret_cast<const char*>(&bits_per_sample), 2);
        
        // Data chunk
        file.write("data", 4);
        file.write(reinterpret_cast<const char*>(&data_size), 4);
        
        // Convert float samples to 16-bit PCM
        for (float sample : audio_data) {
            short pcm_sample = static_cast<short>(std::max(-1.0f, std::min(sample, 1.0f)) * 32767.0f);
            file.write(reinterpret_cast<const char*>(&pcm_sample), 2);
        }
        
        file.close();
        std::cout << "💾 WAV file saved: " << filename << std::endl;
    }
    
    void printAudioStats(const std::vector<float>& audio_data) {
        if (audio_data.empty()) return;
        
        float min_val = *std::min_element(audio_data.begin(), audio_data.end());
        float max_val = *std::max_element(audio_data.begin(), audio_data.end());
        float peak = std::max(std::abs(min_val), std::abs(max_val));
        
        double rms = 0.0;
        for (float sample : audio_data) {
            rms += sample * sample;
        }
        rms = std::sqrt(rms / audio_data.size());
        
        double duration = audio_data.size() / (sample_rate_ * 2); // Stereo
        
        std::cout << "\n📊 Audio Statistics:" << std::endl;
        std::cout << "   Duration: " << duration << " seconds" << std::endl;
        std::cout << "   Sample Rate: " << sample_rate_ << " Hz" << std::endl;
        std::cout << "   Channels: 2 (stereo)" << std::endl;
        std::cout << "   Total Samples: " << audio_data.size() << std::endl;
        std::cout << "   Peak Level: " << peak << " (" << (20.0 * std::log10(peak + 1e-10)) << " dB)" << std::endl;
        std::cout << "   RMS Level: " << rms << " (" << (20.0 * std::log10(rms + 1e-10)) << " dB)" << std::endl;
        std::cout << "   Dynamic Range: " << (20.0 * std::log10(peak / (rms + 1e-10))) << " dB" << std::endl;
    }
    
    std::unique_ptr<Utils::Logger> logger_;
    std::unique_ptr<Utils::ConfigManager> config_manager_;
    std::unique_ptr<Abstraction::InputAbstractor> input_abstractor_;
    std::unique_ptr<Synthesis::PianoSynthesizer> piano_synthesizer_;
    
    double sample_rate_;
    int buffer_size_;
};

int main() {
    std::cout << "🎼 Piano Synthesizer Demo Application" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "This demo plays 'Mary Had a Little Lamb' using the physical modeling" << std::endl;
    std::cout << "piano synthesizer, showcasing the complete audio pipeline:" << std::endl;
    std::cout << "  1. Note events → Input Abstraction Layer" << std::endl;
    std::cout << "  2. Abstracted events → Piano Synthesizer" << std::endl;
    std::cout << "  3. Physical modeling synthesis (strings, hammers, resonance)" << std::endl;
    std::cout << "  4. Audio generation → WAV file output" << std::endl;
    std::cout << std::endl;
    
    try {
        TunePlayer player;
        
        if (!player.initialize()) {
            std::cerr << "❌ Failed to initialize tune player" << std::endl;
            return 1;
        }
        
        std::cout << "🎯 Ready to play! Press Enter to start..." << std::endl;
        std::cin.get();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        player.playTune(mary_had_a_little_lamb);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "\n⏱️  Processing completed in " << duration.count() << " ms" << std::endl;
        std::cout << "🎉 Demo finished successfully!" << std::endl;
        std::cout << "\n💡 To listen to the generated audio:" << std::endl;
        std::cout << "   - Open 'mary_had_a_little_lamb.wav' in any audio player" << std::endl;
        std::cout << "   - The file contains the complete synthesized performance" << std::endl;
        std::cout << "   - You'll hear realistic piano sounds with physical modeling" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}