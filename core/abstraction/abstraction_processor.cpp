#include "../shared/interfaces/dll_interfaces.h"
#include "../shared/utils/json_config.h"
#include "note_event.h"
#include <iostream>

class AbstractionProcessorImpl : public PianoSynth::Abstraction::IAbstractionProcessor {
public:
    bool initialize(const std::string& config_path) override {
        std::cout << "Abstraction processor initialized with config: " << config_path << std::endl;
        return true;
    }
    
    void shutdown() override {
        std::cout << "Abstraction processor shutdown" << std::endl;
    }
    
    void start() override {
        std::cout << "Abstraction processor started" << std::endl;
    }
    
    void stop() override {
        std::cout << "Abstraction processor stopped" << std::endl;
    }
    
    void processMidiEvent(const PianoSynth::Common::RawMidiEvent& midi_event) override {
        std::cout << "Processing MIDI event: type=" << static_cast<int>(midi_event.type) 
                  << " note=" << static_cast<int>(midi_event.data1) 
                  << " velocity=" << static_cast<int>(midi_event.data2) << std::endl;
    }
    
    std::vector<PianoSynth::Common::NoteEvent> getAbstractedEvents() override {
        // Return empty vector for now
        return {};
    }
    
    void setEventCallback(std::function<void(const PianoSynth::Common::NoteEvent&)> callback) override {
        // Store callback for future use
    }
    
    void configure(const std::string& json_config) override {
        std::cout << "Abstraction processor configured" << std::endl;
    }
};

// DLL exports
extern "C" {
    PianoSynth::Abstraction::IAbstractionProcessor* createAbstractionProcessor() {
        return new AbstractionProcessorImpl();
    }
    
    void destroyAbstractionProcessor(PianoSynth::Abstraction::IAbstractionProcessor* processor) {
        delete processor;
    }
}
