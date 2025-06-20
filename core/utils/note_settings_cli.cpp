#include "note_settings_cli.h"
#include "note_params_manager.h"
#include <iostream>

namespace PianoSynth {
namespace Utils {

void showNoteSettings(const std::string& path) {
    NoteParamsManager manager;
    if (!manager.loadFromFile(path)) {
        std::cerr << "Failed to load note settings from " << path << std::endl;
        return;
    }
    std::cout << "Note settings from " << path << ":" << std::endl;
    for (int note = 0; note < 128; ++note) {
        auto p = manager.getParams(note);
        if (p.partials > 0 || p.inharmonicity != 0.0 || p.decay != 0.0 ||
            p.hammer != 0.0 || p.volume != 0.0 || p.tension != 1.0) {
            std::cout << note
                      << ": inharm=" << p.inharmonicity
                      << " partials=" << p.partials
                      << " decay=" << p.decay
                      << " hammer=" << p.hammer
                      << " volume=" << p.volume
                      << " tension=" << p.tension << std::endl;
        }
    }
}

} // namespace Utils
} // namespace PianoSynth

