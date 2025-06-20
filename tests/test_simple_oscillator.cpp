#include <gtest/gtest.h>
#include "../instruments/piano/simple_oscillator.h"
#include "../shared/interfaces/dll_interfaces.h"
#include <vector>
#include <cmath>

using namespace PianoSynth::Interfaces;

/**
 * @brief [AI GENERATED] Verify oscillator instrument generates audio at 192kHz.
 */
TEST(SimpleOscillator, GeneratesAudio) {
    IInstrumentSynthesizer* synth = create_instrument_synthesizer();
    ASSERT_NE(synth, nullptr);

    double rate = 192000.0;
    size_t buffer_size = 256;
    ASSERT_TRUE(synth->initialize("{}", rate, buffer_size * 4));

    MusicalEvent on;
    on.type = EventType::NOTE_ON;
    on.timestamp = std::chrono::high_resolution_clock::now();
    on.note_number = 60;
    on.velocity = 0.8f;
    EXPECT_TRUE(synth->process_events(&on, 1));

    std::vector<float> data(buffer_size * 2);
    AudioBuffer buf;
    buf.samples = data.data();
    buf.frame_count = buffer_size;
    buf.channel_count = 2;
    buf.sample_rate = rate;
    buf.timestamp = std::chrono::high_resolution_clock::now();

    EXPECT_GT(synth->generate_audio(&buf), 0);

    bool nonzero = false;
    for (float s : data) {
        if (std::fabs(s) > 1e-5f) { nonzero = true; break; }
    }
    EXPECT_TRUE(nonzero);

    destroy_instrument_synthesizer(synth);
}
