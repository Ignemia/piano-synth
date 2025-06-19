#include "json_config.h"
#include <fstream>
#include <sstream>

namespace PianoSynth {
namespace Utils {

// Simple JSON parser implementation (basic functionality)
class JsonConfig::Impl {
public:
    bool valid = false;
    std::string error_message;
    std::string json_content;
    
    // Very basic JSON parsing - in real implementation, use nlohmann/json or similar
    bool parse(const std::string& content) {
        json_content = content;
        valid = !content.empty(); // Simplified validation
        return valid;
    }
    
    // Simplified value extraction - real implementation would parse JSON properly
    std::string extract_value(const std::string& path) const {
        // This is a very basic implementation for testing
        // Real implementation should use proper JSON parsing
        size_t pos = json_content.find("\"" + path + "\"");
        if (pos != std::string::npos) {
            pos = json_content.find(":", pos);
            if (pos != std::string::npos) {
                pos++;
                while (pos < json_content.length() && std::isspace(json_content[pos])) pos++;
                
                if (pos < json_content.length()) {
                    size_t end_pos = pos;
                    if (json_content[pos] == '"') {
                        // String value
                        pos++;
                        end_pos = json_content.find('"', pos);
                        if (end_pos != std::string::npos) {
                            return json_content.substr(pos, end_pos - pos);
                        }
                    } else {
                        // Numeric or boolean value
                        while (end_pos < json_content.length() && 
                               json_content[end_pos] != ',' && 
                               json_content[end_pos] != '}' && 
                               json_content[end_pos] != '\n') {
                            end_pos++;
                        }
                        std::string value = json_content.substr(pos, end_pos - pos);
                        // Trim whitespace
                        size_t start = 0;
                        while (start < value.length() && std::isspace(value[start])) start++;
                        size_t end = value.length();
                        while (end > start && std::isspace(value[end-1])) end--;
                        return value.substr(start, end - start);
                    }
                }
            }
        }
        return "";
    }
};

JsonConfig::JsonConfig() : pimpl_(new Impl()) {}

JsonConfig::~JsonConfig() {
    delete pimpl_;
}

bool JsonConfig::load_from_string(const std::string& json_string) {
    return pimpl_->parse(json_string);
}

bool JsonConfig::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        pimpl_->error_message = "Could not open file: " + filename;
        pimpl_->valid = false;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return load_from_string(buffer.str());
}

bool JsonConfig::get_bool(const std::string& path, bool default_value) const {
    std::string value = pimpl_->extract_value(path);
    if (value.empty()) return default_value;
    return value == "true" || value == "1";
}

int JsonConfig::get_int(const std::string& path, int default_value) const {
    std::string value = pimpl_->extract_value(path);
    if (value.empty()) return default_value;
    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

float JsonConfig::get_float(const std::string& path, float default_value) const {
    std::string value = pimpl_->extract_value(path);
    if (value.empty()) return default_value;
    try {
        return std::stof(value);
    } catch (...) {
        return default_value;
    }
}

double JsonConfig::get_double(const std::string& path, double default_value) const {
    std::string value = pimpl_->extract_value(path);
    if (value.empty()) return default_value;
    try {
        return std::stod(value);
    } catch (...) {
        return default_value;
    }
}

std::string JsonConfig::get_string(const std::string& path, const std::string& default_value) const {
    std::string value = pimpl_->extract_value(path);
    return value.empty() ? default_value : value;
}

bool JsonConfig::has_value(const std::string& path) const {
    return !pimpl_->extract_value(path).empty();
}

bool JsonConfig::get_section(const std::string& path, JsonConfig& section) const {
    // Simplified implementation
    section.pimpl_->json_content = pimpl_->json_content;
    section.pimpl_->valid = pimpl_->valid;
    return true;
}

bool JsonConfig::is_valid() const {
    return pimpl_->valid;
}

std::string JsonConfig::get_error() const {
    return pimpl_->error_message;
}

} // namespace Utils
} // namespace PianoSynth
