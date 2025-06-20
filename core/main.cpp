#include "piano_synth.h"
#include "../app/cli_arguments.h"
#include "utils/note_settings_cli.h"
#include <string>

int main(int argc, char* argv[]) {
    using namespace PianoSynth;

    App::CliOptions options = App::parseCliArguments(argc, argv);

    if (options.show_config) {
        Utils::showNoteSettings("config/piano_config.json");
        return 0;
    }

    PianoSynthApp app;
    app.run(options.record);
    return 0;
}
