#include <iostream>
#include <cmath>

class StringModel {
public:
    void initialize() {
        std::cout << "String model initialized" << std::endl;
    }
    
    void shutdown() {
        std::cout << "String model shutdown" << std::endl;
    }
    
    float synthesize(float frequency, float amplitude) {
        // Simple sine wave for testing
        static float phase = 0.0f;
        float sample = amplitude * std::sin(2.0f * M_PI * frequency * phase);
        phase += 1.0f / 44100.0f; // Assuming 44.1kHz sample rate
        if (phase >= 1.0f) phase -= 1.0f;
        return sample;
    }
};
