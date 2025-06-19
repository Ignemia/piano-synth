#pragma once

#include "common_types.h"

namespace PianoSynth {
namespace Interfaces {

// Input DLL Interface
class IInputProcessor {
public:
    virtual ~IInputProcessor() = default;
    
    // Initialize the input system
    virtual bool initialize(const char* config_json) = 0;
    
    // Detect and enumerate input devices
    virtual int detect_devices() = 0;
    virtual const char* get_device_name(int device_id) = 0;
    virtual const char* get_device_type(int device_id) = 0; // "piano", "drum", "controller"
    
    // Device management
    virtual bool open_device(int device_id) = 0;
    virtual void close_device(int device_id) = 0;
    virtual bool is_device_open(int device_id) = 0;
    
    // Event processing
    virtual int poll_events(MusicalEvent* events, int max_events) = 0;
    
    // Configuration
    virtual bool update_config(const char* config_json) = 0;
    
    // Cleanup
    virtual void shutdown() = 0;
};

// Abstraction DLL Interface
class IAbstractionProcessor {
public:
    virtual ~IAbstractionProcessor() = default;
    
    // Initialize the abstraction system
    virtual bool initialize(const char* config_json) = 0;
    
    // Process raw events into standardized musical events
    virtual int process_events(const MusicalEvent* input_events, int input_count,
                              MusicalEvent* output_events, int max_output) = 0;
    
    // Configuration for processing behavior
    virtual bool set_velocity_curve(float curve) = 0;
    virtual bool set_timing_quantization(float amount) = 0;
    virtual bool set_pedal_sensitivity(float sensitivity) = 0;
    
    // Configuration updates
    virtual bool update_config(const char* config_json) = 0;
    
    // State management
    virtual void reset_state() = 0;
    virtual void shutdown() = 0;
};

// Instrument DLL Interface
class IInstrumentSynthesizer {
public:
    virtual ~IInstrumentSynthesizer() = default;
    
    // Initialize the instrument
    virtual bool initialize(const char* config_json, double sample_rate, size_t max_buffer_size) = 0;
    
    // Instrument information
    virtual const char* get_instrument_name() = 0;
    virtual const char* get_instrument_version() = 0;
    virtual int get_polyphony_limit() = 0;
    
    // Audio generation
    virtual bool process_events(const MusicalEvent* events, int event_count) = 0;
    virtual int generate_audio(AudioBuffer* buffer) = 0;
    
    // Real-time configuration
    virtual bool update_config(const char* config_json) = 0;
    virtual bool set_parameter(const char* parameter_name, float value) = 0;
    virtual float get_parameter(const char* parameter_name) = 0;
    
    // State management
    virtual void all_notes_off() = 0;
    virtual void reset() = 0;
    virtual void shutdown() = 0;
};

// Output DLL Interface
class IOutputProcessor {
public:
    virtual ~IOutputProcessor() = default;
    
    // Initialize output system
    virtual bool initialize(const char* config_json) = 0;
    
    // Output device management
    virtual int get_output_device_count() = 0;
    virtual const char* get_output_device_name(int device_id) = 0;
    virtual bool set_output_device(int device_id) = 0;
    virtual bool is_output_active() = 0;
    
    // Audio output
    virtual bool queue_audio(const AudioBuffer* buffer) = 0;
    virtual int get_output_latency_ms() = 0;
    virtual int get_buffer_fill_level() = 0;
    
    // Recording
    virtual bool start_recording(const char* filename) = 0;
    virtual void stop_recording() = 0;
    virtual bool is_recording() = 0;
    
    // Configuration
    virtual bool update_config(const char* config_json) = 0;
    virtual bool set_volume(float volume) = 0; // 0.0 - 1.0
    virtual float get_volume() = 0;
    
    // Cleanup
    virtual void shutdown() = 0;
};

// Core Engine Interface
class ICoreEngine {
public:
    virtual ~ICoreEngine() = default;
    
    // Initialize core engine
    virtual bool initialize(const char* config_json) = 0;
    
    // DLL management
    virtual bool load_input_dll(const char* dll_path) = 0;
    virtual bool load_abstraction_dll(const char* dll_path) = 0;
    virtual bool load_instrument_dll(const char* dll_path, const char* instrument_id) = 0;
    virtual bool load_output_dll(const char* dll_path) = 0;
    
    // Processing control
    virtual bool start_processing() = 0;
    virtual void stop_processing() = 0;
    virtual bool is_processing() = 0;
    
    // Configuration management
    virtual bool load_config_file(const char* config_file) = 0;
    virtual bool update_dll_config(const char* dll_name, const char* config_json) = 0;
    
    // Runtime control
    virtual void set_master_volume(float volume) = 0;
    virtual float get_master_volume() = 0;
    virtual int get_active_voice_count() = 0;
    virtual double get_cpu_usage() = 0;
    
    // Cleanup
    virtual void shutdown() = 0;
};

} // namespace Interfaces
} // namespace PianoSynth

// C-style export functions for DLL loading
extern "C" {
    // Each DLL exports its main interface creation function
    PianoSynth::Interfaces::IInputProcessor* create_input_processor();
    PianoSynth::Interfaces::IAbstractionProcessor* create_abstraction_processor();
    PianoSynth::Interfaces::IInstrumentSynthesizer* create_instrument_synthesizer();
    PianoSynth::Interfaces::IOutputProcessor* create_output_processor();
    PianoSynth::Interfaces::ICoreEngine* create_core_engine();
    
    // DLL cleanup functions
    void destroy_input_processor(PianoSynth::Interfaces::IInputProcessor* processor);
    void destroy_abstraction_processor(PianoSynth::Interfaces::IAbstractionProcessor* processor);
    void destroy_instrument_synthesizer(PianoSynth::Interfaces::IInstrumentSynthesizer* synthesizer);
    void destroy_output_processor(PianoSynth::Interfaces::IOutputProcessor* processor);
    void destroy_core_engine(PianoSynth::Interfaces::ICoreEngine* engine);
}
