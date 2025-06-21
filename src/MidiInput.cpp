#include "../include/MidiInput.h"
#include <vector>
#include <cmath>

/**
 * @brief [AI GENERATED] Generates Rush E opening section for comprehensive
 *        piano sound testing with rapid scales, arpeggios, and various dynamics.
 */
std::vector<MidiMessage> MidiInput::generateDemo() const {
    std::vector<MidiMessage> messages;
    double dCurrentTime = 0.0;
    
    // Rush E opening: Rapid E notes (multiple octaves)
    const int eNotes[] = {64, 76, 88, 76, 64, 76, 88, 76}; // E in different octaves
    const double rapidDuration = 0.1;
    for (int i = 0; i < 8; ++i) {
        messages.push_back({eNotes[i], rapidDuration * 2, dCurrentTime});
        dCurrentTime += rapidDuration;
    }
    
    // Fast ascending C major scale
    const int ascendingScale[] = {60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83, 84};
    const double scaleDuration = 0.08;
    for (int note : ascendingScale) {
        messages.push_back({note, scaleDuration * 1.5, dCurrentTime});
        dCurrentTime += scaleDuration;
    }
    
    // Rapid descending scale
    for (int i = 14; i >= 0; --i) {
        messages.push_back({ascendingScale[i], scaleDuration * 1.5, dCurrentTime});
        dCurrentTime += scaleDuration;
    }
    
    // Arpeggios - C major triad in different inversions
    const int arpeggio1[] = {48, 52, 55, 60, 64, 67, 72, 76, 79, 84}; // C major arpeggio
    const double arpeggioTime = 0.12;
    for (int note : arpeggio1) {
        messages.push_back({note, arpeggioTime * 2, dCurrentTime});
        dCurrentTime += arpeggioTime;
    }
    
    // Fast chromatic run
    const double chromaticTime = 0.06;
    for (int note = 60; note <= 84; ++note) {
        messages.push_back({note, chromaticTime * 1.5, dCurrentTime});
        dCurrentTime += chromaticTime;
    }
    
    // Octave jumps (bass to treble)
    const int octaveJumps[] = {36, 84, 40, 88, 43, 91, 36, 84};
    const double jumpTime = 0.15;
    for (int note : octaveJumps) {
        messages.push_back({note, jumpTime * 2, dCurrentTime});
        dCurrentTime += jumpTime;
    }
    
    // Rapid repeated notes (different pitches)
    const int repeatedNotes[] = {69, 69, 69, 69, 74, 74, 74, 74, 78, 78, 78, 78};
    const double repeatTime = 0.07;
    for (int note : repeatedNotes) {
        messages.push_back({note, repeatTime * 1.5, dCurrentTime});
        dCurrentTime += repeatTime;
    }
    
    // Final flourish - descending arpeggio
    const int finalArp[] = {96, 91, 88, 84, 79, 76, 72, 67, 64, 60, 55, 52, 48};
    const double finalTime = 0.1;
    for (int note : finalArp) {
        messages.push_back({note, finalTime * 2, dCurrentTime});
        dCurrentTime += finalTime;
    }
    
    return messages;
}
