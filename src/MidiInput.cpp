#include "../include/MidiInput.h"
#include <vector>
#include <algorithm>

/**
 * @brief [AI GENERATED] Default drum pad mapping for M-Audio Oxygen Pro 61.
 */
const DrumMapping MidiInput::drumMap[8] = {
    {0, 36, "Kick Drum"},      // Pad 0 -> C2 (36)
    {1, 38, "Snare Drum"},     // Pad 1 -> D2 (38)
    {2, 42, "Closed Hi-Hat"},  // Pad 2 -> F#2 (42)
    {3, 46, "Open Hi-Hat"},    // Pad 3 -> A#2 (46)
    {4, 49, "Crash Cymbal"},   // Pad 4 -> C#3 (49)
    {5, 51, "Ride Cymbal"},    // Pad 5 -> D#3 (51)
    {6, 47, "Low Tom"},        // Pad 6 -> B2 (47)
    {7, 50, "High Tom"}        // Pad 7 -> D3 (50)
};

/**
 * @brief [AI GENERATED] Generates Rush E based on actual virtual piano notation.
 */
std::vector<MidiMessage> MidiInput::generateRushE() const {
    std::vector<MidiMessage> messages;
    double currentTime = 0.0;
    const double sixteenth = 0.125; // 16th note
    const double eighth = 0.25;     // 8th note
    const double quarter = 0.5;     // quarter note
    
    // Virtual piano to MIDI mapping (based on standard layout)
    // 6u = E4 (64), 3u = E3 (52), 80u = E5 (76), etc.
    
    // Opening section: [6u] [80u]u[3u] [80u]u[6u] [80u]u[3u] [80u]u[6u]i[80u]Y[3u] [80p] [6s] [80] 3 [80s]
    const int opening[] = {64, 76, 76, 52, 76, 76, 64, 76, 76, 52, 76, 76, 64, 77, 76, 79, 52, 83, 66, 76, 52, 76, 66};
    const double openingDur[] = {sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth, sixteenth};
    
    for (size_t i = 0; i < sizeof(opening)/sizeof(opening[0]); ++i) {
        messages.push_back({opening[i], openingDur[i], currentTime});
        currentTime += openingDur[i];
    }
    
    // [%d] [90d]d[%d]s[90a]d[6s] [80] 6 [80p]s[7a] [(ea]a[7I] [(ea] [0rWO]| [30u]|
    const int phrase1[] = {62, 74, 74, 62, 66, 69, 62, 66, 76, 64, 76, 83, 66, 69, 71, 69, 71, 77, 69, 71, 79, 52};
    for (int note : phrase1) {
        messages.push_back({note, sixteenth, currentTime});
        currentTime += sixteenth;
    }
    
    currentTime += quarter; // Rest
    
    // Second occurrence: [6u] [80u]u[3u] [80u]u[6u] [80u]u[3u]i[80u]Y[6u] [80p]s[3f] [80j] [6l] [80] 3 [80l]
    const int phrase2[] = {64, 76, 76, 52, 76, 76, 64, 76, 76, 52, 77, 76, 79, 64, 76, 83, 66, 52, 65, 76, 74, 64, 76, 52, 76, 76};
    for (int note : phrase2) {
        messages.push_back({note, sixteenth, currentTime});
        currentTime += sixteenth;
    }
    
    // [%z]l[90k]z[6l] [80] [7k]j[e(H]k[8j] [0e] [9g]f[qed]s[0a] [Wr]O[etup]| [6e]|
    const int phrase3[] = {48, 76, 75, 48, 76, 76, 75, 74, 71, 75, 74, 64, 67, 65, 62, 66, 69, 79, 83, 52};
    for (int note : phrase3) {
        messages.push_back({note, sixteenth, currentTime});
        currentTime += sixteenth;
    }
    
    currentTime += quarter; // Rest
    
    // Middle section with more complex patterns
    // [9d] [qe]S[9d] [qef] [9g] [qe] 9 [qed] [8f] [0e] 8 [0es] [8f] [0e] 8 [0es]
    const int middle1[] = {74, 64, 78, 74, 65, 67, 64, 74, 62, 74, 65, 64, 66, 65, 64, 66};
    for (int note : middle1) {
        messages.push_back({note, eighth, currentTime});
        currentTime += eighth;
    }
    
    // [7a] [(e] 7 [e(P] [7a] [(es] [7d] [(ea] [8s] [0e] 8 [0e] 8 [0e] [8s] [0es]
    const int middle2[] = {69, 64, 69, 83, 69, 66, 62, 69, 66, 64, 66, 64, 66, 64, 66, 66};
    for (int note : middle2) {
        messages.push_back({note, eighth, currentTime});
        currentTime += eighth;
    }
    
    // Higher register section: [9p] [qep]p[6p] [qep]p[9p] [qep]p[6p] [qep]p[9p]P[qep]O[6p] [qed] [9g] [qe] 6 [qeg]
    const int higher1[] = {83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 87, 83, 79, 83, 62, 67, 83, 83, 67};
    for (int note : higher1) {
        messages.push_back({note, sixteenth, currentTime});
        currentTime += sixteenth;
    }
    
    // Rapid virtuosic ending section
    // [wd] [Eyd]d[9d] [Eyd]d[wd] [Eyd]d[9d] [Eyd]d[wd]D[Eyd]S[9d] [Eyh] [wJ] [Ey] 9 [yEJ]
    const int virtuosic[] = {86, 86, 86, 74, 86, 86, 86, 86, 86, 74, 86, 86, 86, 89, 86, 78, 74, 71, 74, 71, 74, 71, 74, 71};
    for (int note : virtuosic) {
        messages.push_back({note, sixteenth * 0.75, currentTime}); // Faster tempo
        currentTime += sixteenth * 0.75;
    }
    
    // Final ascending run to end
    for (int note = 60; note <= 96; note += 2) {
        messages.push_back({note, sixteenth * 0.5, currentTime});
        currentTime += sixteenth * 0.5;
    }
    
    // Final E chord
    messages.push_back({64, quarter, currentTime}); // E4
    messages.push_back({76, quarter, currentTime}); // E5
    messages.push_back({88, quarter, currentTime}); // E6
    
    return messages;
}

/**
 * @brief [AI GENERATED] Generates Für Elise with accurate melody and left hand accompaniment.
 */
std::vector<MidiMessage> MidiInput::generateFurElise() const {
    std::vector<MidiMessage> messages;
    double currentTime = 0.0;
    const double eighth = 0.25;  // Eighth note duration
    const double quarter = 0.5;  // Quarter note duration
    
    // First section: Right hand melody with left hand bass
    // Measure 1-2: E-D#-E-D#-E-B-D-C-A
    const int rightHand1[] = {76, 75, 76, 75, 76, 71, 74, 72, 69}; // E5-D#5-E5-D#5-E5-B4-D5-C5-A4
    const double rhDuration1[] = {eighth, eighth, eighth, eighth, eighth, eighth, eighth, eighth, quarter + eighth};
    
    // Left hand accompaniment - bass notes and chords
    const int leftHand1[] = {0, 0, 0, 0, 0, 0, 0, 45, 52};  // Rest, rest, rest, rest, rest, rest, rest, A2, E3
    const double lhDuration1[] = {eighth, eighth, eighth, eighth, eighth, eighth, eighth, eighth, quarter + eighth};
    
    for (size_t i = 0; i < 9; i++) {
        // Right hand melody
        messages.push_back({rightHand1[i], rhDuration1[i], currentTime});
        // Left hand bass (if not rest)
        if (leftHand1[i] > 0) {
            messages.push_back({leftHand1[i], lhDuration1[i], currentTime});
        }
        currentTime += rhDuration1[i];
    }
    
    // Rest measure
    currentTime += quarter;
    
    // Measure 3-4: C-E-A-B
    const int rightHand2[] = {48, 52, 57, 59}; // C3-E3-A3-B3 (lower register for bass accompaniment feeling)
    const double rhDuration2[] = {eighth, eighth, eighth, quarter + eighth};
    const int leftHand2[] = {36, 40, 45, 47}; // C2-E2-A2-B2 (bass octave)
    
    for (size_t i = 0; i < 4; i++) {
        messages.push_back({rightHand2[i], rhDuration2[i], currentTime});
        messages.push_back({leftHand2[i], rhDuration2[i], currentTime});
        currentTime += rhDuration2[i];
    }
    
    // Rest
    currentTime += quarter;
    
    // Measure 5-6: E-G#-B-C (building back to melody)
    const int rightHand3[] = {52, 56, 59, 60}; // E3-G#3-B3-C4
    const double rhDuration3[] = {eighth, eighth, eighth, quarter + eighth};
    const int leftHand3[] = {40, 44, 47, 48}; // E2-G#2-B2-C3
    
    for (size_t i = 0; i < 4; i++) {
        messages.push_back({rightHand3[i], rhDuration3[i], currentTime});
        messages.push_back({leftHand3[i], rhDuration3[i], currentTime});
        currentTime += rhDuration3[i];
    }
    
    // Repeat opening phrase with accompaniment
    for (size_t i = 0; i < 9; i++) {
        messages.push_back({rightHand1[i], rhDuration1[i], currentTime});
        if (leftHand1[i] > 0) {
            messages.push_back({leftHand1[i], lhDuration1[i], currentTime});
        }
        currentTime += rhDuration1[i];
    }
    
    // Additional phrase for completeness
    const int rightHand4[] = {69, 72, 76, 69}; // A4-C5-E5-A4
    const double rhDuration4[] = {quarter, quarter, quarter, quarter};
    const int leftHand4[] = {45, 48, 52, 45}; // A2-C3-E3-A2
    
    for (size_t i = 0; i < 4; i++) {
        messages.push_back({rightHand4[i], rhDuration4[i], currentTime});
        messages.push_back({leftHand4[i], rhDuration4[i], currentTime});
        currentTime += rhDuration4[i];
    }
    
    return messages;
}

/**
 * @brief [AI GENERATED] Generates Beethoven's 5th Symphony opening motif.
 */
std::vector<MidiMessage> MidiInput::generateBeethoven5th() const {
    std::vector<MidiMessage> messages;
    double currentTime = 0.0;
    
    // Famous opening motif: G-G-G-Eb (short-short-short-long) - lower octave
    const int motif1[] = {55, 55, 55, 51}; // G3-G3-G3-Eb3
    const double duration1[] = {0.25, 0.25, 0.25, 1.0};
    
    for (size_t i = 0; i < 4; i++) {
        messages.push_back({motif1[i], duration1[i], currentTime});
        currentTime += duration1[i];
    }
    
    // Rest
    currentTime += 0.5;
    
    // Second motif: F-F-F-D (one tone lower)
    const int motif2[] = {53, 53, 53, 50}; // F3-F3-F3-D3
    const double duration2[] = {0.25, 0.25, 0.25, 1.0};
    
    for (size_t i = 0; i < 4; i++) {
        messages.push_back({motif2[i], duration2[i], currentTime});
        currentTime += duration2[i];
    }
    
    // Development of the motif - ascending from bass to treble
    const int development[] = {43, 48, 55, 60, 67, 72, 79, 84}; // Ascending across octaves
    for (int note : development) {
        messages.push_back({note, 0.5, currentTime});
        currentTime += 0.5;
    }
    
    // Return to main motif in higher octave
    const int motif3[] = {67, 67, 67, 63}; // G4-G4-G4-Eb4
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
    
    // Opening melody: B-D-E-F#-G-A-B-A-G-F#-E-D-B - start lower
    const int melody[] = {47, 50, 52, 54, 55, 57, 59, 57, 55, 54, 52, 50, 47}; // B2 to B3 and back
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
    
    // Fourth iteration - fastest with harmonies across multiple octaves
    noteDuration = 0.15;
    for (size_t i = 0; i < 13; i++) {
        messages.push_back({melody[i], noteDuration, currentTime});
        messages.push_back({melody[i] + 12, noteDuration, currentTime}); // Octave harmony
        messages.push_back({melody[i] + 24, noteDuration, currentTime}); // Two octave harmony
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
    
    // Spring opening melody (simplified version) - violin range
    // "Spring has come" motif
    const int springMelody[] = {
        79, 77, 76, 77, 79, 81, 79, 77, 76, 74, 72, 71, 72, 74, 76, 77, 79
    }; // G5 down to C5 and back up - proper violin range
    
    const double durations[] = {
        0.3, 0.3, 0.3, 0.3, 0.6, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.9
    };
    
    for (size_t i = 0; i < 17; i++) {
        messages.push_back({springMelody[i], durations[i], currentTime});
        currentTime += durations[i];
    }
    
    // Ornamental flourishes - high violin range
    const int flourish[] = {91, 89, 88, 86, 84, 83, 81, 79, 77, 76, 74, 72, 71, 69, 67};
    for (int note : flourish) {
        messages.push_back({note, 0.1, currentTime});
        currentTime += 0.08;
    }
    
    // Gentle ascending passage - moderate to high range
    const int ascending[] = {67, 69, 71, 72, 74, 76, 77, 79, 81, 83, 84, 86, 88, 89, 91};
    for (int note : ascending) {
        messages.push_back({note, 0.2, currentTime});
        currentTime += 0.15;
    }
    
    // Final resolution - descending to comfortable range
    const int resolution[] = {91, 86, 83, 79, 76, 72, 67};
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
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, msg.note, 80, msg.startTime));
        
        // Key up event at end time
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, msg.note, 0, msg.startTime + msg.duration));
    }
    
    // Sort by timestamp to ensure proper ordering
    std::sort(keyEvents.begin(), keyEvents.end(), 
              [](const KeyEvent& a, const KeyEvent& b) { return a.timestamp < b.timestamp; });
    
    return keyEvents;
}

/**
 * @brief [AI GENERATED] Generates Für Elise with realistic key press/release events, velocity, and proper accompaniment.
 */
std::vector<KeyEvent> MidiInput::generateFurEliseKeys() const {
    std::vector<KeyEvent> keyEvents;
    double currentTime = 0.0;
    const double eighth = 0.25;
    const double quarter = 0.5;
    
    struct NoteData {
        int note;
        double duration;
        int velocity;
        bool isLeftHand;
    };
    
    // Opening phrase with both hands
    const NoteData opening[] = {
        // Right hand melody
        {76, eighth, 65, false},   // E5 - gentle start
        {75, eighth, 60, false},   // D#5 - softer
        {76, eighth, 70, false},   // E5 - slightly stronger
        {75, eighth, 60, false},   // D#5 - softer
        {76, eighth, 75, false},   // E5 - stronger
        {71, eighth, 70, false},   // B4 - moderate
        {74, eighth, 72, false},   // D5 - moderate
        {72, eighth, 68, false},   // C5 - gentle
        {69, quarter + eighth, 80, false}, // A4 - strong ending
        
        // Left hand bass (delayed entrance)
        {45, eighth, 50, true},    // A2 - soft bass (starts with C5)
        {52, quarter + eighth, 55, true} // E3 - bass support
    };
    
    // Generate opening with proper timing
    double rightHandTime = currentTime;
    double leftHandTime = currentTime + (eighth * 7); // Left hand enters with C5
    
    // Right hand melody
    for (int i = 0; i < 9; i++) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, opening[i].note, opening[i].velocity, rightHandTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, opening[i].note, 0, rightHandTime + opening[i].duration));
        rightHandTime += opening[i].duration;
    }
    
    // Left hand bass
    for (int i = 9; i < 11; i++) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, opening[i].note, opening[i].velocity, leftHandTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, opening[i].note, 0, leftHandTime + opening[i].duration));
        leftHandTime += opening[i].duration;
    }
    
    currentTime = std::max(rightHandTime, leftHandTime) + quarter;
    
    // Second phrase: Both hands together
    const NoteData phrase2[] = {
        // Right hand (bass register melody)
        {48, eighth, 55, false},   // C3
        {52, eighth, 60, false},   // E3
        {57, eighth, 65, false},   // A3
        {59, quarter + eighth, 75, false}, // B3
        
        // Left hand (deeper bass)
        {36, eighth, 45, true},    // C2
        {40, eighth, 45, true},    // E2
        {45, eighth, 50, true},    // A2
        {47, quarter + eighth, 55, true}  // B2
    };
    
    // Both hands together
    for (int i = 0; i < 4; i++) {
        // Right hand
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, phrase2[i].note, phrase2[i].velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, phrase2[i].note, 0, currentTime + phrase2[i].duration));
        // Left hand
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, phrase2[i + 4].note, phrase2[i + 4].velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, phrase2[i + 4].note, 0, currentTime + phrase2[i + 4].duration));
        currentTime += phrase2[i].duration;
    }
    
    currentTime += quarter;
    
    // Third phrase: Building intensity
    const NoteData phrase3[] = {
        // Right hand
        {52, eighth, 58, false},   // E3
        {56, eighth, 62, false},   // G#3
        {59, eighth, 70, false},   // B3
        {60, quarter + eighth, 85, false}, // C4 - forte
        
        // Left hand
        {40, eighth, 48, true},    // E2
        {44, eighth, 52, true},    // G#2
        {47, eighth, 58, true},    // B2
        {48, quarter + eighth, 65, true}   // C3
    };
    
    for (int i = 0; i < 4; i++) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, phrase3[i].note, phrase3[i].velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, phrase3[i].note, 0, currentTime + phrase3[i].duration));
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, phrase3[i + 4].note, phrase3[i + 4].velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, phrase3[i + 4].note, 0, currentTime + phrase3[i + 4].duration));
        currentTime += phrase3[i].duration;
    }
    
    // Repeat opening phrase with more expression
    rightHandTime = currentTime;
    leftHandTime = currentTime + (eighth * 7);
    
    for (int i = 0; i < 9; i++) {
        int adjustedVelocity = opening[i].velocity + 8; // More expressive
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, opening[i].note, adjustedVelocity, rightHandTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, opening[i].note, 0, rightHandTime + opening[i].duration));
        rightHandTime += opening[i].duration;
    }
    
    for (int i = 9; i < 11; i++) {
        int adjustedVelocity = opening[i].velocity + 5;
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, opening[i].note, adjustedVelocity, leftHandTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, opening[i].note, 0, leftHandTime + opening[i].duration));
        leftHandTime += opening[i].duration;
    }
    
    return keyEvents;
}

/**
 * @brief [AI GENERATED] Generate Rush E with velocity dynamics based on virtual piano notation.
 */
std::vector<KeyEvent> MidiInput::generateRushEKeys() const {
    std::vector<KeyEvent> keyEvents;
    double currentTime = 0.0;
    const double sixteenth = 0.125;
    const double eighth = 0.25;
    const double quarter = 0.5;
    
    // Opening section with building intensity: [6u] [80u]u[3u] [80u]u[6u] [80u]u[3u] [80u]u[6u]i[80u]Y[3u] [80p] [6s] [80] 3 [80s]
    struct RushNote {
        int note;
        double duration;
        int velocity;
    };
    
    const RushNote opening[] = {
        {64, sixteenth, 80},  // 6u - E4 moderate start
        {76, sixteenth, 85},  // 80u - E5 building
        {76, sixteenth, 90},  // u - repeated E5 stronger
        {52, sixteenth, 75},  // 3u - E3 bass note
        {76, sixteenth, 92},  // 80u - E5 continuing build
        {76, sixteenth, 95},  // u - E5 stronger
        {64, sixteenth, 85},  // 6u - E4 
        {76, sixteenth, 100}, // 80u - E5 forte
        {76, sixteenth, 102}, // u - E5 fortissimo
        {52, sixteenth, 80},  // 3u - E3 bass
        {76, sixteenth, 105}, // 80u - E5 maximum
        {76, sixteenth, 110}, // u - E5 intense
        {64, sixteenth, 100}, // 6u - E4
        {77, sixteenth, 112}, // i - F5 higher pitch
        {76, sixteenth, 115}, // 80u - E5 climax
        {79, sixteenth, 110}, // Y - G5
        {52, sixteenth, 90},  // 3u - E3
        {83, sixteenth, 105}, // p - C6 high register
        {66, sixteenth, 95},  // s - F#4
        {76, sixteenth, 100}, // 80 - E5
        {52, sixteenth, 85},  // 3 - E3
        {76, sixteenth, 90},  // 80s - E5
        {66, sixteenth, 95}   // s - F#4
    };
    
    for (const auto& note : opening) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += note.duration;
    }
    
    // First phrase with varying dynamics
    const RushNote phrase1[] = {
        {62, sixteenth, 100}, // %d - D4
        {74, sixteenth, 105}, // 90d - D5
        {74, sixteenth, 110}, // d - D5 stronger
        {62, sixteenth, 95},  // %d - D4
        {66, sixteenth, 100}, // s - F#4
        {69, sixteenth, 105}, // 90a - A4
        {62, sixteenth, 90},  // d - D4
        {66, sixteenth, 95},  // 6s - F#4
        {76, sixteenth, 100}, // 80 - E5
        {64, sixteenth, 85},  // 6 - E4
        {76, sixteenth, 90},  // 80p - E5
        {83, sixteenth, 95},  // s - C6
        {69, sixteenth, 100}, // 7a - A4
        {71, sixteenth, 105}, // (ea - B4
        {69, sixteenth, 110}, // a - A4
        {77, sixteenth, 115}, // 7I - F5
        {69, sixteenth, 100}, // (ea - A4
        {79, sixteenth, 105}, // 0rWO - G5
        {52, sixteenth, 80}   // 30u - E3
    };
    
    for (const auto& note : phrase1) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += note.duration;
    }
    
    currentTime += quarter; // Rest
    
    // High-intensity virtuosic section
    const RushNote virtuosic[] = {
        {86, sixteenth * 0.75, 120}, // wd - High register
        {86, sixteenth * 0.75, 122}, // Eyd - repeated
        {86, sixteenth * 0.75, 125}, // d - building
        {74, sixteenth * 0.75, 118}, // 9d - D5
        {86, sixteenth * 0.75, 127}, // Eyd - maximum velocity
        {86, sixteenth * 0.75, 127}, // d - fortissimo
        {89, sixteenth * 0.75, 127}, // D - even higher
        {78, sixteenth * 0.75, 120}, // S - G#5
        {74, sixteenth * 0.75, 115}, // d - D5
        {71, sixteenth * 0.75, 110}  // h - B4
    };
    
    for (const auto& note : virtuosic) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += note.duration;
    }
    
    // Final ascending run with maximum intensity
    for (int note = 60; note <= 96; note += 2) {
        int velocity = 100 + (note - 60); // Increasing velocity with pitch
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note, std::min(velocity, 127), currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note, 0, currentTime + sixteenth * 0.4));
        currentTime += sixteenth * 0.4;
    }
    
    // Final E chord with maximum impact
    keyEvents.push_back(createPianoEvent(KeyState::KeyDown, 64, 127, currentTime)); // E4
    keyEvents.push_back(createPianoEvent(KeyState::KeyDown, 76, 127, currentTime)); // E5
    keyEvents.push_back(createPianoEvent(KeyState::KeyDown, 88, 127, currentTime)); // E6
    keyEvents.push_back(createPianoEvent(KeyState::KeyUp, 64, 0, currentTime + quarter));
    keyEvents.push_back(createPianoEvent(KeyState::KeyUp, 76, 0, currentTime + quarter));
    keyEvents.push_back(createPianoEvent(KeyState::KeyUp, 88, 0, currentTime + quarter));
    
    return keyEvents;
}

std::vector<KeyEvent> MidiInput::generateBeethoven5thKeys() const {
    std::vector<KeyEvent> keyEvents;
    double currentTime = 0.0;
    const double quarter = 0.5;
    const double whole = 2.0;
    
    struct BeethovenNote {
        int note;
        double duration;
        int velocity;
    };
    
    // Famous opening motif: G-G-G-Eb (short-short-short-long) with dramatic dynamics
    const BeethovenNote motif1[] = {
        {55, quarter, 110}, // G3 - forte, dramatic
        {55, quarter, 115}, // G3 - stronger
        {55, quarter, 120}, // G3 - fortissimo
        {51, whole, 127}    // Eb3 - maximum drama
    };
    
    for (const auto& note : motif1) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += note.duration;
    }
    
    currentTime += quarter; // Rest
    
    // Second motif: F-F-F-D (one tone lower) with building tension
    const BeethovenNote motif2[] = {
        {53, quarter, 105}, // F3 - forte
        {53, quarter, 110}, // F3 - stronger
        {53, quarter, 115}, // F3 - fortissimo
        {50, whole, 120}    // D3 - powerful ending
    };
    
    for (const auto& note : motif2) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += note.duration;
    }
    
    // Development section - ascending from bass to treble with increasing intensity
    const BeethovenNote development[] = {
        {43, quarter, 90},  // G2 - bass foundation
        {48, quarter, 95},  // C3 - building
        {55, quarter, 100}, // G3 - moderate forte
        {60, quarter, 105}, // C4 - stronger
        {67, quarter, 110}, // G4 - forte
        {72, quarter, 115}, // C5 - fortissimo
        {79, quarter, 120}, // G5 - powerful
        {84, quarter, 127}  // C6 - climax
    };
    
    for (const auto& note : development) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += quarter;
    }
    
    // Return to main motif in higher octave with maximum drama
    const BeethovenNote motif3[] = {
        {67, quarter, 125}, // G4 - fortissimo
        {67, quarter, 127}, // G4 - maximum
        {67, quarter, 127}, // G4 - peak intensity
        {63, whole, 127}    // Eb4 - dramatic conclusion
    };
    
    for (const auto& note : motif3) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += note.duration;
    }
    
    return keyEvents;
}

std::vector<KeyEvent> MidiInput::generateHallOfMountainKingKeys() const {
    std::vector<KeyEvent> keyEvents;
    double currentTime = 0.0;
    
    struct HallNote {
        int note;
        double duration;
        int velocity;
    };
    
    // Opening melody: B-D-E-F#-G-A-B-A-G-F#-E-D-B with accelerando and crescendo
    const int melody[] = {47, 50, 52, 54, 55, 57, 59, 57, 55, 54, 52, 50, 47}; // B2 to B3 and back
    
    // First iteration - slow and mysterious (pp)
    double noteDuration = 0.4;
    int velocity = 45; // pianissimo
    for (int note : melody) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note, velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note, 0, currentTime + noteDuration));
        currentTime += noteDuration;
    }
    
    // Second iteration - faster and louder (mp)
    noteDuration = 0.3;
    velocity = 65; // mezzo-piano
    for (int note : melody) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note, velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note, 0, currentTime + noteDuration));
        currentTime += noteDuration;
    }
    
    // Third iteration - even faster and louder (mf)
    noteDuration = 0.2;
    velocity = 85; // mezzo-forte
    for (int note : melody) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note, velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note, 0, currentTime + noteDuration));
        currentTime += noteDuration;
    }
    
    // Fourth iteration - fastest with multi-octave harmonies (ff)
    noteDuration = 0.15;
    velocity = 110; // forte
    for (size_t i = 0; i < 13; i++) {
        // Original melody
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, melody[i], velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, melody[i], 0, currentTime + noteDuration));
        
        // Octave harmony
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, melody[i] + 12, velocity - 10, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, melody[i] + 12, 0, currentTime + noteDuration));
        
        // Two octave harmony for climax
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, melody[i] + 24, velocity - 15, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, melody[i] + 24, 0, currentTime + noteDuration));
        
        currentTime += noteDuration;
        velocity = std::min(127, velocity + 1); // Crescendo to fortissimo
    }
    
    // Final dramatic chord
    keyEvents.push_back(createPianoEvent(KeyState::KeyDown, 47, 127, currentTime)); // B2
    keyEvents.push_back(createPianoEvent(KeyState::KeyDown, 59, 127, currentTime)); // B3
    keyEvents.push_back(createPianoEvent(KeyState::KeyDown, 71, 127, currentTime)); // B4
    keyEvents.push_back(createPianoEvent(KeyState::KeyDown, 83, 127, currentTime)); // B5
    
    keyEvents.push_back(createPianoEvent(KeyState::KeyUp, 47, 0, currentTime + 1.0));
    keyEvents.push_back(createPianoEvent(KeyState::KeyUp, 59, 0, currentTime + 1.0));
    keyEvents.push_back(createPianoEvent(KeyState::KeyUp, 71, 0, currentTime + 1.0));
    keyEvents.push_back(createPianoEvent(KeyState::KeyUp, 83, 0, currentTime + 1.0));
    
    return keyEvents;
}

std::vector<KeyEvent> MidiInput::generateVivaldiSpringKeys() const {
    std::vector<KeyEvent> keyEvents;
    double currentTime = 0.0;
    
    struct VivaldiNote {
        int note;
        double duration;
        int velocity;
    };
    
    // Spring opening melody with baroque ornamentation - violin range
    const VivaldiNote springMelody[] = {
        {79, 0.3, 75},  // G5 - gentle spring opening
        {77, 0.3, 70},  // F5 - dolce
        {76, 0.3, 72},  // E5 - graceful
        {77, 0.3, 74},  // F5 - rising
        {79, 0.6, 80},  // G5 - sustained, blooming
        {81, 0.3, 85},  // A5 - bright spring
        {79, 0.3, 82},  // G5 - flowing
        {77, 0.3, 78},  // F5 - gentle
        {76, 0.3, 75},  // E5 - graceful descent
        {74, 0.3, 72},  // D5 - continuing down
        {72, 0.3, 70},  // C5 - soft
        {71, 0.3, 68},  // B4 - delicate
        {72, 0.3, 72},  // C5 - rising again
        {74, 0.3, 75},  // D5 - building
        {76, 0.3, 78},  // E5 - growing
        {77, 0.3, 80},  // F5 - ascending
        {79, 0.9, 85}   // G5 - joyful conclusion
    };
    
    for (const auto& note : springMelody) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += note.duration;
    }
    
    // Ornamental flourishes - rapid high violin-like passages
    const VivaldiNote flourish[] = {
        {91, 0.1, 90},  // G6 - brilliant high notes
        {89, 0.1, 88},  // F6 - sparkling
        {88, 0.1, 86},  // E6 - virtuosic
        {86, 0.1, 84},  // D6 - dazzling
        {84, 0.1, 82},  // C6 - flowing
        {83, 0.1, 80},  // B5 - graceful
        {81, 0.1, 78},  // A5 - descending
        {79, 0.1, 76},  // G5 - continuing
        {77, 0.1, 74},  // F5 - gentle
        {76, 0.1, 72},  // E5 - soft
        {74, 0.1, 70},  // D5 - delicate
        {72, 0.1, 68},  // C5 - whisper-like
        {71, 0.1, 66},  // B4 - fading
        {69, 0.1, 64},  // A4 - gentle
        {67, 0.1, 62}   // G4 - soft landing
    };
    
    for (const auto& note : flourish) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += 0.08; // Slightly overlapping for legato effect
    }
    
    // Gentle ascending passage - spring awakening
    const VivaldiNote ascending[] = {
        {67, 0.2, 65},  // G4 - awakening
        {69, 0.2, 68},  // A4 - growing
        {71, 0.2, 70},  // B4 - rising
        {72, 0.2, 72},  // C5 - building
        {74, 0.2, 75},  // D5 - ascending
        {76, 0.2, 78},  // E5 - climbing
        {77, 0.2, 80},  // F5 - soaring
        {79, 0.2, 82},  // G5 - reaching
        {81, 0.2, 85},  // A5 - high
        {83, 0.2, 88},  // B5 - higher
        {84, 0.2, 90},  // C6 - brilliant
        {86, 0.2, 92},  // D6 - radiant
        {88, 0.2, 95},  // E6 - glorious
        {89, 0.2, 98},  // F6 - triumphant
        {91, 0.2, 100}  // G6 - peak of spring joy
    };
    
    for (const auto& note : ascending) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += 0.15; // Slightly faster tempo
    }
    
    // Final resolution - peaceful spring conclusion
    const VivaldiNote resolution[] = {
        {91, 0.4, 95},  // G6 - glorious
        {86, 0.4, 90},  // D6 - settling
        {83, 0.4, 85},  // B5 - peaceful
        {79, 0.4, 80},  // G5 - gentle
        {76, 0.4, 75},  // E5 - calm
        {72, 0.4, 70},  // C5 - restful
        {67, 0.8, 65}   // G4 - serene conclusion
    };
    
    for (const auto& note : resolution) {
        keyEvents.push_back(createPianoEvent(KeyState::KeyDown, note.note, note.velocity, currentTime));
        keyEvents.push_back(createPianoEvent(KeyState::KeyUp, note.note, 0, currentTime + note.duration));
        currentTime += 0.3;
    }
    
    return keyEvents;
}

/**
 * @brief [AI GENERATED] Generate drum pattern using the 8 pads.
 */
std::vector<KeyEvent> MidiInput::generateDrumPattern() const {
    std::vector<KeyEvent> keyEvents;
    double currentTime = 0.0;
    const double beatDuration = 0.5; // 120 BPM
    
    // Basic rock beat pattern
    for (int bar = 0; bar < 8; ++bar) {
        double barStart = bar * beatDuration * 4;
        
        // Beat 1: Kick + Hi-Hat
        keyEvents.push_back(createDrumEvent(KeyState::KeyDown, 0, 100, barStart)); // Kick
        keyEvents.push_back(createDrumEvent(KeyState::KeyUp, 0, 0, barStart + 0.1));
        keyEvents.push_back(createDrumEvent(KeyState::KeyDown, 2, 80, barStart)); // Closed Hi-Hat
        keyEvents.push_back(createDrumEvent(KeyState::KeyUp, 2, 0, barStart + 0.1));
        
        // Beat 2: Hi-Hat
        keyEvents.push_back(createDrumEvent(KeyState::KeyDown, 2, 60, barStart + beatDuration));
        keyEvents.push_back(createDrumEvent(KeyState::KeyUp, 2, 0, barStart + beatDuration + 0.1));
        
        // Beat 3: Snare + Hi-Hat
        keyEvents.push_back(createDrumEvent(KeyState::KeyDown, 1, 110, barStart + beatDuration * 2)); // Snare
        keyEvents.push_back(createDrumEvent(KeyState::KeyUp, 1, 0, barStart + beatDuration * 2 + 0.1));
        keyEvents.push_back(createDrumEvent(KeyState::KeyDown, 2, 80, barStart + beatDuration * 2));
        keyEvents.push_back(createDrumEvent(KeyState::KeyUp, 2, 0, barStart + beatDuration * 2 + 0.1));
        
        // Beat 4: Hi-Hat + occasional crash
        keyEvents.push_back(createDrumEvent(KeyState::KeyDown, 2, 60, barStart + beatDuration * 3));
        keyEvents.push_back(createDrumEvent(KeyState::KeyUp, 2, 0, barStart + beatDuration * 3 + 0.1));
        
        if (bar == 3 || bar == 7) { // Crash on bars 4 and 8
            keyEvents.push_back(createDrumEvent(KeyState::KeyDown, 4, 100, barStart + beatDuration * 3.5));
            keyEvents.push_back(createDrumEvent(KeyState::KeyUp, 4, 0, barStart + beatDuration * 3.5 + 0.3));
        }
    }
    
    return keyEvents;
}

/**
 * @brief [AI GENERATED] Generate mixed performance with piano and drums.
 */
std::vector<KeyEvent> MidiInput::generateMixedPerformance() const {
    std::vector<KeyEvent> keyEvents;
    
    // Get piano melody (Für Elise)
    auto pianoEvents = generateFurEliseKeys();
    keyEvents.insert(keyEvents.end(), pianoEvents.begin(), pianoEvents.end());
    
    // Get drum pattern
    auto drumEvents = generateDrumPattern();
    keyEvents.insert(keyEvents.end(), drumEvents.begin(), drumEvents.end());
    
    // Sort by timestamp
    std::sort(keyEvents.begin(), keyEvents.end(), 
              [](const KeyEvent& a, const KeyEvent& b) { return a.timestamp < b.timestamp; });
    
    return keyEvents;
}

/**
 * @brief [AI GENERATED] Create piano key event.
 */
KeyEvent MidiInput::createPianoEvent(KeyState state, int note, int velocity, double timestamp, int channel) const {
    return {DeviceType::Piano, state, note, velocity, channel, timestamp};
}

/**
 * @brief [AI GENERATED] Create drum pad event.
 */
KeyEvent MidiInput::createDrumEvent(KeyState state, int padNumber, int velocity, double timestamp, int channel) const {
    int midiNote = getMidiNoteForPad(padNumber);
    return {DeviceType::DrumPad, state, midiNote, velocity, channel, timestamp};
}

/**
 * @brief [AI GENERATED] Get drum mapping for pad number.
 */
const DrumMapping& MidiInput::getDrumMapping(int padNumber) {
    if (padNumber >= 0 && padNumber < 8) {
        return drumMap[padNumber];
    }
    return drumMap[0]; // Default to kick drum
}

/**
 * @brief [AI GENERATED] Get MIDI note for drum pad.
 */
int MidiInput::getMidiNoteForPad(int padNumber) {
    return getDrumMapping(padNumber).midiNote;
}

/**
 * @brief [AI GENERATED] Get drum name for pad.
 */
const char* MidiInput::getDrumName(int padNumber) {
    return getDrumMapping(padNumber).name;
}

/**
 * @brief [AI GENERATED] Generates current demo (updated Rush E based on virtual piano notation).
 */
std::vector<MidiMessage> MidiInput::generateDemo() const {
    return generateRushE(); // Default to updated Rush E implementation
}