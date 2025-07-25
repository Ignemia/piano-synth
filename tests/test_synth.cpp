#include "../include/MidiInput.h"
#include "../include/Abstractor.h"
#include "../include/NoteSynth.h"
#include "../include/OutputHandler.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <cstdint>

/**
 * @brief [AI GENERATED] Basic integration tests for synthesizer modules.
 */
int main() {
    MidiInput midi;
    auto midiData = midi.generateDemo();
    assert(!midiData.empty());
    assert(midiData.size() > 3);
    assert(midiData[0].startTime == 0.0);
    assert(midiData[1].startTime == 0.0);
    assert(midiData[2].startTime == 0.0);
    assert(midiData[3].startTime == midiData[4].startTime);

    Abstractor abs;
    auto notes = abs.convert(midiData);
    assert(notes.size() == midiData.size());
    assert(notes[0].frequency > 0);
    assert(notes[0].startTime == 0.0);

    NoteSynth synth;
    std::srand(0);
    auto samples = synth.synthesize(notes, 8000);
    assert(!samples.empty());
    constexpr double kReleaseTime = 0.3;
    double totalDuration = 0.0;
    for (const auto& n : notes) {
        totalDuration = std::max(totalDuration, n.startTime + n.duration + kReleaseTime);

    }
    int expectedSamples = static_cast<int>(totalDuration * 8000);
    assert(samples.size() == static_cast<size_t>(expectedSamples));
    int holdSamples = static_cast<int>(notes[0].duration * 8000);
    int releaseSamples = static_cast<int>(kReleaseTime * 8000);
    int firstCount = holdSamples + releaseSamples;
    int holdEndIdx = holdSamples - 1;
    int endIdx = firstCount - 1;
    int attackIdx = static_cast<int>(0.005 * 8000);
    assert(samples[attackIdx] > samples[0]);
    assert(std::abs(samples[endIdx]) < std::abs(samples[holdEndIdx]));

    double maxVal = 0.0;
    for (double s : samples) {
        maxVal = std::max(maxVal, std::abs(s));
    }
    assert(maxVal <= 1.0);
    for (double s : samples) {
        maxVal = std::max(maxVal, std::abs(s));
    }
    assert(maxVal <= 1.0);

    OutputHandler out;
    std::string file = "test.wav";
    out.writeWav(samples, file, 8000);
    assert(std::filesystem::exists(file));
    std::uintmax_t size = std::filesystem::file_size(file);
    assert(size > 44);
    std::filesystem::remove(file);

    std::cout << "All tests passed\n";
    return 0;
}
