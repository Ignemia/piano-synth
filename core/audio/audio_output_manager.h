#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <portaudio.h>

namespace PianoSynth {
namespace Audio {

/**
 * Manages audio output to the system's default speakers
 * using PortAudio for cross-platform compatibility
 */
class AudioOutputManager {
public:
    AudioOutputManager();
    ~AudioOutputManager();

    // Initialization
    bool initialize();
    void shutdown();
    
    // Audio output
    void outputBuffer(const std::vector<float>& audio_buffer);
    void setVolume(float volume);
    
    // Device management
    bool selectOutputDevice(int device_index = -1); // -1 for default
    std::vector<std::string> getAvailableOutputDevices();
    
    // Status
    bool isInitialized() const { return initialized_; }
    bool isActive() const { return stream_active_; }
    double getCurrentLatency() const;
    
private:
    // PortAudio state
    PaStream* stream_;
    bool initialized_;
    bool stream_active_;
    int selected_device_;
    
    // Audio parameters
    double sample_rate_;
    int frames_per_buffer_;
    int channels_;
    float master_volume_;
    
    // Internal buffer for PortAudio callback
    std::vector<float> output_buffer_;
    mutable std::mutex buffer_mutex_;
    
    // PortAudio callback
    static int audioCallback(const void* input, void* output,
                           unsigned long frame_count,
                           const PaStreamCallbackTimeInfo* time_info,
                           PaStreamCallbackFlags status_flags,
                           void* user_data);
    
    int processAudio(float* output, unsigned long frame_count);
    
    // Internal methods
    bool initializePortAudio();
    bool openAudioStream();
    void closeAudioStream();
    PaDeviceIndex findDefaultOutputDevice();
    bool isDeviceSupported(PaDeviceIndex device);
    
    // Utility functions
    void logDeviceInfo();
    std::string getDeviceName(PaDeviceIndex device);
};

} // namespace Audio
} // namespace PianoSynth
