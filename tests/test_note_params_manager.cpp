// [AI GENERATED]
#include <gtest/gtest.h>
#include "../core/utils/note_params_manager.h"
#include "../core/synthesis/piano_synthesizer.h"
#include <filesystem>
#include <fstream>

using namespace PianoSynth::Utils;
using namespace PianoSynth::Synthesis;

class NoteParamsManagerTest : public ::testing::Test {
protected:
    std::string test_file = "test_midi_params.json";

    void SetUp() override {
        std::ofstream file(test_file);
        file << R"({
            \"60\": {"inharm": 0.0002, "partials": 18, "decay": 9, "hammer": 0.1, "volume": 0.8, "tension": 1.0, "detune": 25},
            \"61\": {"inharm": 0.00021, "partials": 17, "decay": 8.5, "hammer": 0.11, "volume": 0.81, "tension": 1.01, "detune": -10}
        })";
        file.close();
    }

    void TearDown() override {
        std::filesystem::remove(test_file);
    }
};

TEST_F(NoteParamsManagerTest, LoadAndGetParams) {
    NoteParamsManager manager;
    ASSERT_TRUE(manager.loadFromFile(test_file));

    NoteParams p60 = manager.getParams(60);
    EXPECT_DOUBLE_EQ(p60.inharmonicity, 0.0002);
    EXPECT_EQ(p60.partials, 18);
    EXPECT_DOUBLE_EQ(p60.decay, 9);
    EXPECT_DOUBLE_EQ(p60.hammer, 0.1);
    EXPECT_DOUBLE_EQ(p60.volume, 0.8);
    EXPECT_DOUBLE_EQ(p60.tension, 1.0);
    EXPECT_DOUBLE_EQ(p60.detune_cents, 25);

    NoteParams p61 = manager.getParams(61);
    EXPECT_DOUBLE_EQ(p61.inharmonicity, 0.00021);
    EXPECT_EQ(p61.partials, 17);
    EXPECT_DOUBLE_EQ(p61.decay, 8.5);
    EXPECT_DOUBLE_EQ(p61.hammer, 0.11);
    EXPECT_DOUBLE_EQ(p61.volume, 0.81);
    EXPECT_DOUBLE_EQ(p61.tension, 1.01);
    EXPECT_DOUBLE_EQ(p61.detune_cents, -10);

    NoteParams p62 = manager.getParams(62);
    EXPECT_EQ(p62.partials, 0);
}

TEST_F(NoteParamsManagerTest, ApplyParamsToVoice) {
    NoteParamsManager manager;
    ASSERT_TRUE(manager.loadFromFile(test_file));

    Synthesis::Voice voice(60);
    voice.initialize(44100.0);
    double base_tension = voice.string_model->getTension();
    voice.applyNoteParams(manager.getParams(60));

    EXPECT_EQ(voice.string_model->getNumHarmonics(), 18u);
    EXPECT_DOUBLE_EQ(voice.string_model->getInharmonicityCoefficient(), 0.0002);
    EXPECT_NEAR(voice.string_model->getDamping(), 1.0 / 9.0, 1e-6);
    EXPECT_DOUBLE_EQ(voice.amplitude, 0.8f);
    EXPECT_NEAR(voice.string_model->getTension(), base_tension * 1.0, 1e-6);
    EXPECT_DOUBLE_EQ(voice.string_model->getDetuneCents(), 25);
}

