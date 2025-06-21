#include "MidiInput.h"

/**
 * @brief [AI GENERATED] Generates a short melody.
 */
std::vector<MidiMessage> MidiInput::generateDemo() const {
    int tune[] = {64, 62, 60, 62, 64, 64, 64, 62, 62, 62, 64, 67, 67};
    const double dur = 0.5;
    std::vector<MidiMessage> messages;
    for (int n : tune) {
        messages.push_back({n, dur});
    }
    return messages;
}
