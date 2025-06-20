#pragma once

#include <string>
#include <vector>

namespace PianoSynth {
namespace Utils {

/**
 * @brief [AI GENERATED] Utility class for writing WAV files.
 *
 * The WavWriter provides a simple static method to write floating
 * point audio samples to a standard WAV file using either 16-bit
 * PCM, 32-bit or 64-bit IEEE float encoding.
 */
class WavWriter {
public:
    /**
     * @brief Write audio samples to a WAV file.
     *
     * @param audio_data       Interleaved audio samples in the range [-1, 1].
     * @param filename         Destination WAV filename.
     * @param sample_rate      Audio sample rate in Hz.
     * @param channels         Number of audio channels (1 or 2).
     * @param bits_per_sample  Bit depth of the output file (16, 32 or 64).
     * @return true if the file was written successfully.
     */
    static bool write(const std::vector<float>& audio_data,
                      const std::string& filename,
                      int sample_rate,
                      int channels,
                      int bits_per_sample = 16);
};

} // namespace Utils
} // namespace PianoSynth

