#include "piano_synth.h"
#include "utils/constants.h"

#include <iostream>
#include <chrono>
#include <algorithm>

namespace PianoSynth {

PianoSynthApp::PianoSynthApp() 
    : running_(false), recording_active_(false), master_volume_(0.8f) {
    
    logger_ = std::make_unique<Utils::Logger>();
    config_manager_ = std::make_unique<Utils::ConfigManager>();
}

PianoSynthApp::~PianoSynthApp() {
    shutdown();
}

bool PianoSynthApp::initialize() {
    logger_->info("Initializing Piano Synthesizer...");
    
    // Load configuration
    if (!config_manager_->loadConfig("config/piano_config.json")) {
        logger_->error("Failed to load configuration");
        return false;
    }
    
    // Initialize MIDI input system
    midi_input_manager_ = std::make_unique<Input::MidiInputManager>();
    if (!midi_input_manager_->initialize()) {
        logger_->error("Failed to initialize MIDI input system");
        return false;
    }
    
    // Initialize input abstraction layer
    input_abstractor_ = std::make_unique<Abstraction::InputAbstractor>();
    input_abstractor_->initialize();
    
    // Initialize piano synthesizer with physical models
    piano_synthesizer_ = std::make_unique<Synthesis::PianoSynthesizer>();
    if (!piano_synthesizer_->initialize(config_manager_.get())) {
        logger_->error("Failed to initialize piano synthesizer");
        return false;
    }
    
    // Initialize audio output system
    audio_output_manager_ = std::make_unique<Audio::AudioOutputManager>();
    if (!audio_output_manager_->initialize()) {
        logger_->error("Failed to initialize audio output system");
        return false;
    }
    
    // Initialize recording manager
    recording_manager_ = std::make_unique<Audio::RecordingManager>();
    recording_manager_->initialize();
    
    logger_->info("Piano Synthesizer initialized successfully");
    return true;
}

void PianoSynthApp::run(bool auto_record) {
    if (!initialize()) {
        logger_->error("Failed to initialize application");
        return;
    }
    
    running_ = true;
    
    // Start processing threads
    audio_thread_ = std::thread(&PianoSynthApp::audioProcessingLoop, this);
    midi_thread_ = std::thread(&PianoSynthApp::midiProcessingLoop, this);
    
    if (auto_record && !recording_active_) {
        auto ts = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(ts);
        startRecording(std::string("recording_") + std::to_string(t) + ".mp3");
    }

    logger_->info("Piano Synthesizer is running. Press 'q' to quit.");
    
    // Main loop - handle user input
    char input;
    while (running_ && std::cin >> input) {
        switch (input) {
            case 'q':
            case 'Q':
                running_ = false;
                break;
            case 'r':
                if (!recording_active_) {
                    auto timestamp = std::chrono::system_clock::now();
                    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
                    std::string filename = "recording_" + std::to_string(time_t) + ".mp3";
                    startRecording(filename);
                    std::cout << "Started recording to " << filename << std::endl;
                } else {
                    stopRecording();
                    std::cout << "Stopped recording" << std::endl;
                }
                break;
            default:
                break;
        }
    }
    
    // Wait for threads to complete
    if (audio_thread_.joinable()) audio_thread_.join();
    if (midi_thread_.joinable()) midi_thread_.join();
}

void PianoSynthApp::shutdown() {
    logger_->info("Shutting down Piano Synthesizer...");
    
    running_ = false;
    
    if (recording_active_) {
        stopRecording();
    }
    
    // Cleanup components in reverse order
    recording_manager_.reset();
    audio_output_manager_.reset();
    piano_synthesizer_.reset();
    input_abstractor_.reset();
    midi_input_manager_.reset();
    config_manager_.reset();
    
    logger_->info("Piano Synthesizer shutdown complete");
}

void PianoSynthApp::audioProcessingLoop() {
    logger_->info("Audio processing thread started");
    
    const int buffer_size = Constants::BUFFER_SIZE;
    const double sample_rate = Constants::SAMPLE_RATE;
    
    while (running_) {
        processAudioBuffer();
        
        // Sleep for approximately one buffer duration
        std::this_thread::sleep_for(
            std::chrono::microseconds(static_cast<int>(1000000.0 * buffer_size / sample_rate))
        );
    }
    
    logger_->info("Audio processing thread stopped");
}

void PianoSynthApp::midiProcessingLoop() {
    logger_->info("MIDI processing thread started");
    
    while (running_) {
        // Process incoming MIDI events
        auto midi_events = midi_input_manager_->pollEvents();
        
        // Convert to abstracted note events
        auto note_events = input_abstractor_->processEvents(midi_events);
        
        // Send to synthesizer
        for (const auto& event : note_events) {
            piano_synthesizer_->processNoteEvent(event);
        }
        
        // Small sleep to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    logger_->info("MIDI processing thread stopped");
}

void PianoSynthApp::processAudioBuffer() {
    const int buffer_size = Constants::BUFFER_SIZE;
    
    // Generate audio samples from the synthesizer
    auto audio_buffer = piano_synthesizer_->generateAudioBuffer(buffer_size);
    
    // Apply master volume
    for (auto& sample : audio_buffer) {
        sample *= master_volume_;
    }
    
    // Output to speakers
    audio_output_manager_->outputBuffer(audio_buffer);
    
    // Record if active
    if (recording_active_) {
        recording_manager_->addSamples(audio_buffer);
    }
}

void PianoSynthApp::startRecording(const std::string& filename) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!recording_active_) {
        recording_manager_->startRecording(filename);
        recording_active_ = true;
        logger_->info("Started recording to: " + filename);
    }
}

void PianoSynthApp::stopRecording() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (recording_active_) {
        recording_manager_->stopRecording();
        recording_active_ = false;
        logger_->info("Stopped recording");
    }
}

void PianoSynthApp::setMasterVolume(float volume) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    master_volume_ = std::clamp(volume, 0.0f, 1.0f);
    logger_->info("Master volume set to: " + std::to_string(master_volume_));
}

void PianoSynthApp::setPedalDamping(float damping) {
    piano_synthesizer_->setPedalDamping(damping);
}

void PianoSynthApp::setStringTension(float tension) {
    piano_synthesizer_->setStringTension(tension);
}

} // namespace PianoSynth
