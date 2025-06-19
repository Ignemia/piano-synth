#pragma once

#include <chrono>

namespace PianoSynth {
namespace Abstraction {

/**
 * Abstracted note event that contains all information needed
 * for physical modeling synthesis
 */
struct NoteEvent {
    enum Type {
        NOTE_ON,
        NOTE_OFF,
        PEDAL_CHANGE,
        PITCH_BEND,
        AFTERTOUCH
    };
    
    Type type;
    int note_number;        // MIDI note number (21-108 for piano)
    float velocity;         // Note velocity (0.0-1.0)
    float release_velocity; // Note off velocity (0.0-1.0)
    
    // Timing information
    std::chrono::high_resolution_clock::time_point press_time;
    std::chrono::high_resolution_clock::time_point release_time;
    double duration_ms;     // Duration in milliseconds (if note_off occurred)
    
    // Physical modeling parameters
    float hammer_velocity;  // Calculated hammer velocity
    float string_excitation; // String excitation force
    float damper_position;  // Damper position (0.0 = fully damped, 1.0 = fully open)
    
    // Pedal state
    bool sustain_pedal;
    bool soft_pedal;
    bool sostenuto_pedal;
    
    // Expression parameters
    float pitch_bend;       // Pitch bend amount (-1.0 to 1.0)
    float aftertouch;       // Channel pressure (0.0-1.0)
    
    NoteEvent() : 
        type(NOTE_ON),
        note_number(60),
        velocity(0.5f),
        release_velocity(0.5f),
        duration_ms(0.0),
        hammer_velocity(0.0f),
        string_excitation(0.0f),
        damper_position(1.0f),
        sustain_pedal(false),
        soft_pedal(false),
        sostenuto_pedal(false),
        pitch_bend(0.0f),
        aftertouch(0.0f) {}
};

/**
 * Extended note event for tracking note state over time
 */
struct ActiveNote {
    NoteEvent initial_event;
    bool is_active;
    std::chrono::high_resolution_clock::time_point last_update;
    
    // Physical state tracking
    float current_amplitude;
    float current_frequency;
    bool pedal_sustained;
    
    // Default constructor
    ActiveNote() : 
        initial_event{},
        is_active(false),
        last_update(std::chrono::high_resolution_clock::now()),
        current_amplitude(0.0f),
        current_frequency(0.0f),
        pedal_sustained(false) {}
    
    ActiveNote(const NoteEvent& event) :
        initial_event(event),
        is_active(true),
        last_update(std::chrono::high_resolution_clock::now()),
        current_amplitude(event.velocity),
        current_frequency(0.0f),
        pedal_sustained(false) {}
};

} // namespace Abstraction
} // namespace PianoSynth
