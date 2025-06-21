#include "MidiInput.h"

/**
 * @brief [AI GENERATED] Generates the opening phrase of Fur Elise with
 *        extended key presses.
 */
std::vector<MidiMessage> MidiInput::generateDemo() const {
    const int kNotes[] = {76, 75, 76, 75, 76, 71, 74, 73, 69};
    const double kDurations[] = {0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 1.6};
    std::vector<MidiMessage> messages;
    for (size_t i = 0; i < sizeof(kNotes) / sizeof(kNotes[0]); ++i) {
        messages.push_back({kNotes[i], kDurations[i]});
    }
    return messages;
}
