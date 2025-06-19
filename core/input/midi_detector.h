#pragma once

#include <vector>
#include <string>
#include <memory>
#include <rtmidi/RtMidi.h>

namespace PianoSynth {
namespace Input {

struct MidiDevice {
    std::string name;
    unsigned int port;
    bool is_piano;
    bool is_drum;
    bool is_controller;
};

/**
 * Detects and identifies MIDI devices, specifically looking for
 * M-AUDIO Oxygen Pro 61 and categorizing its functionality
 */
class MidiDetector {
public:
    MidiDetector();
    ~MidiDetector();

    // Device detection
    std::vector<MidiDevice> detectDevices();
    bool isPianoDevice(const std::string& device_name);
    bool isDrumDevice(const std::string& device_name);
    bool isControllerDevice(const std::string& device_name);

    // Device management
    bool openDevice(unsigned int port);
    void closeDevice();
    bool isDeviceOpen() const { return device_open_; }

private:
    std::unique_ptr<RtMidiIn> midi_in_;
    bool device_open_;
    
    // Device identification patterns
    std::vector<std::string> piano_patterns_;
    std::vector<std::string> drum_patterns_;
    std::vector<std::string> controller_patterns_;
    
    void initializePatterns();
    bool matchesPattern(const std::string& device_name, const std::vector<std::string>& patterns);
};

} // namespace Input
} // namespace PianoSynth
