#include <iostream>
#include <string>

class DllManager {
public:
    void initialize() {
        std::cout << "DLL manager initialized" << std::endl;
    }
    
    void shutdown() {
        std::cout << "DLL manager shutdown" << std::endl;
    }
    
    bool loadDll(const std::string& path) {
        std::cout << "Loading DLL: " << path << std::endl;
        return true;
    }
    
    void unloadDll(const std::string& path) {
        std::cout << "Unloading DLL: " << path << std::endl;
    }
};
