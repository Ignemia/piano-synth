cmake_minimum_required(VERSION 3.10)
project(piano_synth)

include_directories(${CMAKE_SOURCE_DIR}/include)

enable_testing()
set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(Threads REQUIRED)

# Platform-specific MIDI libraries
if(WIN32)
    set(MIDI_LIBRARIES winmm)
elseif(APPLE)
    find_library(COREMIDI_LIBRARY CoreMIDI)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    set(MIDI_LIBRARIES ${COREMIDI_LIBRARY} ${COREFOUNDATION_LIBRARY})
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA REQUIRED alsa)
    set(MIDI_LIBRARIES ${ALSA_LIBRARIES})
    include_directories(${ALSA_INCLUDE_DIRS})
endif()

add_library(MidiInput SHARED src/MidiInput.cpp)
target_include_directories(MidiInput PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(Abstractor SHARED src/Abstractor.cpp)
target_include_directories(Abstractor PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(NoteSynth SHARED src/NoteSynth.cpp)
target_include_directories(NoteSynth PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(OutputHandler SHARED src/OutputHandler.cpp)
target_include_directories(OutputHandler PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(MidiDevice SHARED src/MidiDevice.cpp)
target_include_directories(MidiDevice PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(MidiDevice ${MIDI_LIBRARIES} Threads::Threads)

add_executable(piano_synth src/main.cpp)
target_link_libraries(piano_synth MidiInput Abstractor NoteSynth OutputHandler MidiDevice)

add_executable(test_synth tests/test_synth.cpp)
target_include_directories(test_synth PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_synth MidiInput Abstractor NoteSynth OutputHandler MidiDevice)
add_test(NAME SynthTests COMMAND test_synth)

# Unit tests
add_executable(test_midi_input tests/unit/test_midi_input.cpp)
target_include_directories(test_midi_input PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_midi_input MidiInput)
add_test(NAME MidiInputUnitTests COMMAND test_midi_input)

add_executable(test_abstractor tests/unit/test_abstractor.cpp)
target_include_directories(test_abstractor PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_abstractor Abstractor MidiInput)
add_test(NAME AbstractorUnitTests COMMAND test_abstractor)

# MIDI device tests
add_executable(test_midi_device tests/midi/test_midi_device.cpp)
target_include_directories(test_midi_device PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_midi_device MidiDevice MidiInput Threads::Threads)
add_test(NAME MidiDeviceTests COMMAND test_midi_device)

# Custom test target to run all tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_synth test_midi_input test_abstractor test_midi_device
    COMMENT "Running all test suites"
)
