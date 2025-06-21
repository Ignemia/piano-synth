#include "../include/Abstractor.h"

#include <vector>
#include <cmath>

/**
 * @brief [AI GENERATED] Perform MIDI to frequency conversion.
 */
std::vector<NoteEvent> Abstractor::convert(const std::vector<MidiMessage>& midi) const {
    std::vector<NoteEvent> events;
    for (const auto& msg : midi) {
        double freq = 440.0 * std::pow(2.0, (msg.note - 69) / 12.0);
        events.push_back({freq, msg.duration, msg.startTime, 0.7}); // Default velocity
    }
    return events;
}

/**
 * @brief [AI GENERATED] Convert key events (press/release) to note events with velocity.
 */
std::vector<NoteEvent> Abstractor::convertKeyEvents(const std::vector<KeyEvent>& keyEvents) const {
    std::vector<NoteEvent> events;
    std::vector<KeyEvent> pendingKeys; // Keys pressed but not yet released
    
    for (const auto& keyEvent : keyEvents) {
        if (keyEvent.state == KeyState::KeyDown) {
            // Store key press event
            pendingKeys.push_back(keyEvent);
        } else if (keyEvent.state == KeyState::KeyUp) {
            // Find matching key press event (match by note, device, and channel)
            for (auto it = pendingKeys.begin(); it != pendingKeys.end(); ++it) {
                if (it->note == keyEvent.note && 
                    it->device == keyEvent.device && 
                    it->channel == keyEvent.channel) {
                    // Calculate note duration from key press to key release
                    double duration = keyEvent.timestamp - it->timestamp;
                    double freq = 440.0 * std::pow(2.0, (it->note - 69) / 12.0);
                    double velocity = it->velocity / 127.0; // Convert MIDI velocity to 0.0-1.0
                    
                    // For drum pads, use shorter default duration if calculated duration is very long
                    if (it->device == DeviceType::DrumPad && duration > 0.5) {
                        duration = 0.2; // Short drum hit
                    }
                    
                    events.push_back({freq, duration, it->timestamp, velocity});
                    pendingKeys.erase(it);
                    break;
                }
            }
        }
    }
    
    // Handle any keys that are still pressed (no key up event yet)
    // Give them a default duration based on device type
    for (const auto& pendingKey : pendingKeys) {
        double freq = 440.0 * std::pow(2.0, (pendingKey.note - 69) / 12.0);
        double velocity = pendingKey.velocity / 127.0;
        
        // Different default durations for different devices
        double defaultDuration;
        if (pendingKey.device == DeviceType::DrumPad) {
            defaultDuration = 0.2; // Short drum hit
        } else {
            defaultDuration = 1.0; // 1 second for piano keys
        }
        
        events.push_back({freq, defaultDuration, pendingKey.timestamp, velocity});
    }
    
    return events;
}
