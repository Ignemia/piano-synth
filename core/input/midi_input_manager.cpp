#include "midi_input_manager.h"
#include "../utils/constants.h"
#include <algorithm>

namespace PianoSynth {
namespace Input {

MidiInputManager::MidiInputManager() : connected_(false) {
    midi_detector_ = std::make_unique<MidiDetector>();
}

MidiInputManager::~MidiInputManager() {
    shutdown();
}

bool MidiInputManager::initialize() {
    // Detect available MIDI devices
    auto detected_devices = midi_detector_->detectDevices();
    
    if (detected_devices.empty()) {
        return false;
    }
    
    // Try to connect to the best piano device
    return connectToPrimaryPiano();
}

void MidiInputManager::shutdown() {
    disconnectAll();
    connected_devices_.clear();
    clearEventBuffer();
}

bool MidiInputManager::connectToPrimaryPiano() {
    auto detected_devices = midi_detector_->detectDevices();
    
    // Find the best piano device
    MidiDevice best_device = selectBestPianoDevice(detected_devices);
    
    if (best_device.name.empty()) {
        return false;
    }
    
    return connectToDevice(best_device.port);
}

bool MidiInputManager::connectToDevice(unsigned int port) {
    if (midi_detector_->openDevice(port)) {
        // Find device info
        auto detected_devices = midi_detector_->detectDevices();
        for (const auto& device : detected_devices) {
            if (device.port == port) {
                connected_devices_.push_back(device);
                connected_ = true;
                return true;
            }
        }
    }
    
    return false;
}

void MidiInputManager::disconnectAll() {
    midi_detector_->closeDevice();
    connected_devices_.clear();
    connected_ = false;
}

std::vector<RawMidiEvent> MidiInputManager::pollEvents() {
    std::lock_guard<std::mutex> lock(event_mutex_);
    std::vector<RawMidiEvent> events = event_buffer_;
    event_buffer_.clear();
    return events;
}

void MidiInputManager::clearEventBuffer() {
    std::lock_guard<std::mutex> lock(event_mutex_);
    event_buffer_.clear();
}

void MidiInputManager::midiCallback(double timestamp, std::vector<unsigned char>* message, void* user_data) {
    if (!message || message->empty() || !user_data) {
        return;
    }
    
    MidiInputManager* manager = static_cast<MidiInputManager*>(user_data);
    manager->processMidiMessage(timestamp, *message, 0); // Port 0 for now
}

void MidiInputManager::processMidiMessage(double timestamp, const std::vector<unsigned char>& message, unsigned int port) {
    if (!isValidPianoMessage(message)) {
        return;
    }
    
    RawMidiEvent event;
    event.data = message;
    event.timestamp = timestamp;
    event.source_port = port;
    
    std::lock_guard<std::mutex> lock(event_mutex_);
    event_buffer_.push_back(event);
}

MidiDevice MidiInputManager::selectBestPianoDevice(const std::vector<MidiDevice>& devices) {
    // Priority order: M-AUDIO Oxygen Pro, other piano devices, any device
    
    // First, look for M-AUDIO Oxygen Pro specifically
    for (const auto& device : devices) {
        std::string lower_name = device.name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        
        if (lower_name.find("oxygen") != std::string::npos && 
            lower_name.find("pro") != std::string::npos) {
            return device;
        }
    }
    
    // Next, look for any piano device
    for (const auto& device : devices) {
        if (device.is_piano) {
            return device;
        }
    }
    
    // Finally, return the first device if any
    if (!devices.empty()) {
        return devices[0];
    }
    
    return MidiDevice{}; // Empty device
}

bool MidiInputManager::isValidPianoMessage(const std::vector<unsigned char>& message) {
    if (message.empty()) {
        return false;
    }
    
    unsigned char status = message[0] & 0xF0;
    
    // Accept note on/off, control change, pitch bend
    return (status == Constants::MIDI_NOTE_ON ||
            status == Constants::MIDI_NOTE_OFF ||
            status == Constants::MIDI_CONTROL_CHANGE ||
            status == 0xE0); // Pitch bend
}

} // namespace Input
} // namespace PianoSynth
