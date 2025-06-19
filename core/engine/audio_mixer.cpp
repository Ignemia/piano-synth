#include <iostream>
#include <vector>

class AudioMixer {
public:
    void initialize() {
        std::cout << "Audio mixer initialized" << std::endl;
    }
    
    void shutdown() {
        std::cout << "Audio mixer shutdown" << std::endl;
    }
    
    void mixAudio(const std::vector<float*>& input_buffers, float* output_buffer, size_t samples) {
        // Simple mixer - just sum all inputs
        for (size_t i = 0; i < samples; ++i) {
            output_buffer[i] = 0.0f;
            for (const auto& buffer : input_buffers) {
                output_buffer[i] += buffer[i];
            }
            output_buffer[i] /= static_cast<float>(input_buffers.size()); // Normalize
        }
    }
};
