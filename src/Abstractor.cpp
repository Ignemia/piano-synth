#include "Abstractor.h"

/**
 * @brief [AI GENERATED] Perform MIDI to frequency conversion.
 */
std::vector<NoteEvent> Abstractor::convert(const std::vector<MidiMessage>& midi) const {
    std::vector<NoteEvent> events;
    for (const auto& msg : midi) {
        double freq = 440.0 * std::pow(2.0, (msg.note - 69) / 12.0);
        events.push_back({freq, msg.duration});
    }
    return events;
}
