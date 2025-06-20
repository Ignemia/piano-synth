#include "piano_synthesizer.h"
#include "../utils/logger.h"
#include "../utils/constants.h"
#include "../utils/math_utils.h"
#include <algorithm>
#include <cmath>

namespace PianoSynth {
namespace Synthesis {

// Voice Implementation
Voice::Voice(int note_num) 
    : note_number(note_num),
      active(false),
      amplitude(0.0f),
      frequency(0.0f),
      age(0.0),
      sustain_pedal_active(false),
      note_off_received(false),
      note_off_time(0.0),
      release_envelope_rate(0.005f),
      lowpass_prev_output(0.0),
      dc_prev_input(0.0),
      dc_prev_output(0.0) {
    
    string_model = std::make_unique<Physics::StringModel>(note_number);
    hammer_model = std::make_unique<Physics::HammerModel>(note_number);
}

Voice::~Voice() = default;

void Voice::initialize(double sample_rate) {
    string_model->initialize(sample_rate);
    hammer_model->initialize(sample_rate);
    frequency = Utils::MathUtils::midiToFrequency(note_number);
}

/**
 * [AI GENERATED] Update the voice to play a different note.
 */
void Voice::updateNoteNumber(int new_note, double sample_rate) {
    note_number = new_note;
    frequency = Utils::MathUtils::midiToFrequency(new_note);

    // Recreate models to reflect the new fundamental frequency
    string_model = std::make_unique<Physics::StringModel>(new_note);
    hammer_model = std::make_unique<Physics::HammerModel>(new_note);
    string_model->initialize(sample_rate);
    hammer_model->initialize(sample_rate);

    // Reset filters and envelope state
    lowpass_prev_output = 0.0;
    dc_prev_input = 0.0;
    dc_prev_output = 0.0;
    amplitude = 0.0f;
    age = 0.0;
    active = false;
}

void Voice::applyNoteParams(const Utils::NoteParams& params) {
    string_model->setNumHarmonics(params.partials);
    string_model->setInharmonicityCoefficient(params.inharmonicity);
    string_model->setDamping(1.0 / std::max(0.001, params.decay));
    double base_tension = string_model->getTension();
    string_model->setTension(base_tension * params.tension);
    amplitude = params.volume;
}

void Voice::noteOn(const Abstraction::NoteEvent& event) {
    active = true;
    note_off_received = false;
    amplitude = Utils::MathUtils::clamp(event.velocity, 0.1f, 1.0f); // Ensure reasonable velocity range
    age = 0.0;
    sustain_pedal_active = event.sustain_pedal;
    
    // Reset models
    string_model->reset();
    hammer_model->reset();
    
    // Configure string based on note event
    string_model->setDamperPosition(event.damper_position);
    
    // More realistic strike parameters based on velocity
    double strike_position = 0.125 + (event.velocity - 0.5) * 0.05; // Slight position variation
    strike_position = std::clamp(strike_position, 0.1, 0.2);
    
    double strike_force = event.string_excitation * (0.5 + event.velocity * 0.5);
    double strike_duration = 0.0008 + event.velocity * 0.0004; // Velocity affects contact time
    
    // Strike the string with hammer
    hammer_model->strike(event.hammer_velocity);
    string_model->excite(strike_position, strike_force, strike_duration);
    
    Utils::Logger logger;
    logger.debug("Voice " + std::to_string(note_number) + " note on: velocity=" + 
                std::to_string(event.velocity) + " hammer_vel=" + std::to_string(event.hammer_velocity));
}

void Voice::noteOff(const Abstraction::NoteEvent& event) {
    note_off_received = true;
    note_off_time = age;
    
    // Extract release velocity for dynamic release behavior
    float release_velocity = event.velocity;
    release_envelope_rate = 0.002f + (1.0f - release_velocity) * 0.008f; // Faster release for softer note-offs
    
    // Update sustain pedal state from event
    sustain_pedal_active = event.sustain_pedal;
    
    // If sustain pedal is not active, start release immediately
    if (!sustain_pedal_active) {
        string_model->setDamperPosition(0.0); // Apply damper
    }
    
    Utils::Logger logger;
    logger.debug("Voice " + std::to_string(note_number) + " note off");
}

double Voice::generateSample() {
    if (!active) {
        return 0.0;
    }
    
    // Get hammer force
    double string_displacement = string_model->step();
    double hammer_force = hammer_model->step(string_displacement);
    
    // Update envelope
    updateEnvelope(1.0 / Constants::SAMPLE_RATE);
    
    // Apply hammer force to modulate the string displacement
    double modulated_displacement = string_displacement * (1.0 + hammer_force * 0.5);
    
    // Apply amplitude envelope with proper scaling
    double output = modulated_displacement * amplitude * 2.0; // Increased output level
    
    // Apply gentle low-pass filtering to reduce harsh frequencies
    double alpha = 0.85; // Simple one-pole filter
    output = alpha * output + (1.0 - alpha) * lowpass_prev_output;
    lowpass_prev_output = output;
    
    // Add DC blocking filter to remove offset
    double dc_blocked = output - dc_prev_input + 0.995 * dc_prev_output;
    dc_prev_input = output;
    dc_prev_output = dc_blocked;
    output = dc_blocked;
    
    // Apply soft clipping with gentler curve
    output = Utils::DSPUtils::softClip(output, 0.6);
    
    // Final scaling with higher output level
    return output * 3.0;
}

void Voice::updateEnvelope(double dt) {
    age += dt;
    
    // More realistic piano decay curve - fast initial decay, then slower
    double fast_decay = 0.3; // Initial fast decay
    double slow_decay = 0.02; // Sustained slow decay
    double transition_time = 0.5; // Transition after 0.5 seconds
    
    double decay_rate;
    if (age < transition_time) {
        // Fast decay phase with exponential curve
        double t_norm = age / transition_time;
        decay_rate = fast_decay * exp(-t_norm * 3.0) + slow_decay;
    } else {
        // Slow sustain decay
        decay_rate = slow_decay;
    }
    
    amplitude *= (1.0 - decay_rate * dt);
    
    // Apply note-off envelope if needed
    if (note_off_received && !sustain_pedal_active) {
        double release_time = Constants::NOTE_OFF_FADE_TIME;
        double time_since_off = age - note_off_time;
        
        if (time_since_off > 0.0) {
            double release_factor = 1.0 - (time_since_off / release_time);
            release_factor = std::max(0.0, release_factor);
            // Apply exponential release curve
            release_factor = exp(-time_since_off / (release_time * 0.4));
            amplitude *= release_factor;
        }
    }
    
    // Deactivate voice if amplitude is too low
    if (amplitude < 0.0005) {
        active = false;
    }
}

bool Voice::shouldRelease() const {
    return !active || amplitude < 0.001;
}

// PianoSynthesizer Implementation
PianoSynthesizer::PianoSynthesizer()
    : max_voices_(Constants::MAX_VOICES),
      sample_rate_(Constants::SAMPLE_RATE),
      pedal_damping_(0.0f),
      string_tension_(1.0f),
      master_tuning_(0.0f),
      velocity_sensitivity_(Constants::VELOCITY_SENSITIVITY),
      config_manager_(nullptr),
      note_params_manager_(nullptr) {
    
    resonance_model_ = std::make_unique<Physics::ResonanceModel>();
}

PianoSynthesizer::~PianoSynthesizer() {
    shutdown();
}

bool PianoSynthesizer::initialize(Utils::ConfigManager* config_manager) {
    Utils::Logger logger;
    logger.info("Initializing Piano Synthesizer...");
    
    config_manager_ = config_manager;
    
    if (config_manager_) {
        // Load configuration
        sample_rate_ = config_manager_->getDouble("audio.sample_rate", Constants::SAMPLE_RATE);
        max_voices_ = config_manager_->getInt("synthesis.max_voices", Constants::MAX_VOICES);
        velocity_sensitivity_ = config_manager_->getDouble("synthesis.velocity_sensitivity", Constants::VELOCITY_SENSITIVITY);
    }
    
    // Initialize resonance model
    resonance_model_->initialize(sample_rate_, Constants::NUM_KEYS);
    
    // Pre-allocate voice pool
    voice_pool_.reserve(max_voices_);
    for (int i = 0; i < max_voices_; ++i) {
        auto voice = std::make_unique<Voice>(60); // Default to middle C
        voice->initialize(sample_rate_);
        voice_pool_.push_back(std::move(voice));
    }
    
    // Initialize audio buffer
    audio_buffer_.resize(Constants::BUFFER_SIZE * Constants::CHANNELS);
    string_outputs_.resize(Constants::NUM_KEYS, 0.0);
    
    logger.info("Piano Synthesizer initialized with " + std::to_string(max_voices_) + " voices");
    return true;
}

void PianoSynthesizer::shutdown() {
    Utils::Logger logger;
    logger.info("Shutting down Piano Synthesizer...");
    
    // Clear all active voices
    active_voices_.clear();
    voice_pool_.clear();
    
    logger.info("Piano Synthesizer shutdown complete");
}

void PianoSynthesizer::processNoteEvent(const Abstraction::NoteEvent& event) {
    switch (event.type) {
        case Abstraction::NoteEvent::NOTE_ON:
            {
                Voice* voice = allocateVoice(event.note_number);
                if (voice) {
                    voice->noteOn(event);
                }
            }
            break;
            
        case Abstraction::NoteEvent::NOTE_OFF:
            {
                auto it = active_voices_.find(event.note_number);
                if (it != active_voices_.end()) {
                    it->second->noteOff(event);
                }
            }
            break;
            
        case Abstraction::NoteEvent::PEDAL_CHANGE:
            // Update sustain pedal state for all active voices
            for (auto& pair : active_voices_) {
                pair.second->sustain_pedal_active = event.sustain_pedal;

                // If sustain pedal released and note off received, apply damper
                if (!event.sustain_pedal && pair.second->note_off_received) {
                    pair.second->string_model->setDamperPosition(0.0);
                }
            }
            // Propagate sustain level to resonance model
            resonance_model_->setSustainLevel(event.sustain_pedal ? 1.0 : 0.0);
            break;
            
        default:
            break;
    }
}

std::vector<float> PianoSynthesizer::generateAudioBuffer(int buffer_size) {
    // Clear audio buffer
    clearAudioBuffer();
    
    if (buffer_size != static_cast<int>(audio_buffer_.size() / Constants::CHANNELS)) {
        audio_buffer_.resize(buffer_size * Constants::CHANNELS);
    }
    
    // Update all active voices and generate samples
    updateAllVoices();
    
    // Process resonance model
    processResonance();
    
    // Mix voices to buffer
    mixVoicesToBuffer(buffer_size);
    
    // Apply master effects
    applyMasterEffects();
    
    // Clean up inactive voices
    cleanupInactiveVoices();
    
    return audio_buffer_;
}

void PianoSynthesizer::setPedalDamping(float damping) {
    pedal_damping_ = Utils::MathUtils::clamp(damping, 0.0f, 1.0f);
    
    // Apply to all active voices
    for (auto& pair : active_voices_) {
        pair.second->string_model->setDamping(pedal_damping_);
    }
}

void PianoSynthesizer::setStringTension(float tension) {
    string_tension_ = Utils::MathUtils::clamp(tension, 0.5f, 2.0f);
    
    // Apply to all active voices
    for (auto& pair : active_voices_) {
        double base_tension = 1000.0 * (1.0 + (pair.first - 60) * 0.01);
        pair.second->string_model->setTension(base_tension * string_tension_);
    }
}

void PianoSynthesizer::setMasterTuning(float tuning_offset) {
    master_tuning_ = Utils::MathUtils::clamp(tuning_offset, -100.0f, 100.0f); // Cents
    
    // Tuning changes require voice reinitialization, so just store for new voices
}

void PianoSynthesizer::setVelocitySensitivity(float sensitivity) {
    velocity_sensitivity_ = Utils::MathUtils::clamp(sensitivity, 0.001f, 0.1f);
}

void PianoSynthesizer::setSoundboardResonance(float resonance) {
    resonance_model_->setSoundboardResonance(resonance);
}

void PianoSynthesizer::setRoomAcoustics(float size, float damping) {
    resonance_model_->setRoomSize(size);
    resonance_model_->setRoomDamping(damping);
}

void PianoSynthesizer::setStringCoupling(float coupling_strength) {
    // Apply string coupling strength to resonance model
    resonance_model_->setCouplingStrength(coupling_strength);
    
    // Also apply to individual voices for sympathetic resonance
    for (auto& voice : voice_pool_) {
        if (voice->active) {
            voice->string_model->setCouplingStrength(coupling_strength);
        }
    }
}

Voice* PianoSynthesizer::allocateVoice(int note_number) {
    // Check if note is already playing
    auto existing = active_voices_.find(note_number);
    if (existing != active_voices_.end()) {
        // Reuse existing voice
        return existing->second;
    }
    
    // Find available voice from pool
    for (auto& voice : voice_pool_) {
        if (!voice->active) {
            voice->updateNoteNumber(note_number, sample_rate_);
            if (note_params_manager_) {
                voice->applyNoteParams(note_params_manager_->getParams(note_number));
            }

            // Apply master tuning
            if (master_tuning_ != 0.0f) {
                voice->frequency *= pow(2.0, master_tuning_ / 1200.0);
            }

            active_voices_[note_number] = voice.get();
            return voice.get();
        }
    }
    
    // No available voices, steal oldest
    Voice* oldest = findOldestVoice();
    if (oldest) {
        // Remove from active voices
        for (auto it = active_voices_.begin(); it != active_voices_.end(); ++it) {
            if (it->second == oldest) {
                active_voices_.erase(it);
                break;
            }
        }
        
        // Reassign to new note
        oldest->updateNoteNumber(note_number, sample_rate_);
        if (note_params_manager_) {
            oldest->applyNoteParams(note_params_manager_->getParams(note_number));
        }

        // Apply master tuning
        if (master_tuning_ != 0.0f) {
            oldest->frequency *= pow(2.0, master_tuning_ / 1200.0);
        }

        oldest->active = false; // Will be activated by noteOn call
        
        active_voices_[note_number] = oldest;
        return oldest;
    }
    
    return nullptr;
}

void PianoSynthesizer::releaseVoice(int note_number) {
    auto it = active_voices_.find(note_number);
    if (it != active_voices_.end()) {
        active_voices_.erase(it);
    }
}

void PianoSynthesizer::updateAllVoices() {
    std::fill(string_outputs_.begin(), string_outputs_.end(), 0.0);
    
    for (auto& pair : active_voices_) {
        Voice* voice = pair.second;
        if (voice->active) {
            double sample = voice->generateSample();
            
            // Map MIDI note to string index
            int string_index = voice->note_number - Constants::LOWEST_KEY;
            if (string_index >= 0 && string_index < Constants::NUM_KEYS) {
                string_outputs_[string_index] = sample;
                
                // Update resonance model
                resonance_model_->updateStringCoupling(string_index, sample, voice->frequency);
            }
        }
    }
}

void PianoSynthesizer::processResonance() {
    // Add sympathetic resonance to string outputs
    for (int i = 0; i < Constants::NUM_KEYS; ++i) {
        double sympathetic = resonance_model_->getSympatheticResonance(i);
        string_outputs_[i] += sympathetic;
    }
}

void PianoSynthesizer::mixVoicesToBuffer(int buffer_size) {
    // Initialize mix buffers
    std::vector<double> mix_buffer_left(buffer_size, 0.0);
    std::vector<double> mix_buffer_right(buffer_size, 0.0);
    
    // Mix all active voices
    for (auto& pair : active_voices_) {
        Voice* voice = pair.second;
        if (voice->active) {
            // Calculate stereo pan based on note position (piano keyboard layout)
            float pan = (voice->note_number - 60.0f) / 48.0f; // -1.0 to 1.0
            pan = Utils::MathUtils::clamp(pan, -1.0f, 1.0f);
            
            // Equal power panning
            float left_gain = cos((pan + 1.0f) * Utils::MathUtils::PI / 4.0f);
            float right_gain = sin((pan + 1.0f) * Utils::MathUtils::PI / 4.0f);
            
            // Generate samples for this voice
            for (int frame = 0; frame < buffer_size; ++frame) {
                double sample = voice->generateSample();
                mix_buffer_left[frame] += sample * left_gain;
                mix_buffer_right[frame] += sample * right_gain;
            }
        }
    }
    
    // Process through soundboard and room acoustics
    for (int frame = 0; frame < buffer_size; ++frame) {
        // Simple soundboard resonance simulation
        double left_processed = mix_buffer_left[frame];
        double right_processed = mix_buffer_right[frame];
        
        // Add subtle room reverb (simple delay + feedback)
        static std::vector<double> reverb_buffer_left(4410, 0.0); // 100ms at 44.1kHz
        static std::vector<double> reverb_buffer_right(4410, 0.0);
        static int reverb_index = 0;
        
        double reverb_left = reverb_buffer_left[reverb_index] * 0.15;
        double reverb_right = reverb_buffer_right[reverb_index] * 0.15;
        
        reverb_buffer_left[reverb_index] = left_processed + reverb_left * 0.3;
        reverb_buffer_right[reverb_index] = right_processed + reverb_right * 0.3;
        
        reverb_index = (reverb_index + 1) % reverb_buffer_left.size();
        
        left_processed += reverb_left;
        right_processed += reverb_right;
        
        // Convert to float and store in audio buffer with higher output level
        audio_buffer_[frame * Constants::CHANNELS] = static_cast<float>(left_processed * 2.5);
        audio_buffer_[frame * Constants::CHANNELS + 1] = static_cast<float>(right_processed * 2.5);
    }
}

void PianoSynthesizer::applyMasterEffects() {
    // Apply soft clipping and normalization
    applySoftClipping();
    normalizeAudio();
}

Voice* PianoSynthesizer::findOldestVoice() {
    Voice* oldest = nullptr;
    double max_age = 0.0;
    
    for (auto& voice : voice_pool_) {
        if (voice->active && voice->age > max_age) {
            max_age = voice->age;
            oldest = voice.get();
        }
    }
    
    return oldest;
}

void PianoSynthesizer::cleanupInactiveVoices() {
    auto it = active_voices_.begin();
    while (it != active_voices_.end()) {
        if (it->second->shouldRelease()) {
            it->second->active = false;
            it = active_voices_.erase(it);
        } else {
            ++it;
        }
    }
}

bool PianoSynthesizer::isVoiceActive(const Voice* voice) const {
    return voice && voice->active && voice->amplitude > 0.001;
}

void PianoSynthesizer::clearAudioBuffer() {
    std::fill(audio_buffer_.begin(), audio_buffer_.end(), 0.0f);
}

void PianoSynthesizer::normalizeAudio() {
    // Find peak level
    float peak = 0.0f;
    for (float sample : audio_buffer_) {
        peak = std::max(peak, std::abs(sample));
    }
    
    // Apply gentle compression if needed
    if (peak > 0.9f) {
        float gain = 0.9f / peak;
        for (float& sample : audio_buffer_) {
            sample *= gain;
        }
    }
}

void PianoSynthesizer::applySoftClipping() {
    for (float& sample : audio_buffer_) {
        // Apply gentle compression and soft clipping
        sample = Utils::DSPUtils::softClip(sample, 0.7f);
        
        // Additional gentle saturation for warmth
        if (std::abs(sample) > 0.1f) {
            float sign = (sample >= 0.0f) ? 1.0f : -1.0f;
            float abs_sample = std::abs(sample);
            sample = sign * (abs_sample + (abs_sample - 0.1f) * 0.1f);
        }
    }
}

} // namespace Synthesis
} // namespace PianoSynth