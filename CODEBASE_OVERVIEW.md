# Codebase Overview

This document summarizes the purpose of each directory and the files contained within the project.

## Directories

- **app/** – Helper code for running the synthesizer.
  - `application.*` – small wrapper class used by early examples.
  - `cli_interface.cpp` – prototype command line interface.
  - `cli_arguments.*` – [AI GENERATED] parser for `--record` and `--config` options.
  - `main.cpp` – legacy entry point not used by the default build.
- **core/** – Implementation of the physical modeling synthesizer.
  - `main.cpp` – application entry point used by `piano_synth` executable.
  - `piano_synth.*` – high level orchestration of MIDI, synthesis and audio.
  - `abstraction/` – converts raw MIDI messages into note events.
  - `audio/` – PortAudio output and MP3 recording management.
  - `engine/` – modular engine components and DLL loading helpers.
  - `input/` – device detection and MIDI event collection.
  - `output/` – platform specific audio outputs and recording back end.
  - `physics/` – mathematical models for strings, hammer and resonance.
  - `synthesis/` – polyphonic voice manager and sound generation.
  - `utils/` – reusable helpers such as configuration and logging.
- **config/** – JSON configuration files controlling the synthesizer.
- **instruments/** – instrument specific implementations, currently only piano.
- **shared/** – common interfaces and utilities for modular builds.
- **tests/** – GoogleTest based unit and integration tests.
- **third_party/** – external single header dependencies (nlohmann/json).
- **tools/** – optional analysis utilities.

## Miscellaneous Files

- `CMakeLists.txt` – main build script.
- `README.md` – project documentation and build instructions.
- `ARCHITECTURE.md` – design notes for the modular system.
- `SUCCESS_SUMMARY.md` – historical summary of implemented features.


## Files By Directory

### app
- `application.cpp` / `application.h` – basic application wrapper used in examples.
- `cli_arguments.cpp` / `cli_arguments.h` – [AI GENERATED] command line parser.
- `cli_interface.cpp` – toy command processor for experiments.
- `main.cpp` – alternative entry point kept for reference.

### core/utils
- `config_manager.*` – load and store JSON configuration files.
- `constants.h` – compile time values shared across the code.
- `logger.*` – simple console logger.
- `math_utils.*` – small mathematical helper functions.
- `note_params_manager.*` – load per-note synthesis parameters.
- `note_settings_cli.*` – [AI GENERATED] CLI to display parameters.

### instruments/piano
- `hammer_model.cpp` – simulates hammer impact on the string.
- `piano_synthesizer.cpp` – instrument level sound generator.
- `resonance_model.cpp` – coupling between strings and soundboard.
- `string_model.cpp` – physical string vibration model.
- `wave_equation_solver.cpp` – numerical solver for the string model.

### tests
- `test_*.cpp` – unit and integration tests covering all libraries.

