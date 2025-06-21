# Piano Synthesizer - Physical Modeling

A realistic piano synthesizer that uses physical modeling instead of sampling. This project implements a complete three-layer architecture for MIDI input processing, abstraction, and physical synthesis of piano sounds.

## Architecture Overview

### Layer 1: Input Layer
- **MIDI Detection**: Automatically detects and identifies MIDI devices, specifically targeting the M-AUDIO Oxygen Pro 61
- **Device Categorization**: Distinguishes between piano, drum, and controller inputs
- **Real-time Processing**: Low-latency MIDI event capture and processing

### Layer 2: Abstraction Layer  
- **Event Processing**: Converts raw MIDI data into abstracted note events
- **Physical Parameters**: Calculates hammer velocity, string excitation, and damper positions
- **Timing Precision**: Tracks note press/release times with high precision

### Layer 3: Synthesis/Output Layer
- **Physical Modeling**: Complete piano physics simulation including:
  - String vibration using wave equation
  - Hammer-string interaction
  - Soundboard resonance
  - Sympathetic string coupling
- **Audio Output**: Real-time audio generation and device management
- **Recording**: Session recording to high-quality MP3 files

## Physical Modeling Details

### String Model (`core/physics/string_model.h`)
- **Wave Equation**: Solves the full 1D wave equation with stiffness and damping
- **Boundary Conditions**: Models fixed ends with realistic impedance
- **Nonlinear Effects**: Includes string stiffness and tension modulation
- **Harmonic Content**: Generates natural harmonic series with realistic decay

### Hammer Model (`core/physics/hammer_model.h`)
- **Contact Dynamics**: Models nonlinear hammer-string contact
- **Felt Compression**: Simulates felt hardness and compression effects
- **Velocity Scaling**: Realistic velocity-to-force conversion per note
- **Contact Time**: Physically accurate contact duration calculation

### Resonance Model (`core/physics/resonance_model.h`)
- **Sympathetic Resonance**: Strings influence each other based on harmonic relationships
- **Soundboard Coupling**: Multi-resonant soundboard model with frequency-dependent response
- **Room Acoustics**: Simple but effective reverb and acoustic space simulation

### Wave Equation Solver (`core/physics/wave_equation_solver.h`)
- **Finite Difference**: Stable numerical solution of the wave equation
- **Boundary Handling**: Multiple boundary condition types (fixed, free, damped)
- **Stiffness Terms**: Fourth-order derivatives for realistic string behavior
- **Damping Models**: Frequency-dependent damping for realistic decay

## Key Features

### Realistic Physics
- **No Sampling**: Pure mathematical modeling of piano physics
- **String Theory**: Based on actual string vibration equations
- **String Noise**: Subtle noise and natural decay emulate real string dynamics
- **Material Properties**: Uses real physical constants for steel strings, wood soundboard
- **Harmonic Accuracy**: Natural generation of harmonic content and beating

### Performance Optimized
- **Multi-threading**: Separate threads for MIDI, audio, and synthesis
- **Voice Management**: Efficient allocation and cleanup of note voices
- **Buffer Management**: Optimized audio buffering for low latency
- **SIMD Ready**: Code structure prepared for vectorization

### Professional Features
- **Session Recording**: High-quality MP3 encoding with LAME
- **Real-time Controls**: Pedal effects, string tension, room acoustics
- **Device Integration**: Automatic recognition of M-AUDIO Oxygen Pro 61
- **Configuration**: Extensive parameter control via JSON configuration

## Building

### Dependencies
```bash
# Ubuntu/Debian
sudo apt install libasound2-dev libportaudio19-dev libmp3lame-dev
sudo apt install librtmidi-dev cmake build-essential

# Install RtMidi if not available in package manager
git clone https://github.com/thestk/rtmidi.git
cd rtmidi && mkdir build && cd build
cmake .. && make && sudo make install
```

### Compilation
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Running
```bash
./piano_synth
# Press 'r' to start/stop recording
# Press 'q' to quit
```

## Project Structure

```
piano-synth/
├── CMakeLists.txt              # Main build configuration
├── README.md                   # This file
├── config/
│   └── piano_config.json      # Synthesis parameters
├── core/
│   ├── main.cpp               # Application entry point
│   ├── piano_synth.{h,cpp}    # Main application class
│   ├── input/                 # Layer 1: MIDI Input
│   │   ├── midi_detector.{h,cpp}
│   │   ├── midi_input_manager.{h,cpp}
│   │   └── device_identifier.{h,cpp}
│   ├── abstraction/           # Layer 2: Event Abstraction
│   │   ├── input_abstractor.{h,cpp}
│   │   ├── note_event.h
│   │   └── event_processor.{h,cpp}
│   ├── physics/               # Physics Engine
│   │   ├── string_model.{h,cpp}
│   │   ├── hammer_model.{h,cpp}
│   │   ├── resonance_model.{h,cpp}
│   │   ├── soundboard_model.{h,cpp}
│   │   ├── damper_model.{h,cpp}
│   │   └── wave_equation_solver.{h,cpp}
│   ├── synthesis/             # Layer 3: Synthesis
│   │   ├── piano_synthesizer.{h,cpp}
│   │   ├── voice_manager.{h,cpp}
│   │   ├── envelope_generator.{h,cpp}
│   │   └── filter_bank.{h,cpp}
│   ├── audio/                 # Audio I/O
│   │   ├── audio_output_manager.{h,cpp}
│   │   ├── buffer_manager.{h,cpp}
│   │   ├── recording_manager.{h,cpp}
│   │   └── mp3_encoder.{h,cpp}
│   └── utils/                 # Utilities
│       ├── math_utils.{h,cpp}
│       ├── dsp_utils.{h,cpp}
│       ├── config_manager.{h,cpp}
│       ├── logger.{h,cpp}
│       └── constants.h
├── tests/                     # Unit tests
│   ├── CMakeLists.txt
│   ├── test_string_model.cpp
│   ├── test_hammer_model.cpp
│   └── test_wave_equation.cpp
└── tools/                     # Analysis tools
    ├── CMakeLists.txt
    ├── string_analyzer.cpp    # String physics analyzer
    ├── midi_tester.cpp        # MIDI device tester
    └── audio_latency_test.cpp # Audio latency measurement
```

## Configuration

The `config/piano_config.json` file contains all synthesis parameters:

- **String Physics**: Tension, damping, stiffness, coupling
- **Hammer Properties**: Mass, felt hardness, contact dynamics
- **Soundboard**: Resonance frequencies, damping, coupling strength
- **Room Acoustics**: Size, reverb time, damping
- **Audio Settings**: Sample rate, buffer size, device selection
- **MIDI Settings**: Velocity curves, device preferences

## Technical Notes

### Numerical Stability
- Uses implicit finite difference schemes where necessary
- Adaptive time stepping for hammer contact simulation
- Stability checking for wave equation parameters

### Real-time Performance
- Voice pooling to avoid memory allocation in audio thread
- Lock-free audio buffer management where possible
- Optimized inner loops for DSP processing

### Accuracy vs Performance
- Configurable spatial resolution for string models
- Variable quality settings for different performance requirements
- Adaptive voice management based on system capabilities

## Future Enhancements

- **GPU Acceleration**: CUDA implementation for parallel string processing
- **Advanced Room Modeling**: Ray-tracing acoustic simulation
- **String Coupling**: More sophisticated string-to-string interaction
- **Pedal Modeling**: Detailed sustain, soft, and sostenuto pedal physics
- **Multi-sampling**: Hybrid approach combining physics with selective sampling

## License

This project is designed for educational and research purposes in physical modeling synthesis.
