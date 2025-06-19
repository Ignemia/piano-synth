#pragma once

#include <memory>
#include <string>

namespace PianoSynth {

class Application {
public:
    Application();
    ~Application();
    
    // Application lifecycle
    bool initialize(const std::string& config_file, bool verbose = false);
    bool start();
    void stop();
    void shutdown();
    
    // Special operations
    void list_devices();
    bool test_audio();
    void run_interactive(volatile bool& running);
    
    // Runtime control
    void set_master_volume(float volume);
    float get_master_volume() const;
    void all_notes_off();
    
    // Status
    bool is_running() const;
    double get_cpu_usage() const;
    int get_active_voices() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace PianoSynth
