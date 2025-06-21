/**
 * @file OutputHandler.h
 * @brief [AI GENERATED] Writes audio data to a WAV file.
 */

#pragma once
#include <vector>
#include <string>

/**
 * @brief [AI GENERATED] Manages audio output.
 */
class OutputHandler {
public:
    void writeWav(const std::vector<double>& samples, const std::string& file, int sampleRate = 44100) const;
};
