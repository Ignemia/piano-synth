#include "midi_detector.h"

#include <algorithm>
#include <iostream>

namespace PianoSynth {
namespace Input {

MidiDetector::MidiDetector() : device_open_(false) {
    try {
        midi_in_ = std::make_unique<RtMidiIn>();
        initializePatterns();
    } catch (RtMidiError& error) {
        std::cerr << "RtMidi initialization error: " << error.getMessage() << std::endl;
    }
}

MidiDetector::~MidiDetector() {
    closeDevice();
}

void MidiDetector::initializePatterns() {
    // Piano device patterns (case-insensitive)
    piano_patterns_ = {
        "oxygen",
        "pro 61",
        "oxygen pro",
        "m-audio",
        "keyboard",
        "piano",
        "digital piano"
    };
    
    // Drum device patterns
    drum_patterns_ = {
        "drum",
        "percussion",
        "pad",
        "trigger"
    };
    
    // Controller patterns
    controller_patterns_ = {
        "control",
        "mixer",
        "fader",
        "knob"
    };
}

std::vector<MidiDevice> MidiDetector::detectDevices() {
    std::vector<MidiDevice> devices;
    
    if (!midi_in_) {
        return devices;
    }
    
    unsigned int port_count = midi_in_->getPortCount();
    
    for (unsigned int i = 0; i < port_count; i++) {
        try {
            std::string port_name = midi_in_->getPortName(i);
            
            MidiDevice device;
            device.name = port_name;
            device.port = i;
            device.is_piano = isPianoDevice(port_name);
            device.is_drum = isDrumDevice(port_name);
            device.is_controller = isControllerDevice(port_name);
            
            devices.push_back(device);
            
            std::cout << "Found MIDI device [" << i << "]: " << port_name;
            if (device.is_piano) std::cout << " (Piano)";
            if (device.is_drum) std::cout << " (Drum)";
            if (device.is_controller) std::cout << " (Controller)";
            std::cout << std::endl;
            
        } catch (RtMidiError& error) {
            std::cerr << "Error getting port name for port " << i << ": " << error.getMessage() << std::endl;
        }
    }
    
    return devices;
}

bool MidiDetector::isPianoDevice(const std::string& device_name) {
    return matchesPattern(device_name, piano_patterns_);
}

bool MidiDetector::isDrumDevice(const std::string& device_name) {
    return matchesPattern(device_name, drum_patterns_);
}

bool MidiDetector::isControllerDevice(const std::string& device_name) {
    return matchesPattern(device_name, controller_patterns_);
}

bool MidiDetector::matchesPattern(const std::string& device_name, const std::vector<std::string>& patterns) {
    std::string lower_name = device_name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    
    for (const auto& pattern : patterns) {
        if (lower_name.find(pattern) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

bool MidiDetector::openDevice(unsigned int port) {
    if (!midi_in_) {
        return false;
    }
    
    try {
        if (device_open_) {
            closeDevice();
        }
        
        midi_in_->openPort(port);
        device_open_ = true;
        
        std::cout << "Opened MIDI port " << port << ": " << midi_in_->getPortName(port) << std::endl;
        return true;
        
    } catch (RtMidiError& error) {
        std::cerr << "Error opening MIDI port " << port << ": " << error.getMessage() << std::endl;
        return false;
    }
}

void MidiDetector::closeDevice() {
    if (midi_in_ && device_open_) {
        midi_in_->closePort();
        device_open_ = false;
    }
}

} // namespace Input
} // namespace PianoSynth
