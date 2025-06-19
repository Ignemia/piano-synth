#pragma once

#include "note_event.h"
#include "../input/midi_input_manager.h"
#include <vector>
#include <map>
#include <memory>

namespace PianoSynth {
namespace Abstraction {

/**
 * Converts raw MIDI events into abstracted note events
 * with physical modeling parameters
 */
class InputAbstractor {
public:
    InputAbstractor();
    ~InputAbstractor();

    void initialize();
    
    // Main processing function
    std::vector<NoteEvent> processEvents(const std::vector<Input::RawMidiEvent>& raw_events);
    
    // State management
    void updatePedalState(int controller, float value);
    void updatePitchBend(float bend_amount);
    void clearActiveNotes();
    
    // Configuration
    void setVelocityCurve(float curve_factor);
    void setHammerResponseCurve(float response_factor);
    
private:
    // Active note tracking
    std::map<int, ActiveNote> active_notes_;
    
    // Pedal state
    bool sustain_pedal_;
    bool soft_pedal_;
    bool sostenuto_pedal_;
    float pitch_bend_;
    
    // Velocity and response curves
    float velocity_curve_;
    float hammer_response_curve_;
    
    // Internal processing methods
    NoteEvent createNoteOnEvent(const Input::RawMidiEvent& raw_event);
    NoteEvent createNoteOffEvent(const Input::RawMidiEvent& raw_event);
    NoteEvent createPedalEvent(const Input::RawMidiEvent& raw_event);
    
    // Physical parameter calculation
    float calculateHammerVelocity(float midi_velocity, int note_number);
    float calculateStringExcitation(float hammer_velocity, int note_number);
    float calculateDamperPosition(bool sustain, bool soft, int note_number);
    
    // MIDI parsing utilities
    bool isMidiNoteOn(const std::vector<unsigned char>& data);
    bool isMidiNoteOff(const std::vector<unsigned char>& data);
    bool isMidiControlChange(const std::vector<unsigned char>& data);
    bool isMidiPitchBend(const std::vector<unsigned char>& data);
    
    int extractNoteNumber(const std::vector<unsigned char>& data);
    float extractVelocity(const std::vector<unsigned char>& data);
    int extractController(const std::vector<unsigned char>& data);
    float extractControllerValue(const std::vector<unsigned char>& data);
    float extractPitchBendValue(const std::vector<unsigned char>& data);
};

} // namespace Abstraction
} // namespace PianoSynth
