# Piano Synth - Modular Architecture

## Overview
The piano synthesizer is redesigned as a modular system with clear separation of concerns using DLLs and configuration files.

## Architecture Components

### 1. Core Engine (DLL)
**Location**: `core/engine/`
**Purpose**: Central orchestrator that takes processed input data and coordinates sound generation and output.
**Responsibilities**:
- Manage instrument DLL loading/unloading
- Route processed input data to appropriate instrument DLLs
- Mix multiple instrument outputs
- Send final audio data to output DLL
- Handle configuration loading and management

### 2. Abstraction Layer (DLL)
**Location**: `core/abstraction/`
**Purpose**: Convert raw MIDI/input data into standardized musical events.
**Configuration**: `config/abstraction.json`
**Responsibilities**:
- Process raw MIDI events into standardized format
- Apply velocity curves, timing corrections
- Handle pedal processing
- Provide configurable event filtering and processing

### 3. Input Layer (DLL)
**Location**: `core/input/`
**Purpose**: Handle MIDI device detection and raw input processing.
**Configuration**: `config/input.json`
**Responsibilities**:
- MIDI device detection and categorization
- Raw MIDI event capture
- Device-specific input handling
- Hot-plug device support

### 4. Instrument DLLs
**Location**: `instruments/`
**Purpose**: Generate audio samples for specific instruments.
**Configuration**: `config/instruments/[instrument].json`

#### 4.1 Piano DLL (`instruments/piano/`)
- Physical modeling synthesis
- String, hammer, and resonance simulation
- Piano-specific configuration

#### 4.2 Future Instruments
- `instruments/organ/` - Organ synthesis
- `instruments/guitar/` - Guitar synthesis  
- `instruments/drums/` - Drum synthesis

### 5. Output Layer (DLL)
**Location**: `core/output/`
**Purpose**: Handle audio output to various targets.
**Configuration**: `config/output.json`
**Responsibilities**:
- OS audio output (ALSA, PortAudio, etc.)
- Bare metal output pins
- Recording and file output
- Configurable output routing

### 6. Main Application
**Location**: `app/`
**Purpose**: Lightweight orchestrator that loads and coordinates DLLs.
**Responsibilities**:
- Load configuration files
- Initialize and manage DLL lifecycle
- Provide simple CLI/API interface

## Data Flow

```
Input DLL -> Abstraction DLL -> Core Engine -> Instrument DLLs -> Output DLL
    ^                ^              ^              ^               ^
    |                |              |              |               |
input.json    abstraction.json  core.json   piano.json      output.json
```

## Standard Interfaces

### Musical Event Format
```cpp
struct MusicalEvent {
    EventType type;           // NOTE_ON, NOTE_OFF, PEDAL, etc.
    double timestamp;         // High precision timing
    int note_number;          // MIDI note number
    float velocity;           // 0.0 - 1.0
    float pressure;           // For aftertouch
    float pitch_bend;         // -1.0 to 1.0
    PedalState pedals;        // Sustain, soft, sostenuto
    // ... other musical parameters
};
```

### Audio Buffer Format
```cpp
struct AudioBuffer {
    float* samples;           // Interleaved audio samples
    size_t frame_count;       // Number of audio frames
    size_t channel_count;     // Number of audio channels
    double sample_rate;       // Sample rate in Hz
    double timestamp;         // Buffer timestamp
};
```

## Configuration System

Each DLL uses JSON/YAML configuration files that can be hot-reloaded:

- `config/core.json` - Core engine settings
- `config/input.json` - Input device configuration
- `config/abstraction.json` - Event processing settings
- `config/output.json` - Output routing configuration
- `config/instruments/piano.json` - Piano-specific settings
- `config/instruments/organ.json` - Organ-specific settings (future)

## Benefits

1. **Modularity**: Each component can be developed, tested, and updated independently
2. **Extensibility**: Easy to add new instruments without changing core
3. **Platform Flexibility**: Output DLL can target different platforms
4. **Configuration-Driven**: Behavior controlled by external config files
5. **Hot-Reload**: Configuration and even DLLs can be reloaded at runtime
6. **Testing**: Each DLL can be unit tested in isolation
7. **Performance**: Only needed DLLs are loaded into memory
