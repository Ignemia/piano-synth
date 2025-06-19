#pragma once

#include <string>
#include <memory>
#include "../../third_party/json.hpp"

namespace PianoSynth {
namespace Utils {

/**
 * Configuration manager for loading and managing piano synthesizer settings
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // Configuration loading
    bool loadConfig(const std::string& config_file);
    bool saveConfig(const std::string& config_file);
    
    // Value access (with defaults)
    int getInt(const std::string& key, int default_value = 0);
    float getFloat(const std::string& key, float default_value = 0.0f);
    double getDouble(const std::string& key, double default_value = 0.0);
    bool getBool(const std::string& key, bool default_value = false);
    std::string getString(const std::string& key, const std::string& default_value = "");
    
    // Value setting
    void setInt(const std::string& key, int value);
    void setFloat(const std::string& key, float value);
    void setDouble(const std::string& key, double value);
    void setBool(const std::string& key, bool value);
    void setString(const std::string& key, const std::string& value);
    
    // Existence check
    bool hasKey(const std::string& key) const;
    
    // Default configuration
    void loadDefaults();
    
private:
    nlohmann::json config_json_;
    std::string current_config_file_;

    // Utility functions
    bool stringToBool(const std::string& str);
    std::string boolToString(bool value);
};

} // namespace Utils
} // namespace PianoSynth
