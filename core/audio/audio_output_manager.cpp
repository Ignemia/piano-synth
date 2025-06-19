#include "audio_output_manager.h"
#include "../utils/logger.h"
#include "../utils/constants.h"
#include <algorithm>
#include <cstring>

namespace PianoSynth {
namespace Audio {

AudioOutputManager::AudioOutputManager() 
    : stream_(nullptr),
      initialized_(false),
      stream_active_(false),
      selected_device_(-1),
      sample_rate_(Constants::SAMPLE_RATE),
      frames_per_buffer_(Constants::BUFFER_SIZE),
      channels_(Constants::CHANNELS),
      master_volume_(0.8f) {
}

AudioOutputManager::~AudioOutputManager() {
    shutdown();
}

bool AudioOutputManager::initialize() {
    Utils::Logger logger;
    logger.info("Initializing Audio Output Manager...");
    
    if (!initializePortAudio()) {
        logger.error("Failed to initialize PortAudio");
        return false;
    }
    
    if (!openAudioStream()) {
        logger.error("Failed to open audio stream");
        Pa_Terminate();
        return false;
    }
    
    // Start the audio stream
    PaError err = Pa_StartStream(stream_);
    if (err != paNoError) {
        logger.error("Failed to start audio stream: " + std::string(Pa_GetErrorText(err)));
        closeAudioStream();
        Pa_Terminate();
        return false;
    }
    
    stream_active_ = true;
    initialized_ = true;
    
    logDeviceInfo();
    logger.info("Audio Output Manager initialized successfully");
    
    return true;
}

void AudioOutputManager::shutdown() {
    Utils::Logger logger;
    logger.info("Shutting down Audio Output Manager...");
    
    if (stream_active_) {
        PaError err = Pa_StopStream(stream_);
        if (err != paNoError) {
            logger.warning("Error stopping audio stream: " + std::string(Pa_GetErrorText(err)));
        }
        stream_active_ = false;
    }
    
    closeAudioStream();
    
    if (initialized_) {
        Pa_Terminate();
        initialized_ = false;
    }
    
    logger.info("Audio Output Manager shutdown complete");
}

void AudioOutputManager::outputBuffer(const std::vector<float>& audio_buffer) {
    if (!stream_active_ || audio_buffer.empty()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    // Ensure output buffer is correctly sized
    size_t required_size = frames_per_buffer_ * channels_;
    if (output_buffer_.size() != required_size) {
        output_buffer_.resize(required_size, 0.0f);
    }
    
    // Copy and apply master volume
    size_t copy_size = std::min(audio_buffer.size(), output_buffer_.size());
    for (size_t i = 0; i < copy_size; ++i) {
        output_buffer_[i] = audio_buffer[i] * master_volume_;
    }
    
    // Zero out remaining samples if input buffer is smaller
    if (copy_size < output_buffer_.size()) {
        std::fill(output_buffer_.begin() + copy_size, output_buffer_.end(), 0.0f);
    }
}

void AudioOutputManager::setVolume(float volume) {
    master_volume_ = std::clamp(volume, 0.0f, 1.0f);
}

bool AudioOutputManager::selectOutputDevice(int device_index) {
    if (device_index == -1) {
        selected_device_ = findDefaultOutputDevice();
    } else {
        if (device_index >= 0 && device_index < Pa_GetDeviceCount()) {
            if (isDeviceSupported(device_index)) {
                selected_device_ = device_index;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    
    // If stream is already open, restart with new device
    if (initialized_) {
        bool was_active = stream_active_;
        
        if (was_active) {
            Pa_StopStream(stream_);
            stream_active_ = false;
        }
        
        closeAudioStream();
        
        if (!openAudioStream()) {
            return false;
        }
        
        if (was_active) {
            PaError err = Pa_StartStream(stream_);
            if (err == paNoError) {
                stream_active_ = true;
            } else {
                return false;
            }
        }
    }
    
    return true;
}

std::vector<std::string> AudioOutputManager::getAvailableOutputDevices() {
    std::vector<std::string> devices;
    
    if (!initialized_) {
        return devices;
    }
    
    int device_count = Pa_GetDeviceCount();
    for (int i = 0; i < device_count; ++i) {
        const PaDeviceInfo* device_info = Pa_GetDeviceInfo(i);
        if (device_info && device_info->maxOutputChannels > 0) {
            devices.push_back(getDeviceName(i));
        }
    }
    
    return devices;
}

double AudioOutputManager::getCurrentLatency() const {
    if (stream_ && stream_active_) {
        const PaStreamInfo* stream_info = Pa_GetStreamInfo(stream_);
        if (stream_info) {
            return stream_info->outputLatency;
        }
    }
    return 0.0;
}

int AudioOutputManager::audioCallback(const void* input, void* output,
                                     unsigned long frame_count,
                                     const PaStreamCallbackTimeInfo* time_info,
                                     PaStreamCallbackFlags status_flags,
                                     void* user_data) {
    (void)input; // Unused
    (void)time_info; // Unused
    (void)status_flags; // Unused
    
    AudioOutputManager* manager = static_cast<AudioOutputManager*>(user_data);
    return manager->processAudio(static_cast<float*>(output), frame_count);
}

int AudioOutputManager::processAudio(float* output, unsigned long frame_count) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    size_t samples_to_copy = frame_count * channels_;
    
    if (output_buffer_.size() >= samples_to_copy) {
        // Copy from our buffer to PortAudio output
        std::memcpy(output, output_buffer_.data(), samples_to_copy * sizeof(float));
    } else {
        // Not enough data, output silence
        std::memset(output, 0, samples_to_copy * sizeof(float));
    }
    
    return paContinue;
}

bool AudioOutputManager::initializePortAudio() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return false;
    }
    
    // Check if we have any output devices
    if (Pa_GetDeviceCount() == 0) {
        Pa_Terminate();
        return false;
    }
    
    return true;
}

bool AudioOutputManager::openAudioStream() {
    PaStreamParameters output_parameters;
    
    // Find appropriate output device
    if (selected_device_ == -1) {
        selected_device_ = findDefaultOutputDevice();
    }
    
    if (selected_device_ == paNoDevice) {
        return false;
    }
    
    const PaDeviceInfo* device_info = Pa_GetDeviceInfo(selected_device_);
    if (!device_info) {
        return false;
    }
    
    // Configure output parameters
    output_parameters.device = selected_device_;
    output_parameters.channelCount = std::min(channels_, device_info->maxOutputChannels);
    output_parameters.sampleFormat = paFloat32;
    output_parameters.suggestedLatency = device_info->defaultLowOutputLatency;
    output_parameters.hostApiSpecificStreamInfo = nullptr;
    
    // Update actual channel count
    channels_ = output_parameters.channelCount;
    
    // Open the stream
    PaError err = Pa_OpenStream(
        &stream_,
        nullptr,                // No input
        &output_parameters,     // Output parameters
        sample_rate_,           // Sample rate
        frames_per_buffer_,     // Frames per buffer
        paClipOff,              // No clipping
        audioCallback,          // Callback function
        this                    // User data
    );
    
    if (err != paNoError) {
        stream_ = nullptr;
        return false;
    }
    
    return true;
}

void AudioOutputManager::closeAudioStream() {
    if (stream_) {
        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }
}

PaDeviceIndex AudioOutputManager::findDefaultOutputDevice() {
    PaDeviceIndex default_device = Pa_GetDefaultOutputDevice();
    
    if (default_device == paNoDevice) {
        // Try to find any output device
        int device_count = Pa_GetDeviceCount();
        for (int i = 0; i < device_count; ++i) {
            const PaDeviceInfo* device_info = Pa_GetDeviceInfo(i);
            if (device_info && device_info->maxOutputChannels > 0) {
                return i;
            }
        }
        return paNoDevice;
    }
    
    return default_device;
}

bool AudioOutputManager::isDeviceSupported(PaDeviceIndex device) {
    if (device < 0 || device >= Pa_GetDeviceCount()) {
        return false;
    }
    
    const PaDeviceInfo* device_info = Pa_GetDeviceInfo(device);
    if (!device_info || device_info->maxOutputChannels == 0) {
        return false;
    }
    
    // Test if we can open a stream with this device
    PaStreamParameters output_parameters;
    output_parameters.device = device;
    output_parameters.channelCount = std::min(2, device_info->maxOutputChannels);
    output_parameters.sampleFormat = paFloat32;
    output_parameters.suggestedLatency = device_info->defaultLowOutputLatency;
    output_parameters.hostApiSpecificStreamInfo = nullptr;
    
    PaError err = Pa_IsFormatSupported(nullptr, &output_parameters, sample_rate_);
    return (err == paFormatIsSupported);
}

void AudioOutputManager::logDeviceInfo() {
    Utils::Logger logger;
    
    if (selected_device_ != paNoDevice) {
        const PaDeviceInfo* device_info = Pa_GetDeviceInfo(selected_device_);
        const PaHostApiInfo* host_api_info = Pa_GetHostApiInfo(device_info->hostApi);
        
        logger.info("Using audio device: " + getDeviceName(selected_device_));
        logger.info("Host API: " + std::string(host_api_info->name));
        logger.info("Sample rate: " + std::to_string(sample_rate_) + " Hz");
        logger.info("Buffer size: " + std::to_string(frames_per_buffer_) + " frames");
        logger.info("Channels: " + std::to_string(channels_));
        logger.info("Latency: " + std::to_string(getCurrentLatency() * 1000.0) + " ms");
    }
}

std::string AudioOutputManager::getDeviceName(PaDeviceIndex device) {
    const PaDeviceInfo* device_info = Pa_GetDeviceInfo(device);
    if (device_info && device_info->name) {
        return std::string(device_info->name);
    }
    return "Unknown Device";
}

} // namespace Audio
} // namespace PianoSynth