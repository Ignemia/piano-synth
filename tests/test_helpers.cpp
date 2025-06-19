#include "test_helpers.h"
#include "../core/utils/math_utils.h"
#include <fstream>
#include <random>
#include <algorithm>

namespace PianoSynth {
namespace Testing {

TestConfig::TestConfig() {
    // Default test configuration
    sample_rate = 44100.0;
    buffer_size = 512;
    channels = 2;
    test_duration = 1.0; // 1 second
    tolerance = 1e-6;
    audio_tolerance = 1e-3;
    frequency_tolerance = 0.05; // 5%
}

bool TestHelpers::createTestConfigFile(const std::string& filename, const TestConfig& config) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{\n";
    file << "  \"audio\": {\n";
    file << "    \"sample_rate\": " << config.sample_rate << ",\n";
    file << "    \"buffer_size\": " << config.buffer_size << ",\n";
    file << "    \"channels\": " << config.channels << ",\n";
    file << "    \"output_device\": \"test_device\"\n";
    file << "  },\n";
    file << "  \"synthesis\": {\n";
    file << "    \"max_voices\": 32,\n";
    file << "    \"master_volume\": 0.8,\n";
    file << "    \"velocity_sensitivity\": 0.01\n";
    file << "  },\n";
    file << "  \"string\": {\n";
    file << "    \"tension_base\": 1000.0,\n";
    file << "    \"damping\": 0.001,\n";
    file << "    \"stiffness\": 1e-5\n";
    file << "  },\n";
    file << "  \"midi\": {\n";
    file << "    \"auto_detect\": false,\n";
    file << "    \"velocity_curve\": 1.0\n";
    file << "  }\n";
    file << "}\n";
    
    file.close();
    return true;
}

std::vector<unsigned char> TestHelpers::createMidiNoteOn(int channel, int note, int velocity) {
    return {
        static_cast<unsigned char>(0x90 | (channel & 0x0F)),
        static_cast<unsigned char>(note & 0x7F),
        static_cast<unsigned char>(velocity & 0x7F)
    };
}

std::vector<unsigned char> TestHelpers::createMidiNoteOff(int channel, int note, int velocity) {
    return {
        static_cast<unsigned char>(0x80 | (channel & 0x0F)),
        static_cast<unsigned char>(note & 0x7F),
        static_cast<unsigned char>(velocity & 0x7F)
    };
}

std::vector<unsigned char> TestHelpers::createMidiControlChange(int channel, int controller, int value) {
    return {
        static_cast<unsigned char>(0xB0 | (channel & 0x0F)),
        static_cast<unsigned char>(controller & 0x7F),
        static_cast<unsigned char>(value & 0x7F)
    };
}

std::vector<unsigned char> TestHelpers::createMidiPitchBend(int channel, int value) {
    int bend_value = value + 8192; // Convert from signed to 14-bit unsigned
    bend_value = std::clamp(bend_value, 0, 16383);
    
    return {
        static_cast<unsigned char>(0xE0 | (channel & 0x0F)),
        static_cast<unsigned char>(bend_value & 0x7F),        // LSB
        static_cast<unsigned char>((bend_value >> 7) & 0x7F)  // MSB
    };
}

Abstraction::NoteEvent TestHelpers::createTestNoteEvent(Abstraction::NoteEvent::Type type, int note, float velocity) {
    Abstraction::NoteEvent event;
    event.type = type;
    event.note_number = note;
    event.velocity = velocity;
    event.release_velocity = velocity * 0.8f;
    event.hammer_velocity = velocity * 3.0f;
    event.string_excitation = velocity * velocity * 2.0f;
    event.damper_position = (type == Abstraction::NoteEvent::NOTE_ON) ? 1.0f : 0.0f;
    event.sustain_pedal = false;
    event.soft_pedal = false;
    event.sostenuto_pedal = false;
    event.pitch_bend = 0.0f;
    event.aftertouch = 0.0f;
    event.press_time = std::chrono::high_resolution_clock::now();
    event.duration_ms = 0.0;
    
    return event;
}

bool TestHelpers::isAudioSilent(const std::vector<float>& audio_buffer, float threshold) {
    for (float sample : audio_buffer) {
        if (std::abs(sample) > threshold) {
            return false;
        }
    }
    return true;
}

float TestHelpers::calculateRMS(const std::vector<float>& audio_buffer) {
    if (audio_buffer.empty()) {
        return 0.0f;
    }
    
    float sum_squares = 0.0f;
    for (float sample : audio_buffer) {
        sum_squares += sample * sample;
    }
    
    return std::sqrt(sum_squares / audio_buffer.size());
}

float TestHelpers::calculatePeak(const std::vector<float>& audio_buffer) {
    float peak = 0.0f;
    for (float sample : audio_buffer) {
        peak = std::max(peak, std::abs(sample));
    }
    return peak;
}

double TestHelpers::estimateFrequency(const std::vector<float>& audio_buffer, double sample_rate) {
    if (audio_buffer.size() < 2) {
        return 0.0;
    }
    
    // Simple zero-crossing frequency estimation
    int zero_crossings = 0;
    for (size_t i = 1; i < audio_buffer.size(); ++i) {
        if ((audio_buffer[i-1] >= 0 && audio_buffer[i] < 0) || 
            (audio_buffer[i-1] < 0 && audio_buffer[i] >= 0)) {
            zero_crossings++;
        }
    }
    
    double duration = audio_buffer.size() / sample_rate;
    return (zero_crossings / 2.0) / duration;
}

std::vector<float> TestHelpers::generateSineWave(double frequency, double amplitude, double duration, double sample_rate) {
    int num_samples = static_cast<int>(duration * sample_rate);
    std::vector<float> samples(num_samples);
    
    for (int i = 0; i < num_samples; ++i) {
        double t = i / sample_rate;
        samples[i] = static_cast<float>(amplitude * std::sin(2.0 * Utils::MathUtils::PI * frequency * t));
    }
    
    return samples;
}

std::vector<float> TestHelpers::generateWhiteNoise(double amplitude, double duration, double sample_rate) {
    int num_samples = static_cast<int>(duration * sample_rate);
    std::vector<float> samples(num_samples);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    
    for (int i = 0; i < num_samples; ++i) {
        samples[i] = static_cast<float>(amplitude * dist(gen));
    }
    
    return samples;
}

std::vector<float> TestHelpers::generateImpulse(double amplitude, double duration, double sample_rate) {
    int num_samples = static_cast<int>(duration * sample_rate);
    std::vector<float> samples(num_samples, 0.0f);
    
    if (num_samples > 0) {
        samples[0] = static_cast<float>(amplitude);
    }
    
    return samples;
}

bool TestHelpers::compareAudioBuffers(const std::vector<float>& buffer1, const std::vector<float>& buffer2, float tolerance) {
    if (buffer1.size() != buffer2.size()) {
        return false;
    }
    
    for (size_t i = 0; i < buffer1.size(); ++i) {
        if (std::abs(buffer1[i] - buffer2[i]) > tolerance) {
            return false;
        }
    }
    
    return true;
}

bool TestHelpers::hasValidAudioRange(const std::vector<float>& audio_buffer) {
    for (float sample : audio_buffer) {
        if (std::isnan(sample) || std::isinf(sample)) {
            return false;
        }
        if (sample < -1.0f || sample > 1.0f) {
            return false;
        }
    }
    return true;
}

void TestHelpers::saveAudioToFile(const std::vector<float>& audio_buffer, const std::string& filename, double sample_rate) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }
    
    // Simple text format for debugging
    file << "# Sample Rate: " << sample_rate << "\n";
    file << "# Samples: " << audio_buffer.size() << "\n";
    file << "# Duration: " << (audio_buffer.size() / sample_rate) << " seconds\n";
    file << "# Format: sample_index sample_value\n";
    
    for (size_t i = 0; i < audio_buffer.size(); ++i) {
        file << i << " " << audio_buffer[i] << "\n";
    }
    
    file.close();
}

AudioAnalysis TestHelpers::analyzeAudio(const std::vector<float>& audio_buffer, double sample_rate) {
    AudioAnalysis analysis;
    
    if (audio_buffer.empty()) {
        return analysis;
    }
    
    analysis.rms = calculateRMS(audio_buffer);
    analysis.peak = calculatePeak(audio_buffer);
    analysis.frequency = estimateFrequency(audio_buffer, sample_rate);
    analysis.is_silent = isAudioSilent(audio_buffer, 1e-6f);
    analysis.has_clipping = (analysis.peak >= 0.99f);
    analysis.dynamic_range = 20.0f * std::log10(analysis.peak / (analysis.rms + 1e-10f));
    
    // Count zero crossings
    analysis.zero_crossings = 0;
    for (size_t i = 1; i < audio_buffer.size(); ++i) {
        if ((audio_buffer[i-1] >= 0 && audio_buffer[i] < 0) || 
            (audio_buffer[i-1] < 0 && audio_buffer[i] >= 0)) {
            analysis.zero_crossings++;
        }
    }
    
    return analysis;
}

PerformanceTimer::PerformanceTimer() : start_time_(std::chrono::high_resolution_clock::now()) {
}

void PerformanceTimer::reset() {
    start_time_ = std::chrono::high_resolution_clock::now();
}

double PerformanceTimer::elapsed() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time_);
    return duration.count() / 1000000.0; // Convert to seconds
}

MockLogger::MockLogger() : log_level_(Utils::LogLevel::DEBUG), log_to_console_(false) {
}

void MockLogger::debug(const std::string& message) {
    log(Utils::LogLevel::DEBUG, message);
}

void MockLogger::info(const std::string& message) {
    log(Utils::LogLevel::INFO, message);
}

void MockLogger::warning(const std::string& message) {
    log(Utils::LogLevel::WARNING, message);
}

void MockLogger::error(const std::string& message) {
    log(Utils::LogLevel::ERROR, message);
}

void MockLogger::log(Utils::LogLevel level, const std::string& message) {
    if (level >= log_level_) {
        LogEntry entry;
        entry.level = level;
        entry.message = message;
        entry.timestamp = std::chrono::high_resolution_clock::now();
        
        log_entries_.push_back(entry);
        
        if (log_to_console_) {
            std::string level_str;
            switch (level) {
                case Utils::LogLevel::DEBUG: level_str = "DEBUG"; break;
                case Utils::LogLevel::INFO: level_str = "INFO"; break;
                case Utils::LogLevel::WARNING: level_str = "WARN"; break;
                case Utils::LogLevel::ERROR: level_str = "ERROR"; break;
            }
            std::cout << "[" << level_str << "] " << message << std::endl;
        }
    }
}

const std::vector<MockLogger::LogEntry>& MockLogger::getLogEntries() const {
    return log_entries_;
}

void MockLogger::clearLog() {
    log_entries_.clear();
}

bool MockLogger::hasLogEntry(Utils::LogLevel level, const std::string& message_substring) const {
    for (const auto& entry : log_entries_) {
        if (entry.level == level && entry.message.find(message_substring) != std::string::npos) {
            return true;
        }
    }
    return false;
}

size_t MockLogger::getLogCount(Utils::LogLevel level) const {
    size_t count = 0;
    for (const auto& entry : log_entries_) {
        if (entry.level == level) {
            count++;
        }
    }
    return count;
}

} // namespace Testing
} // namespace PianoSynth