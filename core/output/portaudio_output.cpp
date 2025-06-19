#include <iostream>

class PortAudioOutput {
public:
    void initialize() {
        std::cout << "PortAudio output initialized" << std::endl;
    }
    
    void shutdown() {
        std::cout << "PortAudio output shutdown" << std::endl;
    }
    
    void playAudio(const float* buffer, size_t samples) {
        std::cout << "Playing " << samples << " audio samples via PortAudio" << std::endl;
    }
};
