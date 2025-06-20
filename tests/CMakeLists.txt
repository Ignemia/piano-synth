cmake_minimum_required(VERSION 3.12)

# This file is included by the main CMakeLists.txt when BUILD_TESTS is ON
# All test configuration is handled in the parent CMakeLists.txt

# Test data directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_data)

# Copy test configuration files
configure_file(
    ${CMAKE_SOURCE_DIR}/tests/data/test_config.json
    ${CMAKE_BINARY_DIR}/test_data/test_config.json
    COPYONLY
)

# Create test output directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_output)

# Test utility library (common test helpers)
add_library(test_utils STATIC
    test_helpers.cpp
)

target_include_directories(test_utils PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/tests
)

target_link_libraries(test_utils
    piano_utils
    ${GTEST_LIBRARIES}
)

# Individual test executables for better modularity
add_executable(test_math_utils
    test_main.cpp
    test_math_utils.cpp
)

target_link_libraries(test_math_utils
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_config_manager
    test_main.cpp
    test_config_manager.cpp
)

target_link_libraries(test_config_manager
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_cli_arguments
    test_main.cpp
    test_cli_arguments.cpp
)

target_link_libraries(test_cli_arguments
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_note_params_manager
    test_main.cpp
    test_note_params_manager.cpp
)

target_link_libraries(test_note_params_manager
    piano_synthesis
    piano_physics
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_note_settings_cli
    test_main.cpp
    test_note_settings_cli.cpp
)

target_link_libraries(test_note_settings_cli
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_string_model
    test_main.cpp
    test_string_model.cpp
)

target_link_libraries(test_string_model
    piano_physics
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_hammer_model
    test_main.cpp
    test_hammer_model.cpp
)

target_link_libraries(test_hammer_model
    piano_physics
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_wave_equation
    test_main.cpp
    test_wave_equation.cpp
)

target_link_libraries(test_wave_equation
    piano_physics
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_abstraction_layer
    test_main.cpp
    test_abstraction_layer.cpp
)

target_link_libraries(test_abstraction_layer
    piano_abstraction
    piano_input
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_midi_detector
    test_main.cpp
    test_midi_detector.cpp
)

target_link_libraries(test_midi_detector
    piano_input
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    ${RTMIDI_LIBRARY}
    Threads::Threads
)

add_executable(test_note_events
    test_main.cpp
    test_note_events.cpp
)

target_link_libraries(test_note_events
    piano_abstraction
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_piano_synthesizer
    test_main.cpp
    test_piano_synthesizer.cpp
)

target_link_libraries(test_piano_synthesizer
    piano_synthesis
    piano_physics
    piano_abstraction
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_synthesis_integration
    test_main.cpp
    test_synthesis_integration.cpp
)

target_link_libraries(test_synthesis_integration
    piano_synth_lib
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_executable(test_integration
    test_main.cpp
    test_integration.cpp
)

target_link_libraries(test_integration
    piano_synth_lib
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

# Register all tests with CTest
add_test(NAME MathUtils COMMAND test_math_utils)
add_test(NAME ConfigManager COMMAND test_config_manager)
add_test(NAME CliArguments COMMAND test_cli_arguments)
add_test(NAME NoteParamsManager COMMAND test_note_params_manager)
add_test(NAME NoteSettingsCli COMMAND test_note_settings_cli)
add_test(NAME StringModel COMMAND test_string_model)
add_test(NAME HammerModel COMMAND test_hammer_model)
add_test(NAME WaveEquation COMMAND test_wave_equation)
add_test(NAME AbstractionLayer COMMAND test_abstraction_layer)
add_test(NAME MidiDetector COMMAND test_midi_detector)
add_test(NAME NoteEvents COMMAND test_note_events)

add_executable(test_wav_writer
    test_main.cpp
    test_wav_writer.cpp
)

target_link_libraries(test_wav_writer
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_test(NAME WavWriter COMMAND test_wav_writer)

add_executable(test_simple_oscillator
    test_main.cpp
    test_simple_oscillator.cpp
)

target_link_libraries(test_simple_oscillator
    piano_synthesis
    piano_utils
    test_utils
    ${GTEST_LIBRARIES}
    Threads::Threads
)

add_test(NAME SimpleOscillator COMMAND test_simple_oscillator)
add_test(NAME PianoSynthesizer COMMAND test_piano_synthesizer)
add_test(NAME SynthesisIntegration COMMAND test_synthesis_integration)
add_test(NAME SystemIntegration COMMAND test_integration)

# Set test properties
set_tests_properties(MathUtils PROPERTIES TIMEOUT 30)
set_tests_properties(ConfigManager PROPERTIES TIMEOUT 30)
set_tests_properties(CliArguments PROPERTIES TIMEOUT 30)
set_tests_properties(NoteParamsManager PROPERTIES TIMEOUT 30)
set_tests_properties(NoteSettingsCli PROPERTIES TIMEOUT 30)
set_tests_properties(StringModel PROPERTIES TIMEOUT 60)
set_tests_properties(HammerModel PROPERTIES TIMEOUT 60)
set_tests_properties(WaveEquation PROPERTIES TIMEOUT 60)
set_tests_properties(AbstractionLayer PROPERTIES TIMEOUT 30)
set_tests_properties(MidiDetector PROPERTIES TIMEOUT 30)
set_tests_properties(NoteEvents PROPERTIES TIMEOUT 30)
set_tests_properties(WavWriter PROPERTIES TIMEOUT 30)
set_tests_properties(PianoSynthesizer PROPERTIES TIMEOUT 120)
set_tests_properties(SynthesisIntegration PROPERTIES TIMEOUT 120)
set_tests_properties(SystemIntegration PROPERTIES TIMEOUT 180)

# Custom test targets for different test categories
add_custom_target(test_unit
    COMMAND ${CMAKE_CTEST_COMMAND} -R "MathUtils|ConfigManager|NoteParamsManager|StringModel|HammerModel|WaveEquation|AbstractionLayer|NoteEvents" --output-on-failure
    DEPENDS test_math_utils test_config_manager test_note_params_manager test_string_model test_hammer_model test_wave_equation test_abstraction_layer test_note_events
    COMMENT "Running unit tests"
)

add_custom_target(test_component
    COMMAND ${CMAKE_CTEST_COMMAND} -R "MidiDetector|PianoSynthesizer" --output-on-failure
    DEPENDS test_midi_detector test_piano_synthesizer
    COMMENT "Running component tests"
)

add_custom_target(test_integration_only
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Integration" --output-on-failure
    DEPENDS test_synthesis_integration test_integration
    COMMENT "Running integration tests"
)

# Performance tests (optional)
add_custom_target(test_performance
    COMMAND ${CMAKE_CTEST_COMMAND} -R "PianoSynthesizer|Integration" --output-on-failure
    DEPENDS test_piano_synthesizer test_integration
    COMMENT "Running performance-related tests"
)

# Memory tests (if valgrind is available)
find_program(VALGRIND_PROGRAM valgrind)
if(VALGRIND_PROGRAM)
    add_custom_target(test_memory
        COMMAND ${VALGRIND_PROGRAM} --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1
        $<TARGET_FILE:test_piano_synthesizer>
        DEPENDS test_piano_synthesizer
        COMMENT "Running memory tests with Valgrind"
    )
endif()

# Benchmark tests (if available)
option(BUILD_BENCHMARKS "Build benchmark tests" OFF)
if(BUILD_BENCHMARKS)
    find_package(benchmark QUIET)
    if(benchmark_FOUND)
        add_executable(piano_synth_benchmarks
            benchmarks/bench_string_model.cpp
            benchmarks/bench_synthesizer.cpp
        )
        
        target_link_libraries(piano_synth_benchmarks
            piano_synth_lib
            benchmark::benchmark
            benchmark::benchmark_main
        )
        
        add_custom_target(run_benchmarks
            COMMAND piano_synth_benchmarks
            DEPENDS piano_synth_benchmarks
            COMMENT "Running performance benchmarks"
        )
    endif()
endif()

message(STATUS "Test targets configured:")
message(STATUS "  make test          - Run all tests")
message(STATUS "  make test_unit     - Run unit tests only")
message(STATUS "  make test_component - Run component tests only")
message(STATUS "  make test_integration_only - Run integration tests only")
if(VALGRIND_PROGRAM)
    message(STATUS "  make test_memory   - Run memory tests with Valgrind")
endif()
if(BUILD_BENCHMARKS AND benchmark_FOUND)
    message(STATUS "  make run_benchmarks - Run performance benchmarks")
endif()