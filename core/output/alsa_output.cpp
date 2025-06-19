#include <iostream>
#include <string>

class AlsaOutput {
public:
    void initialize() {
        std::cout << "ALSA output initialized" << std::endl;
    }
    
    void shutdown() {
        std::cout << "ALSA output shutdown" << std::endl;
    }
    
    void playAudio(const float* buffer, size_t samples) {
        std::cout << "Playing " << samples << " audio samples via ALSA" << std::endl;
    }
};
