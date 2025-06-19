// Stub implementation for input processor
// This file is not used in the main build but kept for compatibility

#include <iostream>
#include <vector>
#include <string>
#include <functional>

// Stub namespace and types to avoid compilation errors
namespace PianoSynth {
namespace Input {
    
    struct RawMidiEvent {
        std::vector<unsigned char> data;
        double timestamp;
        unsigned int source_port;
    };
    
    class IInputProcessor {
    public:
        virtual ~IInputProcessor() = default;
        virtual bool initialize(const std::string& config_path) = 0;
        virtual void shutdown() = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual std::vector<RawMidiEvent> processInput() = 0;
        virtual void setEventCallback(std::function<void(const RawMidiEvent&)> callback) = 0;
        virtual void configure(const std::string& json_config) = 0;
    };
}

namespace Common {
    using RawMidiEvent = Input::RawMidiEvent;
}
}

class InputProcessorImpl : public PianoSynth::Input::IInputProcessor {
public:
    bool initialize(const std::string& config_path) override {
        std::cout << "Input processor initialized with config: " << config_path << std::endl;
        return true;
    }
    
    void shutdown() override {
        std::cout << "Input processor shutdown" << std::endl;
    }
    
    void start() override {
        std::cout << "Input processor started" << std::endl;
    }
    
    void stop() override {
        std::cout << "Input processor stopped" << std::endl;
    }
    
    std::vector<PianoSynth::Input::RawMidiEvent> processInput() override {
        // Return empty vector for now
        return {};
    }
    
    void setEventCallback(std::function<void(const PianoSynth::Input::RawMidiEvent&)> callback) override {
        // Store callback for future use
        (void)callback; // Suppress unused parameter warning
    }
    
    void configure(const std::string& json_config) override {
        std::cout << "Input processor configured: " << json_config << std::endl;
    }
};

// DLL exports
extern "C" {
    PianoSynth::Input::IInputProcessor* createInputProcessor() {
        return new InputProcessorImpl();
    }
    
    void destroyInputProcessor(PianoSynth::Input::IInputProcessor* processor) {
        delete processor;
    }
}