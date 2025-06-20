/**
 * @file simple_oscillator.cpp
 * @brief [AI GENERATED] Minimal oscillator-based piano instrument implementation.
 */

#include "../../shared/interfaces/dll_interfaces.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace PianoSynth {
namespace Instruments {

/**
 * @brief [AI GENERATED] Simple oscillator voice state.
 */
struct Voice {
    int note = 0;
    double frequency = 0.0;
    double phase = 0.0;
    float amplitude = 0.0f;
    bool releasing = false;
};

/**
 * @brief [AI GENERATED] Piano instrument that generates sine waves per note.
 */
class SimpleOscillatorInstrument : public Interfaces::IInstrumentSynthesizer {
public:
    bool initialize(const char* /*config_json*/, double sample_rate,
                    size_t /*max_buffer_size*/) override {
        sample_rate_ = sample_rate;
        voices_.clear();
        return true;
    }

    const char* get_instrument_name() override { return "SimpleOscillatorPiano"; }

    const char* get_instrument_version() override { return "0.1"; }

    int get_polyphony_limit() override { return static_cast<int>(polyphony_); }

    bool process_events(const Interfaces::MusicalEvent* events,
                        int event_count) override {
        if (!events || event_count <= 0) {
            return false;
        }

        for (int i = 0; i < event_count; ++i) {
            const auto& ev = events[i];
            switch (ev.type) {
            case Interfaces::EventType::NOTE_ON:
                add_voice(ev.note_number, ev.velocity);
                break;
            case Interfaces::EventType::NOTE_OFF:
                release_voice(ev.note_number);
                break;
            default:
                break;
            }
        }
        return true;
    }

    int generate_audio(Interfaces::AudioBuffer* buffer) override {
        if (!buffer || !buffer->samples || buffer->frame_count == 0) {
            return 0;
        }

        for (size_t i = 0; i < buffer->total_samples(); ++i) {
            buffer->samples[i] = 0.0f;
        }

        for (size_t frame = 0; frame < buffer->frame_count; ++frame) {
            float mix = 0.0f;
            for (auto& v : voices_) {
                mix += std::sin(v.phase) * v.amplitude;
                v.phase += 2.0 * M_PI * v.frequency / sample_rate_;
                if (v.releasing) {
                    v.amplitude -= release_rate_;
                }
            }

            voices_.erase(std::remove_if(voices_.begin(), voices_.end(),
                                         [](const Voice& v) {
                                             return v.amplitude <= 0.0f;
                                         }),
                          voices_.end());

            for (size_t ch = 0; ch < buffer->channel_count; ++ch) {
                buffer->samples[frame * buffer->channel_count + ch] = mix;
            }
        }
        return static_cast<int>(buffer->frame_count);
    }

    bool update_config(const char* /*config_json*/) override { return true; }

    bool set_parameter(const char* /*parameter_name*/, float /*value*/) override {
        return false;
    }

    float get_parameter(const char* /*parameter_name*/) override { return 0.0f; }

    void all_notes_off() override { voices_.clear(); }

    void reset() override { voices_.clear(); }

    void shutdown() override { voices_.clear(); }

private:
    void add_voice(int note, float velocity) {
        if (voices_.size() >= polyphony_) {
            return;
        }
        Voice v;
        v.note = note;
        v.frequency = 440.0 * std::pow(2.0, (note - 69) / 12.0);
        v.phase = 0.0;
        v.amplitude = velocity;
        v.releasing = false;
        voices_.push_back(v);
    }

    void release_voice(int note) {
        for (auto& v : voices_) {
            if (v.note == note && !v.releasing) {
                v.releasing = true;
            }
        }
    }

    double sample_rate_ = 44100.0;
    std::vector<Voice> voices_;
    size_t polyphony_ = 64;
    const float release_rate_ = 0.001f;
};

} // namespace Instruments
} // namespace PianoSynth

extern "C" {
/**
 * @brief [AI GENERATED] Create a new simple oscillator piano instrument.
 */
PianoSynth::Interfaces::IInstrumentSynthesizer* create_instrument_synthesizer() {
    return new PianoSynth::Instruments::SimpleOscillatorInstrument();
}

/**
 * @brief [AI GENERATED] Destroy a previously created piano instrument.
 */
void destroy_instrument_synthesizer(
    PianoSynth::Interfaces::IInstrumentSynthesizer* synth) {
    delete synth;
}
}
