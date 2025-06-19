#pragma once

#include <cstdint>
#include <chrono>

namespace PianoSynth {
namespace Interfaces {

// Standard event types across the system
enum class EventType {
    NOTE_ON,
    NOTE_OFF,
    PEDAL_CHANGE,
    PITCH_BEND,
    CONTROL_CHANGE,
    AFTERTOUCH,
    SYSTEM_RESET
};

// Pedal states
struct PedalState {
    bool sustain = false;
    bool soft = false;
    bool sostenuto = false;
    float sustain_position = 0.0f;   // 0.0 = fully released, 1.0 = fully pressed
    float soft_position = 0.0f;
    float sostenuto_position = 0.0f;
};

// Standard musical event format used between DLLs
struct MusicalEvent {
    EventType type = EventType::NOTE_ON;
    std::chrono::high_resolution_clock::time_point timestamp;
    
    // Note information
    int note_number = 60;            // MIDI note number (0-127)
    float velocity = 0.0f;           // 0.0 - 1.0
    float release_velocity = 0.0f;   // For note off events
    
    // Expression
    float pressure = 0.0f;           // Aftertouch pressure (0.0 - 1.0)
    float pitch_bend = 0.0f;         // -1.0 to 1.0 (semitones)
    
    // Pedals
    PedalState pedals;
    
    // Control changes
    int controller_number = 0;       // For CC events
    float controller_value = 0.0f;   // 0.0 - 1.0
    
    // Source information
    int channel = 0;                 // MIDI channel (0-15)
    int source_device_id = 0;        // Which input device
};

// Standard audio buffer format
struct AudioBuffer {
    float* samples = nullptr;        // Interleaved audio samples (L, R, L, R, ...)
    size_t frame_count = 0;          // Number of audio frames
    size_t channel_count = 2;        // Number of audio channels (usually 2 for stereo)
    double sample_rate = 44100.0;    // Sample rate in Hz
    std::chrono::high_resolution_clock::time_point timestamp;
    
    // Helper methods
    size_t total_samples() const { return frame_count * channel_count; }
    size_t size_bytes() const { return total_samples() * sizeof(float); }
};

// Configuration change notification
struct ConfigUpdate {
    const char* dll_name;
    const char* config_section;
    const char* json_data;
};

} // namespace Interfaces
} // namespace PianoSynth
