#include "OutputHandler.h"
#include <fstream>
#include <cstdint>

/**
 * @brief [AI GENERATED] Helper to write little-endian integers.
 */
static void writeLE(std::ofstream& stream, uint32_t value, int size) {
    for (int i = 0; i < size; ++i) {
        stream.put(static_cast<char>(value & 0xFF));
        value >>= 8;
    }
}

/**
 * @brief [AI GENERATED] Write samples to a WAV file.
 */
void OutputHandler::writeWav(const std::vector<double>& samples, const std::string& file, int sampleRate) const {
    std::ofstream out(file, std::ios::binary);
    uint32_t dataSize = samples.size() * sizeof(int16_t);

    out.write("RIFF", 4);
    writeLE(out, 36 + dataSize, 4);
    out.write("WAVE", 4);

    out.write("fmt ", 4);
    writeLE(out, 16, 4);
    writeLE(out, 1, 2);
    writeLE(out, 1, 2);
    writeLE(out, sampleRate, 4);
    writeLE(out, sampleRate * 2, 4);
    writeLE(out, 2, 2);
    writeLE(out, 16, 2);

    out.write("data", 4);
    writeLE(out, dataSize, 4);

    for (double s : samples) {
        int16_t v = static_cast<int16_t>(s * 32767);
        writeLE(out, static_cast<uint16_t>(v), 2);
    }
}
