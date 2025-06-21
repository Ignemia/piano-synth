#include "MidiInput.h"
#include "Abstractor.h"
#include "NoteSynth.h"
#include "OutputHandler.h"
#include <iostream>
#include <string>

/**
 * @file main.cpp
 * @brief [AI GENERATED] Entry point selecting demo or realtime mode.
 */
int main(int argc, char* argv[]) {
    bool demo = (argc > 1 && std::string(argv[1]) == "--demo");

    MidiInput midi;
    Abstractor abs;
    NoteSynth synth;
    OutputHandler out;

    if (demo) {
        auto midiData = midi.generateDemo();
        auto notes = abs.convert(midiData);
        auto samples = synth.synthesize(notes);
        out.writeWav(samples, "demo_output.wav");
        std::cout << "Demo written to demo_output.wav\n";
    } else {
        std::cout << "Realtime mode not implemented\n";
    }

    return 0;
}
