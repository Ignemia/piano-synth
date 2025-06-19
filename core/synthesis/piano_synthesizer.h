#pragma once

#include "../abstraction/note_event.h"
#include "../physics/string_model.h"
#include "../physics/hammer_model.h"
#include "../physics/resonance_model.h"
#include "../utils/config_manager.h"
#include <vector>
#include <memory>
#include <map>

namespace PianoSynth {
namespace Synthesis {

/**
 * Voice represents a single note being played, containing
 * all the physical models for that note
 */
struct Voice {
    int note_number;
    bool active;
    
    std::unique_ptr<Physics::StringModel> string_model;
    std::unique_ptr<Physics::HammerModel> hammer_model;
    
    // Voice state
    float amplitude;
    float frequency;
    double age; // Time since note on
    bool sustain_pedal_active;
    bool note_off_received;
    double note_off_time;
    float release_envelope_rate; // Dynamic release rate based on note-off velocity
    
    // Filter state (to avoid static variable sharing)
    double lowpass_prev_output;
    double dc_prev_input;
    double dc_prev_output;
    
    Voice(int note_num);
    ~Voice();
    
    void initialize(double sample_rate);
    void noteOn(const Abstraction::NoteEvent& event);
    void noteOff(const Abstraction::NoteEvent& event);
    double generateSample();
    void updateEnvelope(double dt);
    bool shouldRelease() const;
};

/**
 * Main piano synthesizer that coordinates all physical models
 * and generates the final audio output
 */
class PianoSynthesizer {
public:
    PianoSynthesizer();
    ~PianoSynthesizer();

    // Initialization
    bool initialize(Utils::ConfigManager* config_manager);
    void shutdown();
    
    // Note processing
    void processNoteEvent(const Abstraction::NoteEvent& event);
    
    // Audio generation
    std::vector<float> generateAudioBuffer(int buffer_size);
    
    // Real-time controls
    void setPedalDamping(float damping);
    void setStringTension(float tension);
    void setMasterTuning(float tuning_offset);
    void setVelocitySensitivity(float sensitivity);
    
    // Synthesis parameters
    void setSoundboardResonance(float resonance);
    void setRoomAcoustics(float size, float damping);
    void setStringCoupling(float coupling_strength);
    
private:
    // Voice management
    std::map<int, Voice*> active_voices_;
    std::vector<std::unique_ptr<Voice>> voice_pool_;
    int max_voices_;
    
    // Global models
    std::unique_ptr<Physics::ResonanceModel> resonance_model_;
    
    // Synthesis parameters
    double sample_rate_;
    float pedal_damping_;
    float string_tension_;
    float master_tuning_;
    float velocity_sensitivity_;
    
    // Processing state
    std::vector<float> audio_buffer_;
    std::vector<double> string_outputs_;
    
    // Configuration
    Utils::ConfigManager* config_manager_;
    
    // Internal methods
    Voice* allocateVoice(int note_number);
    void releaseVoice(int note_number);
    void updateAllVoices();
    void processResonance();
    void mixVoicesToBuffer(int buffer_size);
    void applyMasterEffects();
    
    // Voice management utilities
    Voice* findOldestVoice();
    void cleanupInactiveVoices();
    bool isVoiceActive(const Voice* voice) const;
    
    // Audio processing utilities
    void clearAudioBuffer();
    void normalizeAudio();
    void applySoftClipping();
};

} // namespace Synthesis
} // namespace PianoSynth
