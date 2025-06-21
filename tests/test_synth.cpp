#include "MidiInput.h"
#include "Abstractor.h"
#include "NoteSynth.h"
#include "OutputHandler.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>

/**
 * @brief [AI GENERATED] Basic integration tests for synthesizer modules.
 */
int main() {
    MidiInput midi;
    auto midiData = midi.generateDemo();
    assert(!midiData.empty());

    Abstractor abs;
    auto notes = abs.convert(midiData);
    assert(notes.size() == midiData.size());
    assert(notes[0].frequency > 0);

    NoteSynth synth;
    auto samples = synth.synthesize(notes, 8000);
    assert(!samples.empty());

    int firstCount = static_cast<int>(notes[0].duration * 8000);
    assert(firstCount >= static_cast<int>(0.8 * 8000));
    int quarterIdx = static_cast<int>(8000 / (4.0 * notes[0].frequency));
    if (quarterIdx >= firstCount) {
        quarterIdx = 0;
    }
    int endIdx = firstCount - quarterIdx - 1;
    assert(samples[0] != 0.0);
    int sustainIdx = static_cast<int>(firstCount * 0.7);
    assert(std::abs(samples[endIdx]) < std::abs(samples[sustainIdx]));

    OutputHandler out;
    const std::string file = "test.wav";
    out.writeWav(samples, file, 8000);
    assert(std::filesystem::exists(file));
    std::uintmax_t size = std::filesystem::file_size(file);
    assert(size > 44);
    std::filesystem::remove(file);

    std::cout << "All tests passed\n";
}
