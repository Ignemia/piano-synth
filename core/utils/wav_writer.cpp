#include "wav_writer.h"
#include <fstream>
#include <algorithm>
#include <cstdint>

namespace PianoSynth {
namespace Utils {

bool WavWriter::write(const std::vector<float>& audio_data,
                      const std::string& filename,
                      int sample_rate,
                      int channels,
                      int bits_per_sample) {
    if (bits_per_sample != 16 && bits_per_sample != 32 && bits_per_sample != 64) {
        bits_per_sample = 16;
    }
    int bytes_per_sample = bits_per_sample / 8;
    int frame_count = audio_data.size() / channels;
    int data_size = frame_count * channels * bytes_per_sample;
    int file_size = 36 + data_size;

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // RIFF header
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&file_size), 4);
    file.write("WAVE", 4);

    // Format chunk
    file.write("fmt ", 4);
    int fmt_chunk_size = 16;
    uint16_t audio_format = (bits_per_sample == 16) ? 1 : 3; // PCM =1, IEEE float=3
    uint16_t num_channels = static_cast<uint16_t>(channels);
    uint32_t byte_rate = sample_rate * channels * bytes_per_sample;
    uint16_t block_align = channels * bytes_per_sample;
    uint16_t bits = static_cast<uint16_t>(bits_per_sample);

    file.write(reinterpret_cast<const char*>(&fmt_chunk_size), 4);
    file.write(reinterpret_cast<const char*>(&audio_format), 2);
    file.write(reinterpret_cast<const char*>(&num_channels), 2);
    file.write(reinterpret_cast<const char*>(&sample_rate), 4);
    file.write(reinterpret_cast<const char*>(&byte_rate), 4);
    file.write(reinterpret_cast<const char*>(&block_align), 2);
    file.write(reinterpret_cast<const char*>(&bits), 2);

    // Data chunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&data_size), 4);

    if (bits_per_sample == 16) {
        for (float sample : audio_data) {
            float clamped = std::clamp(sample, -1.0f, 1.0f);
            int16_t pcm = static_cast<int16_t>(clamped * 32767.0f);
            file.write(reinterpret_cast<const char*>(&pcm), 2);
        }
    } else if (bits_per_sample == 32) { // 32-bit float
        for (float sample : audio_data) {
            float clamped = std::clamp(sample, -1.0f, 1.0f);
            file.write(reinterpret_cast<const char*>(&clamped), 4);
        }
    } else { // 64-bit float
        for (float sample : audio_data) {
            double clamped = std::clamp(static_cast<double>(sample), -1.0, 1.0);
            file.write(reinterpret_cast<const char*>(&clamped), 8);
        }
    }

    file.close();
    return true;
}

} // namespace Utils
} // namespace PianoSynth

