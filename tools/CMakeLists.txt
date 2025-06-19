cmake_minimum_required(VERSION 3.16)

# String analysis tool
add_executable(string_analyzer
    string_analyzer.cpp
    ../core/physics/string_model.cpp
    ../core/physics/wave_equation_solver.cpp
    ../core/utils/math_utils.cpp
)

target_include_directories(string_analyzer PRIVATE
    ${CMAKE_SOURCE_DIR}/core
)

target_link_libraries(string_analyzer m)

# MIDI device tester
add_executable(midi_tester
    midi_tester.cpp
    ../core/input/midi_detector.cpp
    ../core/utils/logger.cpp
)

target_include_directories(midi_tester PRIVATE
    ${CMAKE_SOURCE_DIR}/core
    ${RTMIDI_INCLUDE_DIR}
)

target_link_libraries(midi_tester ${RTMIDI_LIBRARY})

# Audio latency tester
add_executable(audio_latency_test
    audio_latency_test.cpp
    ../core/utils/logger.cpp
)

target_include_directories(audio_latency_test PRIVATE
    ${CMAKE_SOURCE_DIR}/core
)

target_link_libraries(audio_latency_test ${PORTAUDIO_LIBRARIES})
