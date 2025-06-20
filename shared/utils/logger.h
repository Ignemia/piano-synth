#pragma once
#include <memory>
#include <string>

namespace PianoSynth {
namespace Utils {

// Log levels for the lightweight logger implementation. Prefixed with 'k'
// to avoid macro conflicts such as DEBUG on some compilers. [AI GENERATED]
enum class LogLevel {
    kDebug,
    kInfo,
    kWarning,
    kError
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
