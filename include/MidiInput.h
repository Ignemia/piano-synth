/**
 * @file MidiInput.h
 * @brief [AI GENERATED] Simple MIDI input provider.
 */

#pragma once
#include <vector>

/**
 * @brief [AI GENERATED] Input device types for M-Audio Oxygen Pro 61.
 */
enum class DeviceType {
    Piano,      /**< 61-key piano keyboard. */
    DrumPad     /**< 8 velocity-sensitive drum pads. */
};

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
    DeviceType device; /**< Which device generated this event. */
    KeyState state;    /**< Whether key is pressed or released. */
    int note;          /**< MIDI note number (0-127) or pad number (0-7) for drums. */
    int velocity;      /**< Key velocity (0-127, how hard/fast key was pressed). */
    int channel;       /**< MIDI channel (1-16). */
    double timestamp;  /**< Time when event occurs in seconds. */
};

/**
 * @brief [AI GENERATED] Drum pad mapping for M-Audio Oxygen Pro 61.
 */
struct DrumMapping {
    int padNumber;     /**< Pad number (0-7). */
    int midiNote;      /**< MIDI note for drum sound. */
    const char* name;  /**< Drum sound name. */
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
 * @brief [AI GENERATED] Generates MIDI and key events for M-Audio Oxygen Pro 61.
 */
class MidiInput {
private:
    static const DrumMapping drumMap[8];  /**< Default drum pad mapping. */
    
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
    
    // M-Audio Oxygen Pro 61 specific methods
    std::vector<KeyEvent> generateDrumPattern() const;
    std::vector<KeyEvent> generateMixedPerformance() const;  // Piano + drums
    
    // Helper methods
    std::vector<KeyEvent> convertToKeyEvents(const std::vector<MidiMessage>& midiMessages) const;
    KeyEvent createPianoEvent(KeyState state, int note, int velocity, double timestamp, int channel = 1) const;
    KeyEvent createDrumEvent(KeyState state, int padNumber, int velocity, double timestamp, int channel = 10) const;
    
    // Utility methods
    static const DrumMapping& getDrumMapping(int padNumber);
    static int getMidiNoteForPad(int padNumber);
    static const char* getDrumName(int padNumber);
};
