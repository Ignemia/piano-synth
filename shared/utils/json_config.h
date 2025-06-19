#pragma once

#include <string>

namespace PianoSynth {
namespace Utils {

class JsonConfig {
public:
    JsonConfig();
    ~JsonConfig();
    
    // Load configuration from JSON string or file
    bool load_from_string(const std::string& json_string);
    bool load_from_file(const std::string& filename);
    
    // Get configuration values
    bool get_bool(const std::string& path, bool default_value = false) const;
    int get_int(const std::string& path, int default_value = 0) const;
    float get_float(const std::string& path, float default_value = 0.0f) const;
    double get_double(const std::string& path, double default_value = 0.0) const;
    std::string get_string(const std::string& path, const std::string& default_value = "") const;
    
    // Check if path exists
    bool has_value(const std::string& path) const;
    
    // Get sections
    bool get_section(const std::string& path, JsonConfig& section) const;
    
    // Validation
    bool is_valid() const;
    std::string get_error() const;

private:
    class Impl;
    Impl* pimpl_;
};

} // namespace Utils
} // namespace PianoSynth
