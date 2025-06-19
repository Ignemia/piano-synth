#include "recording_manager.h"
#include "../utils/logger.h"
#include "../utils/constants.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace PianoSynth {
namespace Audio {

RecordingManager::RecordingManager()
    : lame_flags_(nullptr),
      lame_initialized_(false),
      recording_active_(false),
      sample_rate_(Constants::SAMPLE_RATE),
      channels_(Constants::CHANNELS),
      mp3_bitrate_(Constants::MP3_BITRATE),
      mp3_quality_(Constants::MP3_QUALITY),
      samples_recorded_(0),
      bytes_written_(0) {
}

RecordingManager::~RecordingManager() {
    shutdown();
}

bool RecordingManager::initialize() {
    Utils::Logger logger;
    logger.info("Initializing Recording Manager...");
    
    if (!initializeLame()) {
        logger.error("Failed to initialize LAME encoder");
        return false;
    }
    
    logger.info("Recording Manager initialized successfully");
    return true;
}

void RecordingManager::shutdown() {
    Utils::Logger logger;
    logger.info("Shutting down Recording Manager...");
    
    if (recording_active_) {
        stopRecording();
    }
    
    shutdownLame();
    
    logger.info("Recording Manager shutdown complete");
}

bool RecordingManager::startRecording(const std::string& filename) {
    std::lock_guard<std::mutex> lock(recording_mutex_);
    
    Utils::Logger logger;
    
    if (recording_active_) {
        logger.warning("Recording already active, stopping current recording");
        stopRecording();
    }
    
    if (!lame_initialized_) {
        logger.error("LAME encoder not initialized");
        return false;
    }
    
    // Validate and setup filename
    std::string actual_filename = filename;
    if (!isValidFilename(actual_filename)) {
        actual_filename = generateTimestampedFilename("recording");
        logger.warning("Invalid filename provided, using: " + actual_filename);
    }
    
    if (!setupOutputFile(actual_filename)) {
        logger.error("Failed to setup output file: " + actual_filename);
        return false;
    }
    
    // Reset LAME encoder for new recording
    if (lame_encode_flush(lame_flags_, nullptr, 0) < 0) {
        logger.error("Failed to reset LAME encoder");
        closeOutputFile();
        return false;
    }
    
    // Reset statistics
    samples_recorded_ = 0;
    bytes_written_ = 0;
    
    recording_active_ = true;
    current_filename_ = actual_filename;
    
    logger.info("Started recording to: " + current_filename_);
    return true;
}

void RecordingManager::stopRecording() {
    std::lock_guard<std::mutex> lock(recording_mutex_);
    
    if (!recording_active_) {
        return;
    }
    
    Utils::Logger logger;
    
    // Flush remaining data to MP3
    flushMP3Encoder();
    
    // Close output file
    closeOutputFile();
    
    recording_active_ = false;
    
    logger.info("Stopped recording. File: " + current_filename_);
    logger.info("Duration: " + std::to_string(getRecordingDuration()) + " seconds");
    logger.info("Bytes written: " + std::to_string(bytes_written_));
    
    current_filename_.clear();
}

void RecordingManager::addSamples(const std::vector<float>& samples) {
    if (!recording_active_ || samples.empty()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(recording_mutex_);
    
    // Convert float samples to PCM
    convertFloatToPCM(samples, pcm_buffer_);
    
    // Encode to MP3
    if (!encodeToMP3(pcm_buffer_)) {
        Utils::Logger logger;
        logger.error("Failed to encode samples to MP3");
        stopRecording();
        return;
    }
    
    samples_recorded_ += samples.size();
}

double RecordingManager::getRecordingDuration() const {
    if (samples_recorded_ == 0) {
        return 0.0;
    }
    
    // Convert samples to duration based on sample rate and channels
    size_t frames = samples_recorded_ / channels_;
    return static_cast<double>(frames) / sample_rate_;
}

bool RecordingManager::initializeLame() {
    lame_flags_ = lame_init();
    if (!lame_flags_) {
        return false;
    }
    
    // Configure LAME parameters
    lame_set_in_samplerate(lame_flags_, static_cast<int>(sample_rate_));
    lame_set_num_channels(lame_flags_, channels_);
    lame_set_brate(lame_flags_, mp3_bitrate_);
    lame_set_quality(lame_flags_, mp3_quality_);
    
    // Set encoding mode
    if (channels_ == 1) {
        lame_set_mode(lame_flags_, MONO);
    } else {
        lame_set_mode(lame_flags_, STEREO);
    }
    
    // Additional quality settings
    lame_set_VBR(lame_flags_, vbr_off); // Use constant bitrate
    lame_set_copyright(lame_flags_, 0);
    lame_set_original(lame_flags_, 1);
    lame_set_error_protection(lame_flags_, 0);
    lame_set_extension(lame_flags_, 0);
    lame_set_strict_ISO(lame_flags_, 0);
    
    // Initialize the encoder
    if (lame_init_params(lame_flags_) < 0) {
        lame_close(lame_flags_);
        lame_flags_ = nullptr;
        return false;
    }
    
    lame_initialized_ = true;
    
    // Prepare MP3 buffer (should be large enough for worst case)
    mp3_buffer_.resize(static_cast<size_t>(1.25 * Constants::BUFFER_SIZE + 7200));
    
    return true;
}

void RecordingManager::shutdownLame() {
    if (lame_flags_) {
        lame_close(lame_flags_);
        lame_flags_ = nullptr;
    }
    lame_initialized_ = false;
}

bool RecordingManager::setupOutputFile(const std::string& filename) {
    // Create directory if it doesn't exist
    std::filesystem::path file_path(filename);
    std::filesystem::path directory = file_path.parent_path();
    
    if (!directory.empty()) {
        try {
            std::filesystem::create_directories(directory);
        } catch (const std::filesystem::filesystem_error& e) {
            Utils::Logger logger;
            logger.error("Failed to create directory: " + std::string(e.what()));
            return false;
        }
    }
    
    // Open output file
    output_file_.open(filename, std::ios::binary | std::ios::trunc);
    if (!output_file_.is_open()) {
        return false;
    }
    
    return true;
}

void RecordingManager::closeOutputFile() {
    if (output_file_.is_open()) {
        output_file_.close();
    }
}

void RecordingManager::convertFloatToPCM(const std::vector<float>& input, std::vector<short>& output) {
    output.resize(input.size());
    
    for (size_t i = 0; i < input.size(); ++i) {
        // Clamp to valid range and convert to 16-bit PCM
        float sample = std::clamp(input[i], -1.0f, 1.0f);
        output[i] = static_cast<short>(sample * 32767.0f);
    }
}

bool RecordingManager::encodeToMP3(const std::vector<short>& pcm_data) {
    if (!lame_initialized_ || !output_file_.is_open() || pcm_data.empty()) {
        return false;
    }
    
    int samples_per_channel = static_cast<int>(pcm_data.size() / channels_);
    int mp3_bytes = 0;
    
    if (channels_ == 1) {
        // Mono encoding
        mp3_bytes = lame_encode_buffer(
            lame_flags_,
            pcm_data.data(),        // Left channel
            nullptr,                // No right channel
            samples_per_channel,
            mp3_buffer_.data(),
            static_cast<int>(mp3_buffer_.size())
        );
    } else {
        // Stereo encoding - deinterleave the data
        std::vector<short> left_channel(samples_per_channel);
        std::vector<short> right_channel(samples_per_channel);
        
        for (int i = 0; i < samples_per_channel; ++i) {
            left_channel[i] = pcm_data[i * 2];
            right_channel[i] = pcm_data[i * 2 + 1];
        }
        
        mp3_bytes = lame_encode_buffer(
            lame_flags_,
            left_channel.data(),
            right_channel.data(),
            samples_per_channel,
            mp3_buffer_.data(),
            static_cast<int>(mp3_buffer_.size())
        );
    }
    
    if (mp3_bytes < 0) {
        Utils::Logger logger;
        logger.error("LAME encoding error: " + std::to_string(mp3_bytes));
        return false;
    }
    
    if (mp3_bytes > 0) {
        // Write MP3 data to file
        output_file_.write(reinterpret_cast<const char*>(mp3_buffer_.data()), mp3_bytes);
        if (output_file_.fail()) {
            Utils::Logger logger;
            logger.error("Failed to write MP3 data to file");
            return false;
        }
        
        bytes_written_ += mp3_bytes;
    }
    
    return true;
}

void RecordingManager::flushMP3Encoder() {
    if (!lame_initialized_ || !output_file_.is_open()) {
        return;
    }
    
    // Flush any remaining data
    int mp3_bytes = lame_encode_flush(
        lame_flags_,
        mp3_buffer_.data(),
        static_cast<int>(mp3_buffer_.size())
    );
    
    if (mp3_bytes > 0) {
        output_file_.write(reinterpret_cast<const char*>(mp3_buffer_.data()), mp3_bytes);
        bytes_written_ += mp3_bytes;
    }
    
    output_file_.flush();
}

bool RecordingManager::isValidFilename(const std::string& filename) {
    if (filename.empty()) {
        return false;
    }
    
    // Check for valid file extension
    std::filesystem::path file_path(filename);
    std::string extension = file_path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    return (extension == ".mp3");
}

std::string RecordingManager::generateTimestampedFilename(const std::string& base_name) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << base_name << "_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << ".mp3";
    
    return ss.str();
}

} // namespace Audio
} // namespace PianoSynth