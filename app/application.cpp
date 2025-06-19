#include "application.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "=== Piano Synth Modular Application ===" << std::endl;
    
    try {
        PianoSynth::Application app;
        
        // Initialize with default config path
        std::string config_path = (argc > 1) ? argv[1] : "config/core.json";
        
        if (!app.initialize(config_path)) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }
        
        std::cout << "Application initialized successfully" << std::endl;
        
        // Run the application
        app.run();
        
        // Cleanup
        app.shutdown();
        std::cout << "Application shutdown complete" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
