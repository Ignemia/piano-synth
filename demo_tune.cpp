#include "core/utils/logger.h"
#include "core/utils/wav_writer.h"
#include "shared/interfaces/dll_interfaces.h"
#include "instruments/piano/simple_oscillator.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <memory>

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
    TunePlayer() : sample_rate_(192000), buffer_size_(512) {}

    ~TunePlayer() {
        if (instrument_) {
            destroy_instrument_synthesizer(instrument_);
            instrument_ = nullptr;
        }
    }
    
    bool initialize() {
        std::cout << "🎹 Piano Synthesizer Demo - Playing 'Mary Had a Little Lamb'" << std::endl;
        std::cout << "============================================================" << std::endl;
        
        // Create logger
        logger_ = std::make_unique<Utils::Logger>(Utils::LogLevel::kInfo);
        logger_->setLogToConsole(true);
        
        instrument_ = create_instrument_synthesizer();
        if (!instrument_) {
            std::cerr << "❌ Failed to create oscillator instrument" << std::endl;
            return false;
        }

        if (!instrument_->initialize("{}", sample_rate_, buffer_size_ * 4)) {
            std::cerr << "❌ Failed to initialize oscillator" << std::endl;
            return false;
        }

        std::cout << "✅ Oscillator instrument initialized successfully" << std::endl;
        return true;
    }
    
    void playTune(const std::vector<Note>& tune) {
        std::cout << "\n🎵 Starting playback..." << std::endl;
        
        std::vector<float> complete_audio;

        Interfaces::AudioBuffer buffer;
        buffer.frame_count = buffer_size_;
        buffer.channel_count = 2;
        buffer.sample_rate = sample_rate_;
        std::vector<float> temp(buffer.total_samples());
        buffer.samples = temp.data();

        for (size_t i = 0; i < tune.size(); ++i) {
            const Note& note = tune[i];
            
            std::cout << "♪ Playing note " << (i + 1) << "/" << tune.size() 
                      << " - MIDI " << note.midi_note 
                      << " (" << getMidiNoteName(note.midi_note) << ")"
                      << " velocity=" << note.velocity 
                      << " duration=" << note.duration << "s" << std::endl;
            
            // Create note on event
            auto note_on_event = createNoteOnEvent(note.midi_note, note.velocity);
            instrument_->process_events(&note_on_event, 1);
            
            // Generate audio for most of the note duration
            double note_on_time = note.duration * 0.8; // 80% of duration with note on
            double note_off_time = note.duration * 0.2; // 20% of duration for release
            
            // Generate audio while note is on
            int note_on_samples = static_cast<int>(note_on_time * sample_rate_);
            int buffers_needed = (note_on_samples + buffer_size_ - 1) / buffer_size_;

            for (int buf = 0; buf < buffers_needed; ++buf) {
                instrument_->generate_audio(&buffer);
                complete_audio.insert(complete_audio.end(), temp.begin(), temp.end());
            }
            
            // Create note off event
            auto note_off_event = createNoteOffEvent(note.midi_note);
            instrument_->process_events(&note_off_event, 1);
            
            // Generate audio for note release
            int note_off_samples = static_cast<int>(note_off_time * sample_rate_);
            int release_buffers = (note_off_samples + buffer_size_ - 1) / buffer_size_;
            
            for (int buf = 0; buf < release_buffers; ++buf) {
                instrument_->generate_audio(&buffer);
                complete_audio.insert(complete_audio.end(), temp.begin(), temp.end());
            }
            
            // Small pause between notes
            if (i < tune.size() - 1) {
                int pause_samples = static_cast<int>(0.05 * sample_rate_); // 50ms pause
                int pause_buffers = (pause_samples + buffer_size_ - 1) / buffer_size_;

                for (int buf = 0; buf < pause_buffers; ++buf) {
                    instrument_->generate_audio(&buffer);
                    complete_audio.insert(complete_audio.end(), temp.begin(), temp.end());
                }
            }
        }

        std::cout << "🎵 Adding fade-out tail..." << std::endl;
        for (int i = 0; i < 50; ++i) {
            instrument_->generate_audio(&buffer);
            complete_audio.insert(complete_audio.end(), temp.begin(), temp.end());
        }

        auto smoothed = smoothAudio(complete_audio, 0.1f);
        Utils::WavWriter::write(smoothed, "mary_had_a_little_lamb.wav",
                                static_cast<int>(sample_rate_), 2, 64);
        
        std::cout << "✅ Playback complete!" << std::endl;
        std::cout << "📁 Audio saved to: mary_had_a_little_lamb.wav" << std::endl;
        
        // Print audio statistics
        printAudioStats(complete_audio);
    }
    
private:
    
    Interfaces::MusicalEvent createNoteOnEvent(int note_number, float velocity) {
        Interfaces::MusicalEvent event;
        event.type = Interfaces::EventType::NOTE_ON;
        event.timestamp = std::chrono::high_resolution_clock::now();
        event.note_number = note_number;
        event.velocity = velocity;
        
        return event;
    }

    Interfaces::MusicalEvent createNoteOffEvent(int note_number) {
        Interfaces::MusicalEvent event;
        event.type = Interfaces::EventType::NOTE_OFF;
        event.timestamp = std::chrono::high_resolution_clock::now();
        event.note_number = note_number;
        event.release_velocity = 0.5f;

        return event;
    }
    
    std::string getMidiNoteName(int midi_note) {
        const char* note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        int octave = (midi_note / 12) - 1;
        int note = midi_note % 12;
        return std::string(note_names[note]) + std::to_string(octave);
    }
    

    /**
     * @brief [AI GENERATED] Apply a simple smoothing filter to audio data.
     *
     * Implements a one-pole low-pass filter controlled by @p alpha to
     * reduce harsh edges in the generated waveform.
     */
    std::vector<float> smoothAudio(const std::vector<float>& input, float alpha) {
        std::vector<float> output(input.size());
        if (input.empty()) return output;
        output[0] = input[0];
        for (size_t i = 1; i < input.size(); ++i) {
            output[i] = alpha * input[i] + (1.0f - alpha) * output[i - 1];
        }
        return output;
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
    Interfaces::IInstrumentSynthesizer* instrument_ = nullptr;
    
    double sample_rate_;
    int buffer_size_;
};

int main() {
    std::cout << "🎼 Piano Synthesizer Demo Application" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "This demo plays 'Mary Had a Little Lamb' using a simple oscillator" << std::endl;
    std::cout << "instrument to demonstrate basic audio generation:" << std::endl;
    std::cout << "  1. Note events" << std::endl;
    std::cout << "  2. Oscillator synthesis" << std::endl;
    std::cout << "  3. Audio generation → WAV file output" << std::endl;
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
        std::cout << "   - You'll hear simple sine wave tones from the oscillator" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}