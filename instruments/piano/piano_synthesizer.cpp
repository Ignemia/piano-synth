// Stub implementation for piano synthesizer DLL interface
// This file is not used in the main build but kept for compatibility

#include <iostream>
#include <string>
#include <cmath>

// Stub namespace and types to avoid compilation errors
namespace PianoSynth {
namespace Common {
    enum class NoteEventType {
        NOTE_ON,
        NOTE_OFF,
        PEDAL_CHANGE
    };
    
    struct NoteEvent {
        NoteEventType type;
        int note_number;
        float velocity;
        double timestamp;
        bool sustain_pedal;
        bool soft_pedal;
    };
}

namespace Instruments {
    class IInstrumentSynthesizer {
    public:
        virtual ~IInstrumentSynthesizer() = default;
        virtual bool initialize(const std::string& config_path) = 0;
        virtual void shutdown() = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void processNoteEvent(const Common::NoteEvent& note_event) = 0;
        virtual void synthesizeAudio(float* output_buffer, size_t sample_count, int sample_rate) = 0;
        virtual void setVoiceParameters(int voice_id, const std::string& parameter_name, float value) = 0;
        virtual void configure(const std::string& json_config) = 0;
    };
}
}

class PianoSynthesizerImpl : public PianoSynth::Instruments::IInstrumentSynthesizer {
public:
    bool initialize(const std::string& config_path) override {
        std::cout << "Piano synthesizer initialized with config: " << config_path << std::endl;
        return true;
    }
    
    void shutdown() override {
        std::cout << "Piano synthesizer shutdown" << std::endl;
    }
    
    void start() override {
        std::cout << "Piano synthesizer started" << std::endl;
    }
    
    void stop() override {
        std::cout << "Piano synthesizer stopped" << std::endl;
    }
    
    void processNoteEvent(const PianoSynth::Common::NoteEvent& note_event) override {
        std::cout << "Processing note event: type=" << static_cast<int>(note_event.type) 
                  << " note=" << note_event.note_number 
                  << " velocity=" << note_event.velocity << std::endl;
    }
    
    void synthesizeAudio(float* output_buffer, size_t sample_count, int sample_rate) override {
        // Generate simple sine wave for testing
        static float phase = 0.0f;
        const float frequency = 440.0f; // A4
        const float amplitude = 0.1f;
        
        for (size_t i = 0; i < sample_count; ++i) {
            output_buffer[i] = amplitude * std::sin(2.0f * static_cast<float>(M_PI) * 2.0f * frequency * phase);
            phase += 1.0f / sample_rate;
            if (phase >= 1.0f) phase -= 1.0f;
        }
    }
    
    void setVoiceParameters(int voice_id, const std::string& parameter_name, float value) override {
        std::cout << "Setting voice parameter: voice=" << voice_id 
                  << " param=" << parameter_name 
                  << " value=" << value << std::endl;
    }
    
    void configure(const std::string& json_config) override {
        std::cout << "Piano synthesizer configured: " << json_config << std::endl;
    }
};

// DLL exports
extern "C" {
    PianoSynth::Instruments::IInstrumentSynthesizer* createInstrumentSynthesizer() {
        return new PianoSynthesizerImpl();
    }
    
    void destroyInstrumentSynthesizer(PianoSynth::Instruments::IInstrumentSynthesizer* synthesizer) {
        delete synthesizer;
    }
}