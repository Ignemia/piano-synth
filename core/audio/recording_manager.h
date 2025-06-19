#pragma once

#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <mutex>
#include <lame/lame.h>

namespace PianoSynth {
namespace Audio {

/**
 * Handles recording of audio sessions to MP3 files
 * using the LAME MP3 encoder
 */
class RecordingManager {
public:
    RecordingManager();
    ~RecordingManager();

    // Initialization
    bool initialize();
    void shutdown();
    
    // Recording control
    bool startRecording(const std::string& filename);
    void stopRecording();
    bool isRecording() const { return recording_active_; }
    
    // Audio input
    void addSamples(const std::vector<float>& samples);
    
    // Settings
    void setBitrate(int bitrate) { mp3_bitrate_ = bitrate; }
    void setQuality(int quality) { mp3_quality_ = quality; }
    
    // Status
    std::string getCurrentFilename() const { return current_filename_; }
    double getRecordingDuration() const; // in seconds
    size_t getBytesWritten() const { return bytes_written_; }
    
private:
    // LAME encoder state
    lame_global_flags* lame_flags_;
    bool lame_initialized_;
    
    // Recording state
    bool recording_active_;
    std::string current_filename_;
    std::ofstream output_file_;
    
    // Audio parameters
    double sample_rate_;
    int channels_;
    int mp3_bitrate_;
    int mp3_quality_;
    
    // Statistics
    size_t samples_recorded_;
    size_t bytes_written_;
    
    // Thread safety
    mutable std::mutex recording_mutex_;
    
    // Internal buffers
    std::vector<short> pcm_buffer_;
    std::vector<unsigned char> mp3_buffer_;
    
    // Internal methods
    bool initializeLame();
    void shutdownLame();
    bool setupOutputFile(const std::string& filename);
    void closeOutputFile();
    
    // Audio processing
    void convertFloatToPCM(const std::vector<float>& input, std::vector<short>& output);
    bool encodeToMP3(const std::vector<short>& pcm_data);
    void flushMP3Encoder();
    
    // Utility functions
    bool isValidFilename(const std::string& filename);
    std::string generateTimestampedFilename(const std::string& base_name);
    void writeMP3Headers();
};

} // namespace Audio
} // namespace PianoSynth
