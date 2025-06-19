#include "../shared/interfaces/dll_interfaces.h"
#include "../shared/utils/json_config.h"
#include <iostream>

class OutputProcessorImpl : public PianoSynth::Output::IOutputProcessor {
public:
    bool initialize(const std::string& config_path) override {
        std::cout << "Output processor initialized with config: " << config_path << std::endl;
        return true;
    }
    
    void shutdown() override {
        std::cout << "Output processor shutdown" << std::endl;
    }
    
    void start() override {
        std::cout << "Output processor started" << std::endl;
    }
    
    void stop() override {
        std::cout << "Output processor stopped" << std::endl;
    }
    
    void processAudio(const float* input_buffer, size_t sample_count, int sample_rate) override {
        // For now, just count the samples
        static size_t total_samples = 0;
        total_samples += sample_count;
        if (total_samples % (sample_rate * 2) == 0) { // Log every 2 seconds
            std::cout << "Processed " << total_samples << " audio samples" << std::endl;
        }
    }
    
    void setOutputDevice(const std::string& device_name) override {
        std::cout << "Setting output device: " << device_name << std::endl;
    }
    
    void setVolume(float volume) override {
        std::cout << "Setting output volume: " << volume << std::endl;
    }
    
    void configure(const std::string& json_config) override {
        std::cout << "Output processor configured" << std::endl;
    }
};

// DLL exports
extern "C" {
    PianoSynth::Output::IOutputProcessor* createOutputProcessor() {
        return new OutputProcessorImpl();
    }
    
    void destroyOutputProcessor(PianoSynth::Output::IOutputProcessor* processor) {
        delete processor;
    }
}
