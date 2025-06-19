#include <iostream>
#include <string>

class ConfigManager {
public:
    void initialize() {
        std::cout << "Config manager initialized" << std::endl;
    }
    
    void shutdown() {
        std::cout << "Config manager shutdown" << std::endl;
    }
    
    bool loadConfig(const std::string& path) {
        std::cout << "Loading config from: " << path << std::endl;
        return true;
    }
    
    void saveConfig(const std::string& path) {
        std::cout << "Saving config to: " << path << std::endl;
    }
};
