#include "cli_arguments.h"
#include <string>

namespace PianoSynth {
namespace App {

CliOptions parseCliArguments(int argc, char* argv[]) {
    CliOptions opts;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--record") {
            opts.record = true;
        } else if (arg == "--config") {
            opts.show_config = true;
        }
    }
    return opts;
}

} // namespace App
} // namespace PianoSynth

