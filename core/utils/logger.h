#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <mutex>

namespace PianoSynth {
namespace Utils {

// Enumeration representing log levels. Names are prefixed with 'k'
// to avoid clashes with system macros such as DEBUG. [AI GENERATED]
enum class LogLevel {
    kDebug = 0,
    kInfo = 1,
    kWarning = 2,
    kError = 3
};

/**
 * Simple logging utility for debugging and monitoring
 */
class Logger {
public:
    Logger(LogLevel min_level = LogLevel::kInfo);
    ~Logger();

    // Logging methods
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    // Configuration
    void setMinLevel(LogLevel level) { min_level_ = level; }
    void setLogToFile(bool enable, const std::string& filename = "piano_synth.log");
    void setLogToConsole(bool enable) { log_to_console_ = enable; }
    
private:
    LogLevel min_level_;
    bool log_to_console_;
    bool log_to_file_;
    std::ofstream log_file_;
    std::mutex log_mutex_;
    
    void log(LogLevel level, const std::string& message);
    std::string levelToString(LogLevel level);
    std::string getCurrentTimestamp();
};

} // namespace Utils
} // namespace PianoSynth
