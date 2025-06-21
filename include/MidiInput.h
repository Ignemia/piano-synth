/**
 * @file MidiInput.h
 * @brief [AI GENERATED] Simple MIDI input provider.
 */

#pragma once
#include <vector>

/**
 * @brief [AI GENERATED] Key state for realistic piano key events.
 */
enum class KeyState {
    KeyDown,    /**< Key pressed down - start playing note. */
    KeyUp       /**< Key released - stop playing note. */
};

/**
 * @brief [AI GENERATED] Represents a realistic key press/release event.
 */
struct KeyEvent {
    KeyState state;    /**< Whether key is pressed or released. */
    int note;          /**< MIDI note number (0-127). */
    int velocity;      /**< Key velocity (0-127, how hard/fast key was pressed). */
    double timestamp;  /**< Time when event occurs in seconds. */
};

/**
 * @brief [AI GENERATED] Represents a single MIDI note message (legacy).
 */
struct MidiMessage {
    int note;          /**< MIDI note number. */
    double duration;   /**< Duration of the note in seconds. */
    double startTime;  /**< Start time of the note in seconds. */
};

/**
 * @brief [AI GENERATED] Generates MIDI and key events.
 */
class MidiInput {
public:
    // Legacy methods returning MidiMessage
    std::vector<MidiMessage> generateDemo() const;
    std::vector<MidiMessage> generateRushE() const;
    std::vector<MidiMessage> generateFurElise() const;
    std::vector<MidiMessage> generateBeethoven5th() const;
    std::vector<MidiMessage> generateHallOfMountainKing() const;
    std::vector<MidiMessage> generateVivaldiSpring() const;
    
    // New methods returning realistic KeyEvent sequences
    std::vector<KeyEvent> generateRushEKeys() const;
    std::vector<KeyEvent> generateFurEliseKeys() const;
    std::vector<KeyEvent> generateBeethoven5thKeys() const;
    std::vector<KeyEvent> generateHallOfMountainKingKeys() const;
    std::vector<KeyEvent> generateVivaldiSpringKeys() const;
    
    // Helper method to convert MidiMessage to KeyEvent sequence
    std::vector<KeyEvent> convertToKeyEvents(const std::vector<MidiMessage>& midiMessages) const;
};
