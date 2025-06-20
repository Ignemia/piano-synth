#include "input_abstractor.h"
#include "../utils/constants.h"
#include "../utils/math_utils.h"

namespace PianoSynth {
namespace Abstraction {

InputAbstractor::InputAbstractor() 
    : sustain_pedal_(false),
      soft_pedal_(false),
      sostenuto_pedal_(false),
      pitch_bend_(0.0f),
      velocity_curve_(1.0f),
      hammer_response_curve_(1.0f) {
}

InputAbstractor::~InputAbstractor() = default;

void InputAbstractor::initialize() {
    active_notes_.clear();
    sustain_pedal_ = false;
    soft_pedal_ = false;
    sostenuto_pedal_ = false;
    pitch_bend_ = 0.0f;
}

std::vector<NoteEvent> InputAbstractor::processEvents(const std::vector<Input::RawMidiEvent>& raw_events) {
    std::vector<NoteEvent> processed_events;
    
    for (const auto& raw_event : raw_events) {
        if (raw_event.data.empty()) continue;
        
        if (isMidiNoteOn(raw_event.data)) {
            auto note_event = createNoteOnEvent(raw_event);
            processed_events.push_back(note_event);
            
            // Add to active notes tracking
            active_notes_[note_event.note_number] = ActiveNote(note_event);
            
        } else if (isMidiNoteOff(raw_event.data)) {
            auto note_event = createNoteOffEvent(raw_event);
            processed_events.push_back(note_event);
            
            // Update active note
            auto it = active_notes_.find(note_event.note_number);
            if (it != active_notes_.end()) {
                it->second.is_active = false;
                it->second.initial_event.release_time = std::chrono::high_resolution_clock::now();
                
                // Calculate duration
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                    it->second.initial_event.release_time - it->second.initial_event.press_time);
                it->second.initial_event.duration_ms = duration.count() / 1000.0;
            }
            
        } else if (isMidiControlChange(raw_event.data)) {
            int controller = extractController(raw_event.data);
            float value = extractControllerValue(raw_event.data);
            
            updatePedalState(controller, value);
            
            auto pedal_event = createPedalEvent(raw_event);
            processed_events.push_back(pedal_event);
            
        } else if (isMidiPitchBend(raw_event.data)) {
            float bend_value = extractPitchBendValue(raw_event.data);
            updatePitchBend(bend_value);
        }
    }
    
    return processed_events;
}

NoteEvent InputAbstractor::createNoteOnEvent(const Input::RawMidiEvent& raw_event) {
    NoteEvent event;
    event.type = NoteEvent::NOTE_ON;
    event.note_number = extractNoteNumber(raw_event.data);
    event.velocity = extractVelocity(raw_event.data);
    event.press_time = std::chrono::high_resolution_clock::now();
    
    // Calculate physical parameters
    event.hammer_velocity = calculateHammerVelocity(event.velocity, event.note_number);
    event.string_excitation = calculateStringExcitation(event.hammer_velocity, event.note_number);
    event.damper_position = calculateDamperPosition(sustain_pedal_, soft_pedal_, event.note_number);
    
    // Set pedal states
    event.sustain_pedal = sustain_pedal_;
    event.soft_pedal = soft_pedal_;
    event.sostenuto_pedal = sostenuto_pedal_;
    event.pitch_bend = pitch_bend_;
    
    return event;
}

NoteEvent InputAbstractor::createNoteOffEvent(const Input::RawMidiEvent& raw_event) {
    NoteEvent event;
    event.type = NoteEvent::NOTE_OFF;
    event.note_number = extractNoteNumber(raw_event.data);
    event.release_velocity = extractVelocity(raw_event.data);
    event.release_time = std::chrono::high_resolution_clock::now();
    
    // Copy current pedal states
    event.sustain_pedal = sustain_pedal_;
    event.soft_pedal = soft_pedal_;
    event.sostenuto_pedal = sostenuto_pedal_;
    event.pitch_bend = pitch_bend_;
    
    return event;
}

/**
 * Translate a raw MIDI control change event into a pedal change note event.
 * Values are normalized so a threshold of 0.5 represents MIDI value 64.
 * [AI GENERATED]
 */
NoteEvent InputAbstractor::createPedalEvent(const Input::RawMidiEvent& raw_event) {
    NoteEvent event;
    event.type = NoteEvent::PEDAL_CHANGE;
    
    int controller = extractController(raw_event.data);
    float value = extractControllerValue(raw_event.data);
    
    // Update pedal states based on controller value. The incoming value is
    // normalized (0.0 - 1.0), so compare against 0.5 to emulate the standard
    // MIDI threshold of 64. [AI GENERATED]
    if (controller == Constants::MIDI_SUSTAIN_PEDAL) {
        sustain_pedal_ = (value > 0.5f);
    }
    
    // Set pedal states
    event.sustain_pedal = sustain_pedal_;
    event.soft_pedal = soft_pedal_;
    event.sostenuto_pedal = sostenuto_pedal_;
    
    return event;
}

float InputAbstractor::calculateHammerVelocity(float midi_velocity, int note_number) {
    // Apply velocity curve
    float curved_velocity = pow(midi_velocity, velocity_curve_);
    
    // Scale based on note position (higher notes have lighter hammers)
    float note_factor = 1.0f - (note_number - Constants::LOWEST_KEY) * 0.005f;
    note_factor = Utils::MathUtils::clamp(note_factor, 0.3f, 1.0f);
    
    // Convert to physical hammer velocity (m/s)
    float hammer_velocity = curved_velocity * hammer_response_curve_ * note_factor * 5.0f; // Max ~5 m/s
    
    return hammer_velocity;
}

float InputAbstractor::calculateStringExcitation(float hammer_velocity, int note_number) {
    // String excitation force based on hammer velocity and string properties
    float string_mass_factor = 1.0f + (Constants::HIGHEST_KEY - note_number) * 0.01f; // Lower strings are heavier
    float excitation_force = hammer_velocity * hammer_velocity * string_mass_factor * 0.1f;
    
    return Utils::MathUtils::clamp(excitation_force, 0.0f, 10.0f);
}

float InputAbstractor::calculateDamperPosition(bool sustain, bool soft, int note_number) {
    float damper_pos = 1.0f; // Fully open by default
    
    if (!sustain) {
        damper_pos = 0.0f; // Fully damped if sustain pedal not pressed
    }
    
    // Apply note-specific damper behavior
    // Higher notes typically have less damper contact
    if (note_number > 80) {
        damper_pos *= 0.8f; // Less damping for very high notes
    } else if (note_number < 30) {
        damper_pos *= 1.2f; // More damping for very low notes
    }
    
    if (soft) {
        damper_pos *= 0.7f; // Soft pedal reduces damper opening
    }
    
    return damper_pos;
}

void InputAbstractor::updatePedalState(int controller, float value) {
    switch (controller) {
        case Constants::MIDI_SUSTAIN_PEDAL:
            sustain_pedal_ = (value > 0.5f);
            break;
        case 67: // Soft pedal (sostenuto)
            soft_pedal_ = (value > 0.5f);
            break;
        case 66: // Sostenuto pedal
            sostenuto_pedal_ = (value > 0.5f);
            break;
    }
}

void InputAbstractor::updatePitchBend(float bend_amount) {
    pitch_bend_ = Utils::MathUtils::clamp(bend_amount, -1.0f, 1.0f);
}

void InputAbstractor::clearActiveNotes() {
    active_notes_.clear();
}

void InputAbstractor::setVelocityCurve(float curve_factor) {
    velocity_curve_ = Utils::MathUtils::clamp(curve_factor, 0.1f, 3.0f);
}

void InputAbstractor::setHammerResponseCurve(float response_factor) {
    hammer_response_curve_ = Utils::MathUtils::clamp(response_factor, 0.1f, 3.0f);
}

// MIDI parsing utilities
bool InputAbstractor::isMidiNoteOn(const std::vector<unsigned char>& data) {
    return data.size() >= 3 && (data[0] & 0xF0) == Constants::MIDI_NOTE_ON && data[2] > 0;
}

bool InputAbstractor::isMidiNoteOff(const std::vector<unsigned char>& data) {
    return data.size() >= 3 && 
           (((data[0] & 0xF0) == Constants::MIDI_NOTE_OFF) || 
            ((data[0] & 0xF0) == Constants::MIDI_NOTE_ON && data[2] == 0));
}

bool InputAbstractor::isMidiControlChange(const std::vector<unsigned char>& data) {
    return data.size() >= 3 && (data[0] & 0xF0) == Constants::MIDI_CONTROL_CHANGE;
}

bool InputAbstractor::isMidiPitchBend(const std::vector<unsigned char>& data) {
    return data.size() >= 3 && (data[0] & 0xF0) == 0xE0;
}

int InputAbstractor::extractNoteNumber(const std::vector<unsigned char>& data) {
    return (data.size() >= 2) ? data[1] : 60;
}

float InputAbstractor::extractVelocity(const std::vector<unsigned char>& data) {
    return (data.size() >= 3) ? (data[2] / 127.0f) : 0.5f;
}

int InputAbstractor::extractController(const std::vector<unsigned char>& data) {
    return (data.size() >= 2) ? data[1] : 0;
}

float InputAbstractor::extractControllerValue(const std::vector<unsigned char>& data) {
    return (data.size() >= 3) ? (data[2] / 127.0f) : 0.0f;
}

float InputAbstractor::extractPitchBendValue(const std::vector<unsigned char>& data) {
    if (data.size() >= 3) {
        int bend_value = (data[2] << 7) | data[1];
        return (bend_value - 8192) / 8192.0f; // Normalize to -1.0 to 1.0
    }
    return 0.0f;
}

} // namespace Abstraction
} // namespace PianoSynth
