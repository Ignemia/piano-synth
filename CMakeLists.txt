cmake_minimum_required(VERSION 3.12)
project(PianoSynth VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
message(STATUS "=== Piano Synthesizer Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# Find PortAudio
find_path(PORTAUDIO_INCLUDE_DIR portaudio.h)
find_library(PORTAUDIO_LIBRARY portaudio)
if(NOT PORTAUDIO_INCLUDE_DIR OR NOT PORTAUDIO_LIBRARY)
    message(FATAL_ERROR "PortAudio not found. Please install portaudio19-dev")
endif()

# Find LAME
find_path(LAME_INCLUDE_DIR lame/lame.h)
find_library(LAME_LIBRARY mp3lame)
if(NOT LAME_INCLUDE_DIR OR NOT LAME_LIBRARY)
    message(FATAL_ERROR "LAME not found. Please install libmp3lame-dev")
endif()

# Find RtMidi
find_path(RTMIDI_INCLUDE_DIR rtmidi/RtMidi.h)
find_library(RTMIDI_LIBRARY rtmidi)
if(NOT RTMIDI_INCLUDE_DIR OR NOT RTMIDI_LIBRARY)
    message(FATAL_ERROR "RtMidi not found. Please install librtmidi-dev")
endif()

# System libraries
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/shared
    ${CMAKE_SOURCE_DIR}/third_party
    ${PORTAUDIO_INCLUDE_DIR}
    ${LAME_INCLUDE_DIR}
    ${RTMIDI_INCLUDE_DIR}
    ${RTMIDI_INCLUDE_DIR}/rtmidi
)

# Core utilities library
add_library(piano_utils STATIC
    core/utils/logger.cpp
    core/utils/math_utils.cpp
    core/utils/config_manager.cpp
    core/utils/note_params_manager.cpp
)

target_link_libraries(piano_utils
    Threads::Threads
)

# Physics engine library
add_library(piano_physics STATIC
    core/physics/string_model.cpp
    core/physics/hammer_model.cpp
    core/physics/resonance_model.cpp
    core/physics/wave_equation_solver.cpp
)

target_link_libraries(piano_physics
    piano_utils
)

# Input processing library
add_library(piano_input STATIC
    core/input/midi_detector.cpp
    core/input/midi_input_manager.cpp
    core/input/device_identifier.cpp
    core/input/device_manager.cpp
)

target_link_libraries(piano_input
    piano_utils
    ${RTMIDI_LIBRARY}
)

# Abstraction layer library
add_library(piano_abstraction STATIC
    core/abstraction/input_abstractor.cpp
)

target_link_libraries(piano_abstraction
    piano_utils
    piano_input
)

# Synthesis engine library
add_library(piano_synthesis STATIC
    core/synthesis/piano_synthesizer.cpp
)

target_link_libraries(piano_synthesis
    piano_utils
    piano_physics
    piano_abstraction
)

# Audio I/O library
add_library(piano_audio STATIC
    core/audio/audio_output_manager.cpp
    core/audio/recording_manager.cpp
)

target_link_libraries(piano_audio
    piano_utils
    ${PORTAUDIO_LIBRARY}
    ${LAME_LIBRARY}
)

# Main piano synthesizer library
add_library(piano_synth_lib STATIC
    core/piano_synth.cpp
)

target_link_libraries(piano_synth_lib
    piano_utils
    piano_input
    piano_abstraction
    piano_synthesis
    piano_audio
)

# Main executable
add_executable(piano_synth
    core/main.cpp
)

target_link_libraries(piano_synth
    piano_synth_lib
    Threads::Threads
)

# Demo tune player executable
add_executable(demo_tune
    demo_tune.cpp
)

target_link_libraries(demo_tune
    piano_synth_lib
    Threads::Threads
)

# Piano sound quality test executable
add_executable(test_piano_sound
    test_piano_sound.cpp
)

target_link_libraries(test_piano_sound
    piano_synth_lib
    Threads::Threads
)

# Create config directory and default config file
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/config)
configure_file(
    ${CMAKE_SOURCE_DIR}/config/piano_config.json.in
    ${CMAKE_BINARY_DIR}/config/piano_config.json
    @ONLY
)

# Install targets
install(TARGETS piano_synth demo_tune test_piano_sound DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/config/piano_config.json DESTINATION share/piano-synth/config)

# Testing
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    # Find Google Test
    find_package(GTest)
    if(NOT GTest_FOUND)
        # Try to find gtest manually
        find_path(GTEST_INCLUDE_DIR gtest/gtest.h)
        find_library(GTEST_LIBRARY gtest)
        find_library(GTEST_MAIN_LIBRARY gtest_main)

        if(GTEST_INCLUDE_DIR AND GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
            set(GTEST_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})
            set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIR})
            message(STATUS "Found Google Test manually")
        else()
            message(WARNING "Google Test not found. Tests will not be built.")
            set(BUILD_TESTS OFF)
        endif()
    else()
        set(GTEST_LIBRARIES GTest::gtest GTest::gtest_main)
        set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS})
        message(STATUS "Found Google Test via find_package")
    endif()
endif()

if(BUILD_TESTS)
    enable_testing()

    # Include test directories
    include_directories(${GTEST_INCLUDE_DIRS})

    # Unit tests executable
    add_executable(piano_synth_tests
        tests/test_main.cpp
        tests/test_math_utils.cpp
        tests/test_config_manager.cpp
        tests/test_string_model.cpp
        tests/test_helpers.cpp
    )

    target_link_libraries(piano_synth_tests
        piano_synth_lib
        piano_utils
        piano_physics
        piano_abstraction
        piano_synthesis
        piano_audio
        ${GTEST_LIBRARIES}
        Threads::Threads
    )

    # Integration tests executable
    add_executable(piano_synth_integration_tests
        tests/test_main.cpp
        tests/test_integration.cpp
    )

    target_link_libraries(piano_synth_integration_tests
        piano_synth_lib
        ${GTEST_LIBRARIES}
        Threads::Threads
    )

    # Add test runners
    add_test(NAME UnitTests COMMAND piano_synth_tests)
    add_test(NAME IntegrationTests COMMAND piano_synth_integration_tests)

    # Custom test target that runs all tests
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4
        DEPENDS piano_synth_tests piano_synth_integration_tests
        COMMENT "Running all tests"
    )

    # Test coverage (if available)
    find_program(GCOV_PROGRAM gcov)
    find_program(LCOV_PROGRAM lcov)
    find_program(GENHTML_PROGRAM genhtml)

    if(GCOV_PROGRAM AND LCOV_PROGRAM AND GENHTML_PROGRAM AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage")

        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E remove_directory coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory coverage
            COMMAND ${LCOV_PROGRAM} --directory . --zerocounters
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LCOV_PROGRAM} --directory . --capture --output-file coverage/coverage.info
            COMMAND ${LCOV_PROGRAM} --remove coverage/coverage.info '/usr/*' --output-file coverage/coverage.info
            COMMAND ${LCOV_PROGRAM} --remove coverage/coverage.info '*/tests/*' --output-file coverage/coverage.info
            COMMAND ${GENHTML_PROGRAM} -o coverage coverage/coverage.info
            DEPENDS piano_synth_tests piano_synth_integration_tests
            COMMENT "Generating test coverage report"
        )
    endif()

    message(STATUS "Tests enabled. Use 'make run_tests' to run all tests.")
else()
    message(STATUS "Tests disabled.")
endif()

# Tools (optional)
option(BUILD_TOOLS "Build analysis tools" OFF)

if(BUILD_TOOLS)
    add_subdirectory(tools)
    message(STATUS "Tools enabled.")
endif()

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "PianoSynth")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Physical Modeling Piano Synthesizer")
set(CPACK_PACKAGE_VENDOR "PianoSynth Project")
set(CPACK_PACKAGE_CONTACT "dev@pianosynth.org")

set(CPACK_GENERATOR "DEB;RPM;TGZ")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libportaudio2, libmp3lame0, librtmidi6")
set(CPACK_RPM_PACKAGE_REQUIRES "portaudio, lame-libs, rtmidi")

include(CPack)

# Print configuration summary
message(STATUS "==============================================")
message(STATUS "Configuration Summary:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "  Tools: ${BUILD_TOOLS}")
message(STATUS "  Documentation: ${DOXYGEN_FOUND}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  PortAudio: ${PORTAUDIO_LIBRARY}")
message(STATUS "  LAME: ${LAME_LIBRARY}")
message(STATUS "  RtMidi: ${RTMIDI_LIBRARY}")
if(BUILD_TESTS)
    message(STATUS "  Google Test: Found")
endif()
message(STATUS "==============================================")
