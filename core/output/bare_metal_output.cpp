#include <iostream>

class BareMetalOutput {
public:
    void initialize() {
        std::cout << "Bare metal output initialized" << std::endl;
    }
    
    void shutdown() {
        std::cout << "Bare metal output shutdown" << std::endl;
    }
    
    void playAudio(const float* buffer, size_t samples) {
        std::cout << "Sending " << samples << " audio samples to bare metal output" << std::endl;
    }
};
