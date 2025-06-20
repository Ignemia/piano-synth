// [AI GENERATED]
#include "note_params_manager.h"
#include <fstream>

namespace PianoSynth {
namespace Utils {

bool NoteParamsManager::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    try {
        nlohmann::json j;
        file >> j;
        file.close();
        params_.clear();
        for (auto it = j.begin(); it != j.end(); ++it) {
            int midi = std::stoi(it.key());
            const auto& obj = it.value();
            NoteParams p;
            if (obj.contains("inharm")) p.inharmonicity = obj["inharm"].get<double>();
            if (obj.contains("partials")) p.partials = obj["partials"].get<int>();
            if (obj.contains("decay")) p.decay = obj["decay"].get<double>();
            if (obj.contains("hammer")) p.hammer = obj["hammer"].get<double>();
            if (obj.contains("volume")) p.volume = obj["volume"].get<double>();
            if (obj.contains("tension")) p.tension = obj["tension"].get<double>();
            if (obj.contains("detune")) p.detune_cents = obj["detune"].get<double>();
            params_[midi] = p;
        }
        return true;
    } catch (...) {
        return false;
    }
}

NoteParams NoteParamsManager::getParams(int midi) const {
    auto it = params_.find(midi);
    if (it != params_.end()) {
        return it->second;
    }
    return NoteParams();
}

} // namespace Utils
} // namespace PianoSynth

