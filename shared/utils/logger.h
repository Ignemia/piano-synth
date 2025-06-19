#pragma once
#include <memory>
#include <string>

namespace PianoSynth {
namespace Utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& getInstance();
    
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
private:
    Logger() = default;
    static std::unique_ptr<Logger> instance_;
};

} // namespace Utils
} // namespace PianoSynth
