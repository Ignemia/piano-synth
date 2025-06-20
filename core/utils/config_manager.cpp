#include "config_manager.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>

namespace PianoSynth {
namespace Utils {

ConfigManager::ConfigManager() {
    loadDefaults();
}

ConfigManager::~ConfigManager() = default;

bool ConfigManager::loadConfig(const std::string& config_file) {
    current_config_file_ = config_file;
    try {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            Logger logger;
            logger.warning("Could not load config file: " + config_file + ", using defaults");
            loadDefaults();
            return false;
        }
        file >> config_json_;
        file.close();
        return true;
    } catch (...) {
        Logger logger;
        logger.warning("Could not parse config file: " + config_file + ", using defaults");
        loadDefaults();
        return false;
    }
}

bool ConfigManager::saveConfig(const std::string& config_file) {
    try {
        std::ofstream file(config_file);
        if (!file.is_open()) {
            return false;
        }
        file << config_json_.dump(2);
        file.close();
        current_config_file_ = config_file;
        return true;
    } catch (...) {
        return false;
    }
}

static nlohmann::json* get_nested_json(nlohmann::json& j, const std::string& key, bool create = false) {
    size_t dot = key.find('.');
    if (dot == std::string::npos) return &j;
    std::string prefix = key.substr(0, dot);
    std::string rest = key.substr(dot + 1);
    if (!j.contains(prefix)) {
        if (create) j[prefix] = nlohmann::json::object();
        else return nullptr;
    }
    return get_nested_json(j[prefix], rest, create);
}

static const nlohmann::json* get_nested_json_const(const nlohmann::json& j, const std::string& key) {
    size_t dot = key.find('.');
    if (dot == std::string::npos) return &j;
    std::string prefix = key.substr(0, dot);
    std::string rest = key.substr(dot + 1);
    if (!j.contains(prefix)) return nullptr;
    return get_nested_json_const(j.at(prefix), rest);
}

static std::string get_last_key(const std::string& key) {
    size_t dot = key.find_last_of('.');
    if (dot == std::string::npos) return key;
    return key.substr(dot + 1);
}

int ConfigManager::getInt(const std::string& key, int default_value) {
    const nlohmann::json* j = get_nested_json_const(config_json_, key);
    std::string last = get_last_key(key);
    if (j && j->contains(last)) {
        try {
            return (*j)[last].get<int>();
        } catch (...) {}
    }
    return default_value;
}

float ConfigManager::getFloat(const std::string& key, float default_value) {
    const nlohmann::json* j = get_nested_json_const(config_json_, key);
    std::string last = get_last_key(key);
    if (j && j->contains(last)) {
        try {
            return (*j)[last].get<float>();
        } catch (...) {}
    }
    return default_value;
}

double ConfigManager::getDouble(const std::string& key, double default_value) {
    const nlohmann::json* j = get_nested_json_const(config_json_, key);
    std::string last = get_last_key(key);
    if (j && j->contains(last)) {
        try {
            return (*j)[last].get<double>();
        } catch (...) {}
    }
    return default_value;
}

/**
 * Fetch a boolean value from the configuration. String and numeric
 * representations are accepted. [AI GENERATED]
 */
bool ConfigManager::getBool(const std::string& key, bool default_value) {
    const nlohmann::json* j = get_nested_json_const(config_json_, key);
    std::string last = get_last_key(key);
    if (j && j->contains(last)) {
        try {
            if ((*j)[last].is_boolean()) {
                return (*j)[last].get<bool>();
            }
            if ((*j)[last].is_string()) {
                return stringToBool((*j)[last].get<std::string>());
            }
            if ((*j)[last].is_number()) {
                return (*j)[last].get<int>() != 0;
            }
        } catch (...) {}
    }
    return default_value;
}

std::string ConfigManager::getString(const std::string& key, const std::string& default_value) {
    const nlohmann::json* j = get_nested_json_const(config_json_, key);
    std::string last = get_last_key(key);
    if (j && j->contains(last)) {
        try {
            return (*j)[last].get<std::string>();
        } catch (...) {}
    }
    return default_value;
}

void ConfigManager::setInt(const std::string& key, int value) {
    nlohmann::json* j = get_nested_json(config_json_, key, true);
    std::string last = get_last_key(key);
    (*j)[last] = value;
}

void ConfigManager::setFloat(const std::string& key, float value) {
    nlohmann::json* j = get_nested_json(config_json_, key, true);
    std::string last = get_last_key(key);
    (*j)[last] = value;
}

void ConfigManager::setDouble(const std::string& key, double value) {
    nlohmann::json* j = get_nested_json(config_json_, key, true);
    std::string last = get_last_key(key);
    (*j)[last] = value;
}

void ConfigManager::setBool(const std::string& key, bool value) {
    nlohmann::json* j = get_nested_json(config_json_, key, true);
    std::string last = get_last_key(key);
    (*j)[last] = value;
}

void ConfigManager::setString(const std::string& key, const std::string& value) {
    nlohmann::json* j = get_nested_json(config_json_, key, true);
    std::string last = get_last_key(key);
    (*j)[last] = value;
}

bool ConfigManager::hasKey(const std::string& key) const {
    const nlohmann::json* j = get_nested_json_const(config_json_, key);
    std::string last = get_last_key(key);
    return j && j->contains(last);
}

void ConfigManager::loadDefaults() {
    config_json_ = nlohmann::json::object();

    // Audio settings
    setDouble("audio.sample_rate", 44100.0);
    setInt("audio.buffer_size", 512);
    setInt("audio.channels", 2);
    setString("audio.output_device", "default");

    // String physics defaults
    setDouble("string.tension_base", 1000.0);
    setDouble("string.damping", 0.001);
    setDouble("string.stiffness", 1e-5);
    setDouble("string.density", 7850.0);
    setInt("string.discretization_points", 100);

    // Hammer physics defaults
    setDouble("hammer.mass", 0.01);
    setDouble("hammer.stiffness", 1e6);
    setDouble("hammer.damping", 100.0);
    setDouble("hammer.contact_time", 0.001);

    // Soundboard defaults
    setDouble("soundboard.area", 0.5);
    setDouble("soundboard.thickness", 0.01);
    setDouble("soundboard.density", 400.0);
    setDouble("soundboard.damping", 0.01);

    // Resonance defaults
    setInt("resonance.max_harmonics", 32);
    setDouble("resonance.harmonic_decay", 0.8);
    setDouble("resonance.sympathetic_resonance", 0.1);

    // Synthesis defaults
    setInt("synthesis.max_voices", 128);
    setDouble("synthesis.note_off_fade_time", 0.1);
    setDouble("synthesis.velocity_sensitivity", 0.01);
    setFloat("synthesis.master_volume", 0.8f);

    // MIDI defaults
    setString("midi.device_name", "");
    setBool("midi.auto_detect", true);
    setFloat("midi.velocity_curve", 1.0f);
    setFloat("midi.hammer_response_curve", 1.0f);

    // Recording defaults
    setInt("recording.mp3_bitrate", 192);
    setInt("recording.mp3_quality", 5);
    setString("recording.output_directory", "recordings/");

    // Room acoustics defaults
    setDouble("room.size", 10.0);
    setDouble("room.reverb_time", 1.5);
    setDouble("room.damping", 0.3);
    setBool("room.early_reflections", true);
}

bool ConfigManager::stringToBool(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return (lower == "true" || lower == "1" || lower == "yes" || lower == "on");
}

std::string ConfigManager::boolToString(bool value) {
    return value ? "true" : "false";
}

} // namespace Utils
} // namespace PianoSynth