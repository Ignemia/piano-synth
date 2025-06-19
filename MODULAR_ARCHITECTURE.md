# Piano Synth - Modular Architecture Implementation

## Project Restructuring Summary

The piano synthesizer has been redesigned from a monolithic application into a modular DLL-based architecture for enhanced flexibility, maintainability, and extensibility.

## New Architecture Benefits

### 1. **Separation of Concerns**
- **Core Engine**: Orchestrates all components, manages DLL lifecycle
- **Input Layer**: MIDI device detection and raw input processing  
- **Abstraction Layer**: Converts raw input to standardized musical events
- **Instrument DLLs**: Generate audio for specific instruments (piano, future organs/guitars/drums)
- **Output Layer**: Handles audio output to various targets (OS audio, bare metal pins)

### 2. **Configuration-Driven Design**
- JSON configuration files for each component
- Hot-reloadable configurations
- Environment-specific settings (OS vs bare metal)
- Extensible instrument definitions

### 3. **Platform Flexibility**
- Output DLL can target different platforms:
  - OS: ALSA, PortAudio, JACK
  - Bare Metal: GPIO pins, DAC outputs
- Input supports various MIDI interfaces
- Modular components can be swapped per platform

### 4. **Extensibility**
- Easy to add new instruments without changing core
- Plugin-style architecture for instruments
- Standard interfaces ensure compatibility
- Future instrument DLLs: organ, guitar, drums, etc.

## File Structure

```
piano-synth/
├── shared/
│   ├── interfaces/           # Standard DLL interfaces
│   │   ├── common_types.h    # MusicalEvent, AudioBuffer, etc.
│   │   └── dll_interfaces.h  # IInputProcessor, IInstrumentSynthesizer, etc.
│   └── utils/               # Shared utilities
│       ├── json_config.h/cpp
│       ├── audio_utils.h/cpp
│       ├── math_utils.h/cpp
│       └── logger.h/cpp
├── core/
│   ├── engine/              # Core orchestration DLL
│   ├── input/               # Input processing DLL
│   ├── abstraction/         # Event abstraction DLL
│   └── output/              # Audio output DLL
├── instruments/
│   ├── piano/               # Piano synthesizer DLL
│   ├── organ/               # Future: Organ DLL
│   └── guitar/              # Future: Guitar DLL
├── app/                     # Lightweight main application
├── config/                  # Configuration files
│   ├── core.json
│   ├── input.json
│   ├── abstraction.json
│   ├── output.json
│   └── instruments/
│       ├── piano.json
│       └── organ.json       # Future
├── tests/                   # Modular integration tests
└── CMakeLists_modular.txt   # New build system
```

## Standard Data Flow

```
Raw MIDI Input → Input DLL → Raw Events
                                 ↓
              Abstraction DLL → Musical Events  
                                 ↓
              Core Engine → Route to Instrument DLLs
                                 ↓
              Piano DLL → Audio Buffers
                                 ↓
              Output DLL → Hardware/OS Audio
```

## Key Interfaces

### MusicalEvent Structure
```cpp
struct MusicalEvent {
    EventType type;              // NOTE_ON, NOTE_OFF, PEDAL_CHANGE, etc.
    std::chrono::high_resolution_clock::time_point timestamp;
    int note_number;             // MIDI note (0-127)
    float velocity;              // 0.0 - 1.0
    float pitch_bend;            // -1.0 to +1.0 semitones
    PedalState pedals;           // Sustain, soft, sostenuto
    int channel;                 // MIDI channel (0-15)
    // ... other musical parameters
};
```

### AudioBuffer Structure
```cpp
struct AudioBuffer {
    float* samples;              // Interleaved audio samples
    size_t frame_count;          // Number of audio frames
    size_t channel_count;        // Number of channels (usually 2)
    double sample_rate;          // Sample rate in Hz
    std::chrono::high_resolution_clock::time_point timestamp;
};
```

## Configuration System

Each DLL is configured via JSON files that can be hot-reloaded:

- **core.json**: Engine settings, DLL paths, audio configuration
- **input.json**: MIDI device patterns, input processing settings  
- **abstraction.json**: Velocity curves, timing, pedal processing
- **output.json**: Audio backend selection, latency settings, recording
- **instruments/piano.json**: Physical modeling parameters, effects

## Building the Modular System

### Prerequisites
```bash
# Install dependencies
sudo apt install -y libasound2-dev libportaudio2 portaudio19-dev 
sudo apt install -y librtmidi-dev libmp3lame-dev libgtest-dev
```

### Build Process
```bash
# Use the new modular CMakeLists.txt
cp CMakeLists_modular.txt CMakeLists.txt

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# This creates:
# - build/dlls/libpiano_input.so
# - build/dlls/libpiano_abstraction.so  
# - build/dlls/libpiano_instrument.so
# - build/dlls/libpiano_output.so
# - build/dlls/libpiano_core.so
# - build/bin/piano_synth (main app)
```

## Testing Framework

### Comprehensive Test Suite
The new test suite includes:

1. **Unit Tests**: Individual component testing
   - `test_common_types.cpp`: Interface structure validation
   - `test_math_utils.cpp`: Mathematical utilities
   
2. **Integration Tests**: DLL interaction testing
   - `test_modular_abstraction.cpp`: Abstraction layer DLL tests
   - `test_modular_instrument.cpp`: Piano instrument DLL tests
   - `test_modular_integration.cpp`: Full pipeline testing

3. **Test Categories**:
   - Single note processing
   - Chord synthesis (multiple simultaneous notes)
   - Velocity response curves
   - Sustain pedal effects
   - Note on/off sequencing
   - Error handling and edge cases
   - Audio quality validation
   - Performance and latency testing

### Running Tests
```bash
# Build and run all tests
cmake --build build
cd build && ctest

# Run specific test categories
./tests/piano_synth_modular_tests --gtest_filter="*Chord*"
./tests/piano_synth_modular_tests --gtest_filter="*Velocity*"
./tests/piano_synth_modular_tests --gtest_filter="*Integration*"
```

## Usage Examples

### Basic Usage
```bash
# Run with default configuration
./build/bin/piano_synth

# Use custom configuration
./build/bin/piano_synth -c my_config.json

# List available MIDI devices
./build/bin/piano_synth --list-devices

# Test audio output
./build/bin/piano_synth --test-audio

# Run as daemon
./build/bin/piano_synth --daemon
```

### Runtime Configuration
The modular design allows runtime reconfiguration:
- Change velocity curves without restart
- Switch audio backends on-the-fly
- Load/unload instrument DLLs dynamically
- Update physical modeling parameters in real-time

## Future Extensions

### New Instrument DLLs
The standardized interfaces make it easy to add:
- **Organ DLL**: Hammond organ simulation, drawbar controls
- **Guitar DLL**: String modeling, pickup simulation, effects
- **Drum DLL**: Physical drum modeling, sample layering
- **Synthesizer DLL**: Subtractive/FM synthesis engines

### Platform Adaptations
- **Embedded Systems**: Optimized DLLs for ARM processors
- **Real-time Systems**: Low-latency, deterministic variants
- **DSP Hardware**: Offload synthesis to dedicated chips
- **Cloud/Server**: Headless audio generation services

## Development Workflow

1. **Individual Component Development**: Each DLL can be developed and tested independently
2. **Interface Compliance**: All DLLs must implement standard interfaces
3. **Configuration Testing**: Each component validates its JSON configuration
4. **Integration Validation**: Full pipeline tests ensure compatibility
5. **Performance Profiling**: Per-DLL performance monitoring
6. **Hot-reload Testing**: Configuration changes without restart

This modular architecture provides a solid foundation for a professional-grade, extensible piano synthesizer while maintaining the flexibility to add new instruments and adapt to different platforms.
