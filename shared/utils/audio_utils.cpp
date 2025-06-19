#include "audio_utils.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace PianoSynth {
namespace Utils {

void AudioUtils::clear_buffer(Interfaces::AudioBuffer& buffer) {
    if (buffer.samples && buffer.total_samples() > 0) {
        std::memset(buffer.samples, 0, buffer.size_bytes());
    }
}

void AudioUtils::copy_buffer(const Interfaces::AudioBuffer& src, Interfaces::AudioBuffer& dst) {
    if (!src.samples || !dst.samples) return;
    
    size_t copy_samples = std::min(src.total_samples(), dst.total_samples());
    std::memcpy(dst.samples, src.samples, copy_samples * sizeof(float));
    
    // Clear remaining samples if dst is larger
    if (dst.total_samples() > copy_samples) {
        std::memset(dst.samples + copy_samples, 0, 
                   (dst.total_samples() - copy_samples) * sizeof(float));
    }
}

void AudioUtils::mix_buffers(const Interfaces::AudioBuffer& src, Interfaces::AudioBuffer& dst, float gain) {
    if (!src.samples || !dst.samples) return;
    
    size_t mix_samples = std::min(src.total_samples(), dst.total_samples());
    
    for (size_t i = 0; i < mix_samples; i++) {
        dst.samples[i] += src.samples[i] * gain;
    }
}

float AudioUtils::calculate_rms(const Interfaces::AudioBuffer& buffer) {
    if (!buffer.samples || buffer.total_samples() == 0) return 0.0f;
    
    double sum = 0.0;
    for (size_t i = 0; i < buffer.total_samples(); i++) {
        double sample = buffer.samples[i];
        sum += sample * sample;
    }
    
    return static_cast<float>(std::sqrt(sum / buffer.total_samples()));
}

float AudioUtils::find_peak(const Interfaces::AudioBuffer& buffer) {
    if (!buffer.samples || buffer.total_samples() == 0) return 0.0f;
    
    float peak = 0.0f;
    for (size_t i = 0; i < buffer.total_samples(); i++) {
        peak = std::max(peak, std::abs(buffer.samples[i]));
    }
    
    return peak;
}

bool AudioUtils::detect_clipping(const Interfaces::AudioBuffer& buffer, float threshold) {
    return find_peak(buffer) >= threshold;
}

void AudioUtils::generate_silence(Interfaces::AudioBuffer& buffer) {
    clear_buffer(buffer);
}

void AudioUtils::generate_sine_wave(Interfaces::AudioBuffer& buffer, double frequency, double phase) {
    if (!buffer.samples || buffer.frame_count == 0) return;
    
    double phase_increment = 2.0 * M_PI * frequency / buffer.sample_rate;
    
    for (size_t frame = 0; frame < buffer.frame_count; frame++) {
        float sample = static_cast<float>(std::sin(phase + frame * phase_increment));
        
        for (size_t channel = 0; channel < buffer.channel_count; channel++) {
            buffer.samples[frame * buffer.channel_count + channel] = sample;
        }
    }
}

void AudioUtils::apply_fade_in(Interfaces::AudioBuffer& buffer, size_t fade_frames) {
    if (!buffer.samples || fade_frames == 0) return;
    
    size_t actual_fade_frames = std::min(fade_frames, buffer.frame_count);
    
    for (size_t frame = 0; frame < actual_fade_frames; frame++) {
        float gain = static_cast<float>(frame) / actual_fade_frames;
        
        for (size_t channel = 0; channel < buffer.channel_count; channel++) {
            buffer.samples[frame * buffer.channel_count + channel] *= gain;
        }
    }
}

void AudioUtils::apply_fade_out(Interfaces::AudioBuffer& buffer, size_t fade_frames) {
    if (!buffer.samples || fade_frames == 0) return;
    
    size_t actual_fade_frames = std::min(fade_frames, buffer.frame_count);
    size_t fade_start = buffer.frame_count - actual_fade_frames;
    
    for (size_t frame = fade_start; frame < buffer.frame_count; frame++) {
        float gain = static_cast<float>(buffer.frame_count - frame) / actual_fade_frames;
        
        for (size_t channel = 0; channel < buffer.channel_count; channel++) {
            buffer.samples[frame * buffer.channel_count + channel] *= gain;
        }
    }
}

void AudioUtils::interleave_channels(const std::vector<float*>& channels, size_t frames, float* output) {
    if (channels.empty() || !output) return;
    
    for (size_t frame = 0; frame < frames; frame++) {
        for (size_t channel = 0; channel < channels.size(); channel++) {
            if (channels[channel]) {
                output[frame * channels.size() + channel] = channels[channel][frame];
            } else {
                output[frame * channels.size() + channel] = 0.0f;
            }
        }
    }
}

void AudioUtils::deinterleave_channels(const float* input, size_t frames, std::vector<float*>& channels) {
    if (!input || channels.empty()) return;
    
    for (size_t frame = 0; frame < frames; frame++) {
        for (size_t channel = 0; channel < channels.size(); channel++) {
            if (channels[channel]) {
                channels[channel][frame] = input[frame * channels.size() + channel];
            }
        }
    }
}

void AudioUtils::resample_linear(const float* input, size_t input_frames, 
                                float* output, size_t output_frames) {
    if (!input || !output || input_frames == 0 || output_frames == 0) return;
    
    double ratio = static_cast<double>(input_frames) / output_frames;
    
    for (size_t i = 0; i < output_frames; i++) {
        double index = i * ratio;
        size_t index_int = static_cast<size_t>(index);
        double fraction = index - index_int;
        
        if (index_int + 1 < input_frames) {
            // Linear interpolation
            output[i] = static_cast<float>(
                input[index_int] * (1.0 - fraction) + 
                input[index_int + 1] * fraction
            );
        } else if (index_int < input_frames) {
            // Last sample
            output[i] = input[index_int];
        } else {
            // Beyond input, use zero
            output[i] = 0.0f;
        }
    }
}

} // namespace Utils
} // namespace PianoSynth
