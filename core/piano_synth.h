#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

#include "input/midi_input_manager.h"
#include "abstraction/input_abstractor.h"
#include "synthesis/piano_synthesizer.h"
#include "audio/audio_output_manager.h"
#include "audio/recording_manager.h"
#include "utils/config_manager.h"
#include "utils/logger.h"

namespace PianoSynth {

/**
 * Main application class that orchestrates all three layers:
 * 1. Input Layer (MIDI detection and processing)
 * 2. Abstraction Layer (Event processing and formatting)
 * 3. Synthesis/Output Layer (Physical modeling and audio output)
 */
class PianoSynthApp {
public:
    PianoSynthApp();
    ~PianoSynthApp();

    // Application lifecycle
    bool initialize();
    void run();
    void shutdown();

    // Recording control
    void startRecording(const std::string& filename);
    void stopRecording();
    bool isRecording() const { return recording_active_; }

    // Real-time controls
    void setMasterVolume(float volume);
    void setPedalDamping(float damping);
    void setStringTension(float tension);

private:
    // Core components
    std::unique_ptr<Input::MidiInputManager> midi_input_manager_;
    std::unique_ptr<Abstraction::InputAbstractor> input_abstractor_;
    std::unique_ptr<Synthesis::PianoSynthesizer> piano_synthesizer_;
    std::unique_ptr<Audio::AudioOutputManager> audio_output_manager_;
    std::unique_ptr<Audio::RecordingManager> recording_manager_;
    std::unique_ptr<Utils::ConfigManager> config_manager_;
    std::unique_ptr<Utils::Logger> logger_;

    // Threading and synchronization
    std::thread audio_thread_;
    std::thread midi_thread_;
    std::atomic<bool> running_;
    std::mutex state_mutex_;

    // Application state
    bool recording_active_;
    float master_volume_;
    
    // Internal methods
    void audioProcessingLoop();
    void midiProcessingLoop();
    bool setupAudioSystem();
    bool setupMidiSystem();
    void processAudioBuffer();
};

} // namespace PianoSynth
