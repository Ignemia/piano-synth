#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace PianoSynth {
namespace Utils {

std::unique_ptr<Logger> Logger::instance_ = nullptr;

Logger& Logger::getInstance() {
    if (!instance_) {
        instance_.reset(new Logger());
    }
    return *instance_;
}

void Logger::log(LogLevel level, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    std::string level_str;
    switch (level) {
        case LogLevel::kDebug: level_str = "DEBUG"; break;
        case LogLevel::kInfo: level_str = "INFO"; break;
        case LogLevel::kWarning: level_str = "WARNING"; break;
        case LogLevel::kError: level_str = "ERROR"; break;
    }
    
    std::cout << "[" << ss.str() << "] [" << level_str << "] " << message << std::endl;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::kDebug, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::kInfo, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::kWarning, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::kError, message);
}

} // namespace Utils
} // namespace PianoSynth
