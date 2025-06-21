# AGENTS

## Scope
These instructions apply to the entire repository.

## Coding Guidelines
- Use the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with Hungarian prefix notation for identifiers.
- Keep the existing docstring requirement: all new or modified functions, classes or modules must include Doxygen-style docstrings.
- Include a literal `[AI GENERATED]` tag in the docstring or as a comment near the top of newly created files to indicate AI involvement.
- Each class or structure should reside in its own header and implementation file pair unless a header-only design is justified. Avoid creating more files than necessary.

## Build Guidelines
- Every class except the one defined in `main.cpp` should be compiled into a separate shared library (DLL).

## Testing Guidelines
- Provide unit tests that exercise every line of newly written or modified code. Avoid mocks or simulated behavior unless specifically required.
- Use the real functionality for tests.
- Run `./build_and_test.sh` after any code changes and ensure all tests pass before committing.
- If a change only affects documentation, running tests is optional.

## File Overview
- **CMakeLists.txt** – CMake build script that defines targets for each shared library, the main executable and the test executable.
- **build_and_test.sh** – Helper script to configure, build and run the test suite.
- **include/MidiInput.h** and **src/MidiInput.cpp** – Provides `MidiInput` class responsible for generating example MIDI messages.
- **include/Abstractor.h** and **src/Abstractor.cpp** – Contains `Abstractor` class for translating MIDI messages into frequency-based note events.
- **include/NoteSynth.h** and **src/NoteSynth.cpp** – Implements `NoteSynth` class which turns note events into audio samples using a simple sine wave.
- **include/OutputHandler.h** and **src/OutputHandler.cpp** – Defines `OutputHandler` class that writes synthesized audio samples to a WAV file.
- **src/main.cpp** – Entry point for the application offering demo playback or realtime mode (not yet implemented).
- **tests/test_synth.cpp** – Integration tests covering the behavior of all classes without using mocks.

## CLI Usage
The `piano_synth` executable accepts `--record` to begin recording immediately and `--config` to display the note settings manager.
