#pragma once

#include "../core/abstraction/note_event.h"
#include "../core/utils/logger.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <chrono>
#include <memory>

namespace PianoSynth {
namespace Testing {

/**
 * Test configuration structure
 */
struct TestConfig {
    double sample_rate;
    int buffer_size;
    int channels;
    double test_duration;
    double tolerance;
    double audio_tolerance;
    double frequency_tolerance;
    
    TestConfig();
};

/**
 * Audio analysis results
 */
struct AudioAnalysis {
    float rms = 0.0f;
    float peak = 0.0f;
    double frequency = 0.0;
    bool is_silent = true;
    bool has_clipping = false;
    float dynamic_range = 0.0f;
    int zero_crossings = 0;
};

/**
 * Test helper functions
 */
class TestHelpers {
public:
    // Configuration helpers
    static bool createTestConfigFile(const std::string& filename, const TestConfig& config = TestConfig());
    
    // MIDI event creation
    static std::vector<unsigned char> createMidiNoteOn(int channel, int note, int velocity);
    static std::vector<unsigned char> createMidiNoteOff(int channel, int note, int velocity);
    static std::vector<unsigned char> createMidiControlChange(int channel, int controller, int value);
    static std::vector<unsigned char> createMidiPitchBend(int channel, int value);
    
    // Note event creation
    static Abstraction::NoteEvent createTestNoteEvent(
        Abstraction::NoteEvent::Type type, 
        int note, 
        float velocity = 0.5f
    );
    
    // Audio analysis
    static bool isAudioSilent(const std::vector<float>& audio_buffer, float threshold = 1e-6f);
    static float calculateRMS(const std::vector<float>& audio_buffer);
    static float calculatePeak(const std::vector<float>& audio_buffer);
    static double estimateFrequency(const std::vector<float>& audio_buffer, double sample_rate);
    static AudioAnalysis analyzeAudio(const std::vector<float>& audio_buffer, double sample_rate);
    
    // Audio generation
    static std::vector<float> generateSineWave(double frequency, double amplitude, double duration, double sample_rate);
    static std::vector<float> generateWhiteNoise(double amplitude, double duration, double sample_rate);
    static std::vector<float> generateImpulse(double amplitude, double duration, double sample_rate);
    
    // Audio comparison
    static bool compareAudioBuffers(const std::vector<float>& buffer1, const std::vector<float>& buffer2, float tolerance);
    static bool hasValidAudioRange(const std::vector<float>& audio_buffer);
    
    // File I/O
    static void saveAudioToFile(const std::vector<float>& audio_buffer, const std::string& filename, double sample_rate);
    
private:
    TestHelpers() = delete; // Static class
};

/**
 * Performance timing utility for tests
 */
class PerformanceTimer {
public:
    PerformanceTimer();
    void reset();
    double elapsed() const; // Returns elapsed time in seconds
    
private:
    std::chrono::high_resolution_clock::time_point start_time_;
};

/**
 * Mock logger for testing
 */
class MockLogger {
public:
    struct LogEntry {
        Utils::LogLevel level;
        std::string message;
        std::chrono::high_resolution_clock::time_point timestamp;
    };
    
    MockLogger();
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    void setMinLevel(Utils::LogLevel level) { log_level_ = level; }
    void setLogToConsole(bool enable) { log_to_console_ = enable; }
    
    const std::vector<LogEntry>& getLogEntries() const;
    void clearLog();
    bool hasLogEntry(Utils::LogLevel level, const std::string& message_substring) const;
    size_t getLogCount(Utils::LogLevel level) const;
    
private:
    Utils::LogLevel log_level_;
    bool log_to_console_;
    std::vector<LogEntry> log_entries_;
    
    void log(Utils::LogLevel level, const std::string& message);
};

/**
 * Test fixture base class with common setup
 */
class PianoSynthTestBase : public ::testing::Test {
protected:
    void SetUp() override {
        test_config_ = TestConfig();
        timer_.reset();
    }
    
    void TearDown() override {
        // Clean up any test files
        cleanupTestFiles();
    }
    
    virtual void cleanupTestFiles() {
        // Override in derived classes to clean up specific files
    }
    
    TestConfig test_config_;
    PerformanceTimer timer_;
    MockLogger mock_logger_;
};

// Useful macros for audio testing
#define EXPECT_AUDIO_SILENT(buffer) \
    EXPECT_TRUE(TestHelpers::isAudioSilent(buffer)) << "Audio buffer should be silent"

#define EXPECT_AUDIO_NOT_SILENT(buffer) \
    EXPECT_FALSE(TestHelpers::isAudioSilent(buffer)) << "Audio buffer should not be silent"

#define EXPECT_AUDIO_VALID_RANGE(buffer) \
    EXPECT_TRUE(TestHelpers::hasValidAudioRange(buffer)) << "Audio samples should be in valid range [-1.0, 1.0]"

#define EXPECT_FREQUENCY_NEAR(buffer, expected_freq, sample_rate, tolerance) \
    do { \
        double actual_freq = TestHelpers::estimateFrequency(buffer, sample_rate); \
        EXPECT_NEAR(actual_freq, expected_freq, expected_freq * tolerance) \
            << "Frequency should be approximately " << expected_freq << " Hz, got " << actual_freq << " Hz"; \
    } while(0)

#define EXPECT_RMS_GREATER_THAN(buffer, threshold) \
    do { \
        float rms = TestHelpers::calculateRMS(buffer); \
        EXPECT_GT(rms, threshold) << "RMS level should be greater than " << threshold << ", got " << rms; \
    } while(0)

#define EXPECT_PERFORMANCE_WITHIN(timer, max_seconds) \
    do { \
        double elapsed = timer.elapsed(); \
        EXPECT_LT(elapsed, max_seconds) \
            << "Operation took " << elapsed << " seconds, should be less than " << max_seconds; \
    } while(0)

} // namespace Testing
} // namespace PianoSynth