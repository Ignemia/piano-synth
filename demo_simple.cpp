#include "core/abstraction/input_abstractor.h"
#include "core/abstraction/note_event.h"
#include "core/utils/config_manager.h"
#include "core/utils/logger.h"
#include "core/utils/math_utils.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>

using namespace PianoSynth;

// Simple sine wave voice for demonstration
class SimpleSineVoice {
public:
    SimpleSineVoice() : active(false), note_number(60), frequency(440.0), 
                       amplitude(0.0), phase(0.0), release_time(0.0), 
                       age(0.0), note_off_received(false) {}
    
    void noteOn(int note, float velocity) {
        active = true;
        note_number = note;
        frequency = Utils::MathUtils::midiToFrequency(note);
        amplitude = velocity;
        phase = 0.0;
        age = 0.0;
        note_off_received = false;
        release_time = 0.0;
    }
    
    void noteOff() {
        note_off_received = true;
        release_time = age;
    }
    
    float generateSample(double sample_rate) {
        if (!active) return 0.0f;
        
        double dt = 1.0 / sample_rate;
        age += dt;
        
        // Generate sine wave
        float sample = amplitude * sin(2.0 * Utils::MathUtils::PI * frequency * phase);
        
        // Update phase
        phase += frequency * dt;
        if (phase >= 1.0) phase -= 1.0;
        
        // Apply envelope
        float envelope = 1.0f;
        
        // Attack (0.1 seconds)
        if (age < 0.1) {
            envelope = age / 0.1;
        }
        // Decay to sustain (0.2 seconds)
        else if (age < 0.3) {
            envelope = 1.0 - 0.3 * ((age - 0.1) / 0.2); // Decay to 70%
        }
        // Sustain
        else if (!note_off_received) {
            envelope = 0.7;
        }
        // Release (0.5 seconds)
        else {
            double release_age = age - release_time;
            if (release_age < 0.5) {
                envelope = 0.7 * (1.0 - release_age / 0.5);
            } else {
                envelope = 0.0;
                active = false;
            }
        }
        
        return sample * envelope;
    }
    
    bool active;
    int note_number;
    double frequency;
    float amplitude;
    double phase;
    double release_time;
    double age;
    bool note_off_received;
};

// Simple polyphonic synthesizer
class SimpleSynthesizer {
public:
    SimpleSynthesizer() : sample_rate_(44100.0), master_volume_(0.8f) {
        voices_.resize(16); // 16 voices
    }
    
    void setSampleRate(double sample_rate) {
        sample_rate_ = sample_rate;
    }
    
    void processNoteEvent(const Abstraction::NoteEvent& event) {
        if (event.type == Abstraction::NoteEvent::NOTE_ON) {
            // Find available voice or steal oldest
            SimpleSineVoice* voice = nullptr;
            for (auto& v : voices_) {
                if (!v.active) {
                    voice = &v;
                    break;
                }
            }
            
            if (!voice) {
                // Steal oldest voice
                double oldest_age = 0.0;
                for (auto& v : voices_) {
                    if (v.age > oldest_age) {
                        oldest_age = v.age;
                        voice = &v;
                    }
                }
            }
            
            if (voice) {
                voice->noteOn(event.note_number, event.velocity);
                std::cout << "  🎵 Voice activated: Note " << event.note_number 
                          << " (" << getMidiNoteName(event.note_number) << ")"
                          << " @ " << Utils::MathUtils::midiToFrequency(event.note_number) << " Hz" << std::endl;
            }
        }
        else if (event.type == Abstraction::NoteEvent::NOTE_OFF) {
            // Find active voice for this note
            for (auto& voice : voices_) {
                if (voice.active && voice.note_number == event.note_number) {
                    voice.noteOff();
                    std::cout << "  🎵 Voice released: Note " << event.note_number << std::endl;
                    break;
                }
            }
        }
    }
    
    std::vector<float> generateAudioBuffer(int buffer_size) {
        std::vector<float> buffer(buffer_size * 2, 0.0f); // Stereo
        
        for (int frame = 0; frame < buffer_size; ++frame) {
            float sample = 0.0f;
            
            // Mix all active voices
            for (auto& voice : voices_) {
                if (voice.active) {
                    sample += voice.generateSample(sample_rate_);
                }
            }
            
            // Apply master volume and soft limiting
            sample *= master_volume_;
            sample = std::max(-0.95f, std::min(sample, 0.95f));
            
            // Stereo (same signal to both channels)
            buffer[frame * 2] = sample;     // Left
            buffer[frame * 2 + 1] = sample; // Right
        }
        
        return buffer;
    }
    
    int getActiveVoiceCount() const {
        int count = 0;
        for (const auto& voice : voices_) {
            if (voice.active) count++;
        }
        return count;
    }
    
private:
    std::string getMidiNoteName(int midi_note) {
        const char* note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        int octave = (midi_note / 12) - 1;
        int note = midi_note % 12;
        return std::string(note_names[note]) + std::to_string(octave);
    }
    
    std::vector<SimpleSineVoice> voices_;
    double sample_rate_;
    float master_volume_;
};

// Note definitions
const int C4 = 60, D4 = 62, E4 = 64, F4 = 65, G4 = 67, A4 = 69, B4 = 71;

struct Note {
    int midi_note;
    float velocity;
    double duration;
    
    Note(int note, float vel = 0.7f, double dur = 0.5) 
        : midi_note(note), velocity(vel), duration(dur) {}
};

// Simple "Mary Had a Little Lamb" melody
std::vector<Note> simple_melody = {
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
};

class SimpleDemo {
public:
    SimpleDemo() : sample_rate_(44100), buffer_size_(512) {}
    
    bool initialize() {
        std::cout << "🎹 Simple Piano Synthesizer Demo" << std::endl;
        std::cout << "=================================" << std::endl;
        std::cout << "Demonstrating the complete audio pipeline with simplified synthesis:" << std::endl;
        std::cout << "  📥 Raw MIDI events → Input Abstraction Layer" << std::endl;
        std::cout << "  🎛️  Abstracted events → Simple Synthesizer" << std::endl;
        std::cout << "  🎵 Sine wave synthesis → Audio Buffer Generation" << std::endl;
        std::cout << "  💾 Audio output → WAV file" << std::endl;
        std::cout << std::endl;
        
        // Create input abstractor
        input_abstractor_ = std::make_unique<Abstraction::InputAbstractor>();
        input_abstractor_->initialize();
        
        // Create simple synthesizer
        synthesizer_ = std::make_unique<SimpleSynthesizer>();
        synthesizer_->setSampleRate(sample_rate_);
        
        std::cout << "✅ Components initialized successfully" << std::endl;
        return true;
    }
    
    void playMelody() {
        std::cout << "\n🎵 Playing 'Mary Had a Little Lamb'..." << std::endl;
        std::cout << "Melody has " << simple_melody.size() << " notes" << std::endl;
        
        std::vector<float> complete_audio;
        
        for (size_t i = 0; i < simple_melody.size(); ++i) {
            const Note& note = simple_melody[i];
            
            std::cout << "\n♪ Note " << (i + 1) << "/" << simple_melody.size() 
                      << " - Playing " << getMidiNoteName(note.midi_note) 
                      << " (MIDI " << note.midi_note << ")" << std::endl;
            
            // Create raw MIDI note on event
            std::vector<unsigned char> midi_on = {0x90, (unsigned char)note.midi_note, (unsigned char)(note.velocity * 127)};
            Input::RawMidiEvent raw_on;
            raw_on.data = midi_on;
            raw_on.timestamp = 0.0;
            raw_on.source_port = 0;
            
            // Process through abstraction layer
            std::vector<Input::RawMidiEvent> raw_events = {raw_on};
            auto abstracted_events = input_abstractor_->processEvents(raw_events);
            
            std::cout << "  📥 Raw MIDI: [" << (int)midi_on[0] << " " << (int)midi_on[1] << " " << (int)midi_on[2] << "]" << std::endl;
            std::cout << "  🎛️  Abstracted: " << abstracted_events.size() << " events" << std::endl;
            
            // Send to synthesizer
            for (const auto& event : abstracted_events) {
                synthesizer_->processNoteEvent(event);
            }
            
            std::cout << "  🎵 Active voices: " << synthesizer_->getActiveVoiceCount() << std::endl;
            
            // Generate audio for note duration
            double note_on_time = note.duration * 0.8;
            int samples_needed = static_cast<int>(note_on_time * sample_rate_);
            int buffers_needed = (samples_needed + buffer_size_ - 1) / buffer_size_;
            
            for (int buf = 0; buf < buffers_needed; ++buf) {
                auto buffer = synthesizer_->generateAudioBuffer(buffer_size_);
                complete_audio.insert(complete_audio.end(), buffer.begin(), buffer.end());
            }
            
            // Create raw MIDI note off event
            std::vector<unsigned char> midi_off = {0x80, (unsigned char)note.midi_note, 64};
            Input::RawMidiEvent raw_off;
            raw_off.data = midi_off;
            raw_off.timestamp = note_on_time;
            raw_off.source_port = 0;
            
            // Process note off
            std::vector<Input::RawMidiEvent> off_events = {raw_off};
            auto off_abstracted = input_abstractor_->processEvents(off_events);
            
            for (const auto& event : off_abstracted) {
                synthesizer_->processNoteEvent(event);
            }
            
            // Generate release audio
            double release_time = note.duration * 0.2;
            int release_samples = static_cast<int>(release_time * sample_rate_);
            int release_buffers = (release_samples + buffer_size_ - 1) / buffer_size_;
            
            for (int buf = 0; buf < release_buffers; ++buf) {
                auto buffer = synthesizer_->generateAudioBuffer(buffer_size_);
                complete_audio.insert(complete_audio.end(), buffer.begin(), buffer.end());
            }
            
            // Small pause between notes
            if (i < simple_melody.size() - 1) {
                int pause_samples = static_cast<int>(0.05 * sample_rate_);
                int pause_buffers = (pause_samples + buffer_size_ - 1) / buffer_size_;
                
                for (int buf = 0; buf < pause_buffers; ++buf) {
                    auto buffer = synthesizer_->generateAudioBuffer(buffer_size_);
                    complete_audio.insert(complete_audio.end(), buffer.begin(), buffer.end());
                }
            }
        }
        
        // Generate final tail
        std::cout << "\n🎵 Generating final audio tail..." << std::endl;
        for (int i = 0; i < 20; ++i) {
            auto buffer = synthesizer_->generateAudioBuffer(buffer_size_);
            complete_audio.insert(complete_audio.end(), buffer.begin(), buffer.end());
        }
        
        // Save to WAV file
        saveToWav(complete_audio, "simple_demo.wav");
        
        // Print statistics
        printAudioStats(complete_audio);
        
        std::cout << "\n✅ Demo completed successfully!" << std::endl;
        std::cout << "📁 Audio saved to: simple_demo.wav" << std::endl;
    }
    
private:
    std::string getMidiNoteName(int midi_note) {
        const char* note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        int octave = (midi_note / 12) - 1;
        int note = midi_note % 12;
        return std::string(note_names[note]) + std::to_string(octave);
    }
    
    void saveToWav(const std::vector<float>& audio_data, const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "❌ Failed to create " << filename << std::endl;
            return;
        }
        
        // WAV header
        int channels = 2;
        int sample_rate = static_cast<int>(sample_rate_);
        int bytes_per_sample = 2;
        int frame_count = audio_data.size() / channels;
        int data_size = frame_count * channels * bytes_per_sample;
        int file_size = 36 + data_size;
        
        // RIFF header
        file.write("RIFF", 4);
        file.write(reinterpret_cast<const char*>(&file_size), 4);
        file.write("WAVE", 4);
        
        // Format chunk
        file.write("fmt ", 4);
        int fmt_size = 16;
        short format = 1;
        short num_channels = channels;
        int byte_rate = sample_rate * channels * bytes_per_sample;
        short block_align = channels * bytes_per_sample;
        short bits_per_sample = bytes_per_sample * 8;
        
        file.write(reinterpret_cast<const char*>(&fmt_size), 4);
        file.write(reinterpret_cast<const char*>(&format), 2);
        file.write(reinterpret_cast<const char*>(&num_channels), 2);
        file.write(reinterpret_cast<const char*>(&sample_rate), 4);
        file.write(reinterpret_cast<const char*>(&byte_rate), 4);
        file.write(reinterpret_cast<const char*>(&block_align), 2);
        file.write(reinterpret_cast<const char*>(&bits_per_sample), 2);
        
        // Data chunk
        file.write("data", 4);
        file.write(reinterpret_cast<const char*>(&data_size), 4);
        
        // Audio data
        for (float sample : audio_data) {
            short pcm = static_cast<short>(std::max(-1.0f, std::min(sample, 1.0f)) * 32767.0f);
            file.write(reinterpret_cast<const char*>(&pcm), 2);
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
        std::cout << "   Peak Level: " << peak << std::endl;
        std::cout << "   RMS Level: " << rms << std::endl;
        
        if (peak > 0.0f) {
            std::cout << "   Peak dB: " << (20.0 * std::log10(peak)) << " dB" << std::endl;
            std::cout << "   RMS dB: " << (20.0 * std::log10(rms + 1e-10)) << " dB" << std::endl;
            std::cout << "   Dynamic Range: " << (20.0 * std::log10(peak / (rms + 1e-10))) << " dB" << std::endl;
        }
    }
    
    std::unique_ptr<Abstraction::InputAbstractor> input_abstractor_;
    std::unique_ptr<SimpleSynthesizer> synthesizer_;
    double sample_rate_;
    int buffer_size_;
};

int main() {
    std::cout << "🚀 Piano Synthesizer - Simple Working Demo" << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << "This demo showcases the complete audio processing pipeline:" << std::endl;
    std::cout << "• MIDI event simulation and abstraction" << std::endl;
    std::cout << "• Voice management and polyphonic synthesis" << std::endl;
    std::cout << "• Real-time audio buffer generation" << std::endl;
    std::cout << "• WAV file output for playback" << std::endl;
    std::cout << std::endl;
    
    try {
        SimpleDemo demo;
        
        if (!demo.initialize()) {
            std::cerr << "❌ Failed to initialize demo" << std::endl;
            return 1;
        }
        
        std::cout << "Press Enter to start the demo..." << std::endl;
        std::cin.get();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        demo.playMelody();
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "\n⏱️  Processing completed in " << duration.count() << " ms" << std::endl;
        std::cout << "🎉 Demo completed successfully!" << std::endl;
        std::cout << "\n💡 Next steps:" << std::endl;
        std::cout << "   • Play 'simple_demo.wav' to hear the synthesized melody" << std::endl;
        std::cout << "   • The audio demonstrates working MIDI→synthesis pipeline" << std::endl;
        std::cout << "   • Physical modeling can be added once numerical stability is resolved" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Demo failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}