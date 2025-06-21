cmake_minimum_required(VERSION 3.10)
project(piano_synth)

include_directories(${CMAKE_SOURCE_DIR}/include)

enable_testing()
set(CMAKE_CXX_STANDARD 17)

add_library(MidiInput SHARED src/MidiInput.cpp)
target_include_directories(MidiInput PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(Abstractor SHARED src/Abstractor.cpp)
target_include_directories(Abstractor PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(NoteSynth SHARED src/NoteSynth.cpp)
target_include_directories(NoteSynth PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(OutputHandler SHARED src/OutputHandler.cpp)
target_include_directories(OutputHandler PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(piano_synth src/main.cpp)
target_link_libraries(piano_synth MidiInput Abstractor NoteSynth OutputHandler)

add_executable(test_synth tests/test_synth.cpp)
target_include_directories(test_synth PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_synth MidiInput Abstractor NoteSynth OutputHandler)
add_test(NAME SynthTests COMMAND test_synth)
