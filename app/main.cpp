#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <signal.h>

#include "application.h"

static bool g_running = true;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  -c, --config <file>    Configuration file (default: config/core.json)\n"
              << "  -d, --daemon           Run as daemon\n"
              << "  -v, --verbose          Verbose output\n"
              << "  -h, --help             Show this help\n"
              << "  --list-devices         List available MIDI devices and exit\n"
              << "  --test-audio           Test audio output and exit\n"
              << "  --version              Show version information\n"
              << std::endl;
}

void print_version() {
    std::cout << "Piano Synth Modular v1.0.0\n"
              << "Modular Piano Synthesizer with Physical Modeling\n"
              << "Built with DLL architecture for extensibility\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    std::string config_file = "config/core.json";
    bool daemon_mode = false;
    bool verbose = false;
    bool list_devices = false;
    bool test_audio = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                std::cerr << "Error: --config requires a filename" << std::endl;
                return 1;
            }
        }
        else if (arg == "-d" || arg == "--daemon") {
            daemon_mode = true;
        }
        else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        }
        else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
        else if (arg == "--list-devices") {
            list_devices = true;
        }
        else if (arg == "--test-audio") {
            test_audio = true;
        }
        else if (arg == "--version") {
            print_version();
            return 0;
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        // Create application instance
        PianoSynth::Application app;
        
        if (verbose) {
            std::cout << "Initializing Piano Synth with config: " << config_file << std::endl;
        }
        
        // Initialize application
        if (!app.initialize(config_file, verbose)) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }
        
        // Handle special modes
        if (list_devices) {
            app.list_devices();
            return 0;
        }
        
        if (test_audio) {
            return app.test_audio() ? 0 : 1;
        }
        
        // Start the audio processing
        if (!app.start()) {
            std::cerr << "Failed to start audio processing" << std::endl;
            return 1;
        }
        
        if (verbose) {
            std::cout << "Piano Synth started successfully" << std::endl;
            std::cout << "Press Ctrl+C to quit" << std::endl;
        }
        
        // Main application loop
        if (daemon_mode) {
            // In daemon mode, just sleep and wait for signals
            while (g_running) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } else {
            // Interactive mode - simple CLI
            app.run_interactive(g_running);
        }
        
        if (verbose) {
            std::cout << "Stopping Piano Synth..." << std::endl;
        }
        
        // Clean shutdown
        app.stop();
        app.shutdown();
        
        if (verbose) {
            std::cout << "Piano Synth stopped successfully" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
