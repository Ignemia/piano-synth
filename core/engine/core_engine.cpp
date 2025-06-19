#include "../shared/interfaces/dll_interfaces.h"
#include "../shared/utils/json_config.h"
#include <iostream>
#include <thread>
#include <chrono>

class CoreEngineImpl : public PianoSynth::Core::ICoreEngine {
public:
    bool initialize(const std::string& config_path) override {
        std::cout << "Core engine initialized with config: " << config_path << std::endl;
        return true;
    }
    
    void shutdown() override {
        std::cout << "Core engine shutdown" << std::endl;
    }
    
    void start() override {
        std::cout << "Core engine started" << std::endl;
    }
    
    void stop() override {
        std::cout << "Core engine stopped" << std::endl;
    }
    
    void loadDLL(const std::string& dll_type, const std::string& dll_path) override {
        std::cout << "Loading DLL: type=" << dll_type << " path=" << dll_path << std::endl;
    }
    
    void unloadDLL(const std::string& dll_type) override {
        std::cout << "Unloading DLL: type=" << dll_type << std::endl;
    }
    
    void run() override {
        std::cout << "Core engine running..." << std::endl;
        // Simple run loop for testing
        for (int i = 0; i < 5; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Engine tick " << (i + 1) << std::endl;
        }
        std::cout << "Core engine run completed" << std::endl;
    }
    
    void configure(const std::string& json_config) override {
        std::cout << "Core engine configured" << std::endl;
    }
};

// DLL exports
extern "C" {
    PianoSynth::Core::ICoreEngine* createCoreEngine() {
        return new CoreEngineImpl();
    }
    
    void destroyCoreEngine(PianoSynth::Core::ICoreEngine* engine) {
        delete engine;
    }
}
