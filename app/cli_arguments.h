#pragma once
// [AI GENERATED]

#include <string>

namespace PianoSynth {
namespace App {

/**
 * \brief [AI GENERATED] Command line options for the piano_synth application.
 */
struct CliOptions {
    bool record = false;     ///< start recording immediately
    bool show_config = false;///< show note settings manager
};

/**
 * \brief [AI GENERATED] Parse application command line arguments.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Parsed options structure.
 */
CliOptions parseCliArguments(int argc, char* argv[]);

} // namespace App
} // namespace PianoSynth

