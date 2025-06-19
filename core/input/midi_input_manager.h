#pragma once

#include "midi_detector.h"
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

namespace PianoSynth {
namespace Input {

struct RawMidiEvent {
    std::vector<unsigned char> data;
    double timestamp;
    unsigned int source_port;
};

/**
 * Manages MIDI input from multiple devices, specifically handling
 * the M-AUDIO Oxygen Pro 61 multi-zone functionality
 */
class MidiInputManager {
public:
    MidiInputManager();
    ~MidiInputManager();

    // Initialization
    bool initialize();
    void shutdown();

    // Device management
    bool connectToPrimaryPiano();
    bool connectToDevice(unsigned int port);
    void disconnectAll();

    // Event processing
    std::vector<RawMidiEvent> pollEvents();
    void clearEventBuffer();

    // Status
    bool isConnected() const { return connected_; }
    std::vector<MidiDevice> getConnectedDevices() const { return connected_devices_; }

private:
    std::unique_ptr<MidiDetector> midi_detector_;
    std::vector<MidiDevice> connected_devices_;
    std::vector<RawMidiEvent> event_buffer_;
    
    mutable std::mutex event_mutex_;
    std::atomic<bool> connected_;
    
    // MIDI callback
    static void midiCallback(double timestamp, std::vector<unsigned char>* message, void* user_data);
    void processMidiMessage(double timestamp, const std::vector<unsigned char>& message, unsigned int port);
    
    // Device selection logic
    MidiDevice selectBestPianoDevice(const std::vector<MidiDevice>& devices);
    bool isValidPianoMessage(const std::vector<unsigned char>& message);
};

} // namespace Input
} // namespace PianoSynth
