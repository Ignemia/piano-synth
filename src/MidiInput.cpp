#include "../include/MidiInput.h"
#include <vector>
#include <cmath>
#include <algorithm>

/**
 * @brief [AI GENERATED] Generates Rush E opening with rapid E notes and virtuosic passages.
 */
std::vector<MidiMessage> MidiInput::generateRushE() const {
    std::vector<MidiMessage> messages;
    double currentTime = 0.0;
    
    // Opening rapid E notes in different octaves
    const int eNotes[] = {52, 64, 76, 88, 76, 64, 52, 64, 76, 88, 76, 64}; // E2, E4, E5, E6
    const double rapidTime = 0.05;
    for (int note : eNotes) {
        messages.push_back({note, rapidTime * 2, currentTime});
        currentTime += rapidTime;
    }
    
    // Fast ascending chromatic run
    for (int note = 60; note <= 96; note++) {
        messages.push_back({note, 0.04, currentTime});
        currentTime += 0.03;
    }
    
    // Descending arpeggio cascade
    const int cascade[] = {96, 91, 88, 84, 79, 76, 72, 67, 64, 60, 55, 52, 48, 43, 40, 36};
    for (int note : cascade) {
        messages.push_back({note, 0.15, currentTime});
        currentTime += 0.06;
    }
    
    // Rapid repeated Es with increasing intensity
    for (int i = 0; i < 20; i++) {
        messages.push_back({76, 0.08, currentTime}); // E5
        currentTime += 0.04;
    }
    
    // Final virtuosic run
    const int finalRun[] = {36, 48, 60, 72, 84, 96, 84, 72, 60, 48, 36};
    for (int note : finalRun) {
        messages.push_back({note, 0.2, currentTime});
        currentTime += 0.08;
    }
    
    return messages;
}

/**
 * @brief [AI GENERATED] Generates Für Elise opening melody accurately.
 */
std::vector<MidiMessage> MidiInput::generateFurElise() const {
    std::vector<MidiMessage> messages;
    double currentTime = 0.0;
    
    // Opening phrase: E-D#-E-D#-E-B-D-C-A
    const int melody1[] = {76, 75, 76, 75, 76, 71, 74, 72, 69}; // E5-D#5-E5-D#5-E5-B4-D5-C5-A4
    const double duration1[] = {0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.5};
    
    for (size_t i = 0; i < 9; i++) {
        messages.push_back({melody1[i], duration1[i], currentTime});
        currentTime += duration1[i];
    }
    
    // Rest
    currentTime += 0.25;
    
    // Second phrase: C-E-A-B
    const int melody2[] = {60, 64, 69, 71}; // C4-E4-A4-B4
    const double duration2[] = {0.25, 0.25, 0.25, 0.5};
    
    for (size_t i = 0; i < 4; i++) {
        messages.push_back({melody2[i], duration2[i], currentTime});
        currentTime += duration2[i];
    }
    
    // Rest
    currentTime += 0.25;
    
    // Third phrase: E-G#-B-C
    const int melody3[] = {64, 68, 71, 72}; // E4-G#4-B4-C5
    const double duration3[] = {0.25, 0.25, 0.25, 0.5};
    
    for (size_t i = 0; i < 4; i++) {
        messages.push_back({melody3[i], duration3[i], currentTime});
        currentTime += duration3[i];
    }
    
    // Repeat opening phrase
    for (size_t i = 0; i < 9; i++) {
        messages.push_back({melody1[i], duration1[i], currentTime});
        currentTime += duration1[i];
    }
    
    return messages;
}

/**
 * @brief [AI GENERATED] Generates Beethoven's 5th Symphony opening motif.
 */
std::vector<MidiMessage> MidiInput::generateBeethoven5th() const {
    std::vector<MidiMessage> messages;
    double currentTime = 0.0;
    
    // Famous opening motif: G-G-G-Eb (short-short-short-long)
    const int motif1[] = {67, 67, 67, 63}; // G4-G4-G4-Eb4
    const double duration1[] = {0.25, 0.25, 0.25, 1.0};
    
    for (size_t i = 0; i < 4; i++) {
        messages.push_back({motif1[i], duration1[i], currentTime});
        currentTime += duration1[i];
    }
    
    // Rest
    currentTime += 0.5;
    
    // Second motif: F-F-F-D (one tone lower)
    const int motif2[] = {65, 65, 65, 62}; // F4-F4-F4-D4
    const double duration2[] = {0.25, 0.25, 0.25, 1.0};
    
    for (size_t i = 0; i < 4; i++) {
        messages.push_back({motif2[i], duration2[i], currentTime});
        currentTime += duration2[i];
    }
    
    // Development of the motif
    const int development[] = {67, 69, 71, 72, 74, 76, 77, 79}; // Ascending scale
    for (int note : development) {
        messages.push_back({note, 0.5, currentTime});
        currentTime += 0.5;
    }
    
    // Return to main motif in different octave
    const int motif3[] = {79, 79, 79, 75}; // G5-G5-G5-Eb5
    for (size_t i = 0; i < 4; i++) {
        messages.push_back({motif3[i], duration1[i], currentTime});
        currentTime += duration1[i];
    }
    
    return messages;
}

/**
 * @brief [AI GENERATED] Generates In the Hall of the Mountain King opening.
 */
std::vector<MidiMessage> MidiInput::generateHallOfMountainKing() const {
    std::vector<MidiMessage> messages;
    double currentTime = 0.0;
    
    // Opening melody: B-D-E-F#-G-A-B-A-G-F#-E-D-B
    const int melody[] = {59, 62, 64, 66, 67, 69, 71, 69, 67, 66, 64, 62, 59}; // B3 to B4 and back
    double noteDuration = 0.4;
    
    // First iteration - slow
    for (int note : melody) {
        messages.push_back({note, noteDuration, currentTime});
        currentTime += noteDuration;
    }
    
    // Second iteration - faster
    noteDuration = 0.3;
    for (int note : melody) {
        messages.push_back({note, noteDuration, currentTime});
        currentTime += noteDuration;
    }
    
    // Third iteration - even faster
    noteDuration = 0.2;
    for (int note : melody) {
        messages.push_back({note, noteDuration, currentTime});
        currentTime += noteDuration;
    }
    
    // Fourth iteration - fastest with harmonies
    noteDuration = 0.15;
    for (size_t i = 0; i < 13; i++) {
        messages.push_back({melody[i], noteDuration, currentTime});
        messages.push_back({melody[i] + 12, noteDuration, currentTime}); // Octave harmony
        currentTime += noteDuration;
    }
    
    return messages;
}

/**
 * @brief [AI GENERATED] Generates Vivaldi's Spring from Four Seasons opening.
 */
std::vector<MidiMessage> MidiInput::generateVivaldiSpring() const {
    std::vector<MidiMessage> messages;
    double currentTime = 0.0;
    
    // Spring opening melody (simplified version)
    // "Spring has come" motif
    const int springMelody[] = {
        76, 74, 72, 74, 76, 79, 76, 74, 72, 71, 69, 67, 69, 71, 72, 74, 76
    }; // E5 down to G4 and back up
    
    const double durations[] = {
        0.3, 0.3, 0.3, 0.3, 0.6, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.9
    };
    
    for (size_t i = 0; i < 17; i++) {
        messages.push_back({springMelody[i], durations[i], currentTime});
        currentTime += durations[i];
    }
    
    // Ornamental flourishes
    const int flourish[] = {84, 83, 81, 79, 77, 76, 74, 72, 71, 69, 67, 65, 64, 62, 60};
    for (int note : flourish) {
        messages.push_back({note, 0.1, currentTime});
        currentTime += 0.08;
    }
    
    // Gentle ascending passage
    const int ascending[] = {60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83, 84};
    for (int note : ascending) {
        messages.push_back({note, 0.2, currentTime});
        currentTime += 0.15;
    }
    
    // Final resolution
    const int resolution[] = {84, 79, 76, 72, 67, 64, 60};
    for (int note : resolution) {
        messages.push_back({note, 0.4, currentTime});
        currentTime += 0.3;
    }
    
    return messages;
}

/**
 * @brief [AI GENERATED] Convert MidiMessage sequence to realistic KeyEvent sequence.
 */
std::vector<KeyEvent> MidiInput::convertToKeyEvents(const std::vector<MidiMessage>& midiMessages) const {
    std::vector<KeyEvent> keyEvents;
    
    for (const auto& msg : midiMessages) {
        // Key down event at start time
        keyEvents.push_back({KeyState::KeyDown, msg.note, 80, msg.startTime}); // Default velocity 80
        
        // Key up event at end time
        keyEvents.push_back({KeyState::KeyUp, msg.note, 0, msg.startTime + msg.duration});
    }
    
    // Sort by timestamp to ensure proper ordering
    std::sort(keyEvents.begin(), keyEvents.end(), 
              [](const KeyEvent& a, const KeyEvent& b) { return a.timestamp < b.timestamp; });
    
    return keyEvents;
}

/**
 * @brief [AI GENERATED] Generates Für Elise with realistic key press/release events and velocity.
 */
std::vector<KeyEvent> MidiInput::generateFurEliseKeys() const {
    std::vector<KeyEvent> keyEvents;
    double currentTime = 0.0;
    
    // Opening phrase: E-D#-E-D#-E-B-D-C-A with realistic velocities
    struct NoteData {
        int note;
        double duration;
        int velocity;
    };
    
    const NoteData melody1[] = {
        {76, 0.25, 65},  // E5 - gentle start
        {75, 0.25, 60},  // D#5 - softer
        {76, 0.25, 70},  // E5 - slightly stronger
        {75, 0.25, 60},  // D#5 - softer
        {76, 0.25, 75},  // E5 - stronger
        {71, 0.25, 70},  // B4 - moderate
        {74, 0.25, 72},  // D5 - moderate
        {72, 0.25, 68},  // C5 - gentle
        {69, 0.5, 80}    // A4 - strong ending
    };
    
    // Generate key events for first phrase
    for (const auto& note : melody1) {
        keyEvents.push_back({KeyState::KeyDown, note.note, note.velocity, currentTime});
        keyEvents.push_back({KeyState::KeyUp, note.note, 0, currentTime + note.duration});
        currentTime += note.duration;
    }
    
    // Rest
    currentTime += 0.25;
    
    // Second phrase: C-E-A-B with bass accompaniment feeling
    const NoteData melody2[] = {
        {60, 0.25, 55},  // C4 - gentle bass
        {64, 0.25, 60},  // E4 - soft
        {69, 0.25, 65},  // A4 - moderate
        {71, 0.5, 75}    // B4 - strong
    };
    
    for (const auto& note : melody2) {
        keyEvents.push_back({KeyState::KeyDown, note.note, note.velocity, currentTime});
        keyEvents.push_back({KeyState::KeyUp, note.note, 0, currentTime + note.duration});
        currentTime += note.duration;
    }
    
    // Rest
    currentTime += 0.25;
    
    // Third phrase: E-G#-B-C with building intensity
    const NoteData melody3[] = {
        {64, 0.25, 58},  // E4 - soft
        {68, 0.25, 62},  // G#4 - moderate
        {71, 0.25, 70},  // B4 - stronger
        {72, 0.5, 85}    // C5 - forte
    };
    
    for (const auto& note : melody3) {
        keyEvents.push_back({KeyState::KeyDown, note.note, note.velocity, currentTime});
        keyEvents.push_back({KeyState::KeyUp, note.note, 0, currentTime + note.duration});
        currentTime += note.duration;
    }
    
    // Repeat opening phrase with slightly different expression
    for (const auto& note : melody1) {
        int adjustedVelocity = note.velocity + 5; // Slightly more expressive on repeat
        keyEvents.push_back({KeyState::KeyDown, note.note, adjustedVelocity, currentTime});
        keyEvents.push_back({KeyState::KeyUp, note.note, 0, currentTime + note.duration});
        currentTime += note.duration;
    }
    
    return keyEvents;
}

/**
 * @brief [AI GENERATED] Generate other pieces as key events (placeholder implementations).
 */
std::vector<KeyEvent> MidiInput::generateRushEKeys() const {
    return convertToKeyEvents(generateRushE());
}

std::vector<KeyEvent> MidiInput::generateBeethoven5thKeys() const {
    return convertToKeyEvents(generateBeethoven5th());
}

std::vector<KeyEvent> MidiInput::generateHallOfMountainKingKeys() const {
    return convertToKeyEvents(generateHallOfMountainKing());
}

std::vector<KeyEvent> MidiInput::generateVivaldiSpringKeys() const {
    return convertToKeyEvents(generateVivaldiSpring());
}

/**
 * @brief [AI GENERATED] Generates current demo (Rush E for now).
 */
std::vector<MidiMessage> MidiInput::generateDemo() const {
    return generateRushE(); // Default to Rush E
}