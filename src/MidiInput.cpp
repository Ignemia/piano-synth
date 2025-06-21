#include "MidiInput.h"

/**
 * @brief [AI GENERATED] Generates the opening phrase of Fur Elise with
 *        extended key presses.
 */
std::vector<MidiMessage> MidiInput::generateDemo() const {
    // Opening phrase with a triad chord at the beginning.
    const int kNotes[] = {76, 80, 83, 75, 76, 75, 76, 71, 74, 73, 69};
    const double kDurations[] = {0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 1.6};

    std::vector<MidiMessage> messages;
    double dCurrentTime = 0.0;
    // First three notes start simultaneously to form a chord.
    for (int i = 0; i < 3; ++i) {
        messages.push_back({kNotes[i], kDurations[i], 0.0});
    }
    dCurrentTime += kDurations[0];
    // Remaining notes are played sequentially.
    for (size_t i = 3; i < sizeof(kNotes) / sizeof(kNotes[0]); ++i) {
        messages.push_back({kNotes[i], kDurations[i], dCurrentTime});
        dCurrentTime += kDurations[i];
    }
    return messages;
}
