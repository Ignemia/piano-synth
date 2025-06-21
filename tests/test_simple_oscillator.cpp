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

/**
 * @brief [AI GENERATED] Estimate frequency from waveform by counting samples
 *        between rising zero crossings.
 */
static double estimate_freq(const std::vector<float>& data,
                            double sample_rate) {
    int first = -1;
    int second = -1;
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i - 1] <= 0.0f && data[i] > 0.0f) {
            if (first < 0) {
                first = static_cast<int>(i);
            } else {
                second = static_cast<int>(i);
                break;
            }
        }
    }
    if (first >= 0 && second > first) {
        return sample_rate / static_cast<double>(second - first);
    }
    return 0.0;
}

/**
 * @brief [AI GENERATED] Verify oscillator applies slight frequency variation
 *        over time.
 */
TEST(SimpleOscillator, FrequencyVariation) {
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

    std::vector<float> early(buffer_size * 2 * 8);
    AudioBuffer buf;
    buf.samples = early.data();
    buf.frame_count = buffer_size * 8;
    buf.channel_count = 2;
    buf.sample_rate = rate;
    buf.timestamp = std::chrono::high_resolution_clock::now();
    EXPECT_GT(synth->generate_audio(&buf), 0);

    double f1 = estimate_freq(early, rate);
    EXPECT_GT(f1, 0.0);

    std::vector<float> later(buffer_size * 2 * 8);
    buf.samples = later.data();
    EXPECT_GT(synth->generate_audio(&buf), 0);

    double f2 = estimate_freq(later, rate);
    EXPECT_GT(f2, 0.0);

    EXPECT_NEAR(f1, 261.63, 1.0); // nominal
    EXPECT_NE(f1, f2);

    destroy_instrument_synthesizer(synth);
}

/**
 * @brief [AI GENERATED] Verify output amplitude decays over time due to
 *        string vibration emulation.
 */
TEST(SimpleOscillator, AmplitudeDecay) {
    IInstrumentSynthesizer* synth = create_instrument_synthesizer();
    ASSERT_NE(synth, nullptr);

    double rate = 192000.0;
    size_t buffer_size = 256;
    ASSERT_TRUE(synth->initialize("{}", rate, buffer_size * 4));

    MusicalEvent on;
    on.type = EventType::NOTE_ON;
    on.timestamp = std::chrono::high_resolution_clock::now();
    on.note_number = 60;
    on.velocity = 1.0f;
    EXPECT_TRUE(synth->process_events(&on, 1));

    std::vector<float> data(buffer_size * 2 * 16);
    AudioBuffer buf;
    buf.samples = data.data();
    buf.frame_count = buffer_size * 16;
    buf.channel_count = 2;
    buf.sample_rate = rate;
    buf.timestamp = std::chrono::high_resolution_clock::now();

    EXPECT_GT(synth->generate_audio(&buf), 0);

    float early = 0.0f;
    for (size_t i = 0; i < buffer_size * 2; ++i) {
        early += std::abs(data[i]);
    }

    float late = 0.0f;
    for (size_t i = data.size() - buffer_size * 2; i < data.size(); ++i) {
        late += std::abs(data[i]);
    }

    EXPECT_GT(early, late);

    destroy_instrument_synthesizer(synth);
}
