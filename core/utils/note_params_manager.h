// [AI GENERATED]
#pragma once

#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

namespace PianoSynth {
namespace Utils {

/**
 * \brief [AI GENERATED] Per-note parameter structure loaded from JSON.
 */
struct NoteParams {
    double inharmonicity{0.0};
    int partials{0};
    double decay{0.0};
    double hammer{0.0};
    double volume{0.0};
    double tension{1.0};
    /**
     * \brief [AI GENERATED] Detuning offset in cents for this note.
     */
    double detune_cents{0.0};
};

/**
 * \brief [AI GENERATED] Manager for loading per-note parameters from JSON files.
 */
class NoteParamsManager {
public:
    /**
     * \brief [AI GENERATED] Load parameters from the given JSON file.
     * @param path Path to the JSON file.
     * @return true on success, false on failure.
     */
    bool loadFromFile(const std::string& path);

    /**
     * \brief [AI GENERATED] Retrieve parameters for a MIDI note.
     * @param midi MIDI note number.
     * @return Parameters for the note or defaults if not present.
     */
    NoteParams getParams(int midi) const;

private:
    std::unordered_map<int, NoteParams> params_;
};

} // namespace Utils
} // namespace PianoSynth

