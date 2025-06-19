#pragma once

#include "../interfaces/common_types.h"
#include <vector>

namespace PianoSynth {
namespace Utils {

class AudioUtils {
public:
    // Audio buffer utilities
    static void clear_buffer(Interfaces::AudioBuffer& buffer);
    static void copy_buffer(const Interfaces::AudioBuffer& src, Interfaces::AudioBuffer& dst);
    static void mix_buffers(const Interfaces::AudioBuffer& src, Interfaces::AudioBuffer& dst, float gain = 1.0f);
    
    // Audio analysis
    static float calculate_rms(const Interfaces::AudioBuffer& buffer);
    static float find_peak(const Interfaces::AudioBuffer& buffer);
    static bool detect_clipping(const Interfaces::AudioBuffer& buffer, float threshold = 0.95f);
    
    // Audio generation utilities
    static void generate_silence(Interfaces::AudioBuffer& buffer);
    static void generate_sine_wave(Interfaces::AudioBuffer& buffer, double frequency, double phase = 0.0);
    static void apply_fade_in(Interfaces::AudioBuffer& buffer, size_t fade_frames);
    static void apply_fade_out(Interfaces::AudioBuffer& buffer, size_t fade_frames);
    
    // Format conversion
    static void interleave_channels(const std::vector<float*>& channels, size_t frames, float* output);
    static void deinterleave_channels(const float* input, size_t frames, std::vector<float*>& channels);
    
    // Sample rate conversion (basic)
    static void resample_linear(const float* input, size_t input_frames, 
                               float* output, size_t output_frames);
};

} // namespace Utils
} // namespace PianoSynth
