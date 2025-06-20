#include "logger.h"
#include <iomanip>
#include <sstream>

namespace PianoSynth {
namespace Utils {

Logger::Logger(LogLevel min_level) 
    : min_level_(min_level), log_to_console_(true), log_to_file_(false) {
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
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

void Logger::setLogToFile(bool enable, const std::string& filename) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    log_to_file_ = enable;
    
    if (enable) {
        log_file_.open(filename, std::ios::app);
        if (!log_file_.is_open()) {
            log_to_file_ = false;
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < min_level_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    std::string timestamp = getCurrentTimestamp();
    std::string level_str = levelToString(level);
    std::string formatted_message = "[" + timestamp + "] [" + level_str + "] " + message;
    
    if (log_to_console_) {
        if (level >= LogLevel::kError) {
            std::cerr << formatted_message << std::endl;
        } else {
            std::cout << formatted_message << std::endl;
        }
    }
    
    if (log_to_file_ && log_file_.is_open()) {
        log_file_ << formatted_message << std::endl;
        log_file_.flush();
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::kDebug:   return "DEBUG";
        case LogLevel::kInfo:    return "INFO";
        case LogLevel::kWarning: return "WARN";
        case LogLevel::kError:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

} // namespace Utils
} // namespace PianoSynth
