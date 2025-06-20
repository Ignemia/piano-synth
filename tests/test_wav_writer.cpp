#include <gtest/gtest.h>
#include "../core/utils/wav_writer.h"
#include <fstream>
#include <vector>

using namespace PianoSynth::Utils;

TEST(WavWriterTest, WritesCorrectBitDepth) {
    std::vector<float> silence(100, 0.0f);
    ASSERT_TRUE(WavWriter::write(silence, "test_output/out16.wav", 44100, 2, 16));
    ASSERT_TRUE(WavWriter::write(silence, "test_output/out32.wav", 44100, 2, 32));
    ASSERT_TRUE(WavWriter::write(silence, "test_output/out64.wav", 44100, 2, 64));

    auto readBits = [](const std::string& path) -> int {
        std::ifstream f(path, std::ios::binary);
        f.seekg(34, std::ios::beg);
        uint16_t bits = 0;
        f.read(reinterpret_cast<char*>(&bits), 2);
        return bits;
    };

    EXPECT_EQ(readBits("test_output/out16.wav"), 16);
    EXPECT_EQ(readBits("test_output/out32.wav"), 32);
    EXPECT_EQ(readBits("test_output/out64.wav"), 64);
}

