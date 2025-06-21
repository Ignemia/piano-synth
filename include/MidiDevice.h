/**
 * @file MidiDevice.h
 * @brief [AI GENERATED] Real MIDI device interface with I/O capabilities and device detection.
 */

#pragma once
#include "MidiInput.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>

/**
 * @brief [AI GENERATED] MIDI device information structure.
 */
struct MidiDeviceInfo {
    int deviceId;              /**< System device ID. */
    std::string name;          /**< Device name. */
    std::string manufacturer;  /**< Manufacturer name. */
    bool isInput;             /**< True if device supports input. */
    bool isOutput;            /**< True if device supports output. */
    bool isConnected;         /**< True if device is currently connected. */
    int portCount;            /**< Number of MIDI ports. */
};

/**
 * @brief [AI GENERATED] Real-time MIDI message structure.
 */
struct RealTimeMidiMessage {
    uint8_t status;           /**< MIDI status byte. */
    uint8_t data1;            /**< First data byte. */
    uint8_t data2;            /**< Second data byte. */
    double timestamp;         /**< Message timestamp in seconds. */
    int channel;              /**< MIDI channel (1-16). */
    int deviceId;             /**< Source device ID. */
};

/**
 * @brief [AI GENERATED] MIDI message types for easier handling.
 */
enum class MidiMessageType {
    NoteOff = 0x80,
    NoteOn = 0x90,
    PolyphonicAftertouch = 0xA0,
    ControlChange = 0xB0,
    ProgramChange = 0xC0,
    ChannelAftertouch = 0xD0,
    PitchBend = 0xE0,
    SystemExclusive = 0xF0,
    SystemCommon = 0xF1,
    SystemRealtime = 0xF8,
    Unknown = 0xFF
};

/**
 * @brief [AI GENERATED] MIDI device error codes.
 */
enum class MidiError {
    None = 0,
    DeviceNotFound,
    DeviceNotConnected,
    DeviceAlreadyOpen,
    DeviceBusy,
    InvalidMessage,
    BufferOverflow,
    SystemError,
    NotSupported
};

/**
 * @brief [AI GENERATED] Callback function type for MIDI input events.
 */
using MidiInputCallback = std::function<void(const RealTimeMidiMessage& message)>;

/**
 * @brief [AI GENERATED] Callback function type for device connection events.
 */
using DeviceConnectionCallback = std::function<void(const MidiDeviceInfo& device, bool connected)>;

/**
 * @brief [AI GENERATED] Abstract base class for MIDI device interface.
 */
class MidiDeviceInterface {
public:
    virtual ~MidiDeviceInterface() = default;
    
    // Device enumeration
    virtual std::vector<MidiDeviceInfo> getAvailableDevices() = 0;
    virtual MidiDeviceInfo getDeviceInfo(int deviceId) = 0;
    virtual bool isDeviceConnected(int deviceId) = 0;
    
    // Device connection
    virtual MidiError openInputDevice(int deviceId) = 0;
    virtual MidiError openOutputDevice(int deviceId) = 0;
    virtual MidiError closeDevice(int deviceId) = 0;
    virtual void closeAllDevices() = 0;
    
    // Real-time MIDI I/O
    virtual void setInputCallback(MidiInputCallback callback) = 0;
    virtual MidiError sendMessage(int deviceId, const RealTimeMidiMessage& message) = 0;
    virtual MidiError sendRawMessage(int deviceId, const uint8_t* data, size_t length) = 0;
    
    // Device monitoring
    virtual void setDeviceConnectionCallback(DeviceConnectionCallback callback) = 0;
    virtual void startDeviceMonitoring() = 0;
    virtual void stopDeviceMonitoring() = 0;
    
    // Utility functions
    virtual std::string getErrorString(MidiError error) = 0;
    virtual MidiMessageType getMessageType(uint8_t status) = 0;
    virtual bool isValidMidiMessage(const RealTimeMidiMessage& message) = 0;
};

/**
 * @brief [AI GENERATED] Cross-platform MIDI device manager.
 */
class MidiDevice {
private:
    std::unique_ptr<MidiDeviceInterface> interface_;
    std::vector<MidiDeviceInfo> connectedDevices_;
    MidiInputCallback inputCallback_;
    DeviceConnectionCallback connectionCallback_;
    
    // Real-time processing
    std::atomic<bool> isProcessing_;
    std::thread processingThread_;
    std::mutex messageQueueMutex_;
    std::queue<RealTimeMidiMessage> messageQueue_;
    
    // M-Audio Oxygen Pro 61 specific
    int oxygenProDeviceId_;
    bool oxygenProConnected_;
    
    // Statistics
    std::atomic<uint64_t> messagesReceived_;
    std::atomic<uint64_t> messagesSent_;
    std::atomic<uint64_t> droppedMessages_;

public:
    MidiDevice();
    ~MidiDevice();
    
    // Disable copy/move for safety
    MidiDevice(const MidiDevice&) = delete;
    MidiDevice& operator=(const MidiDevice&) = delete;
    MidiDevice(MidiDevice&&) = delete;
    MidiDevice& operator=(MidiDevice&&) = delete;
    
    // Initialization
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // Device management
    std::vector<MidiDeviceInfo> scanForDevices();
    MidiDeviceInfo findDevice(const std::string& namePattern);
    MidiDeviceInfo findMAudioOxygenPro();
    bool connectToDevice(int deviceId);
    bool connectToDevice(const std::string& deviceName);
    void disconnectDevice(int deviceId);
    void disconnectAllDevices();
    
    // M-Audio Oxygen Pro 61 specific functions
    bool connectToOxygenPro();
    bool isOxygenProConnected() const;
    void configureOxygenPro();
    MidiError sendOxygenProSysEx(const std::vector<uint8_t>& sysexData);
    
    // Real-time MIDI processing
    void setMidiInputCallback(MidiInputCallback callback);
    void setDeviceConnectionCallback(DeviceConnectionCallback callback);
    void startRealTimeProcessing();
    void stopRealTimeProcessing();
    bool isProcessingRealTime() const;
    
    // MIDI I/O
    MidiError sendNoteOn(int deviceId, int channel, int note, int velocity);
    MidiError sendNoteOff(int deviceId, int channel, int note, int velocity = 64);
    MidiError sendControlChange(int deviceId, int channel, int controller, int value);
    MidiError sendProgramChange(int deviceId, int channel, int program);
    MidiError sendPitchBend(int deviceId, int channel, int value);
    
    // Convenience functions for piano synthesis
    MidiError sendKeyEvent(int deviceId, const KeyEvent& keyEvent);
    std::vector<KeyEvent> getRecentKeyEvents(double timeWindow = 1.0);
    void clearKeyEventHistory();
    
    // Utility functions
    static RealTimeMidiMessage parseRawMidiMessage(const uint8_t* data, size_t length, double timestamp, int deviceId);
    static std::vector<uint8_t> serializeMidiMessage(const RealTimeMidiMessage& message);
    static bool isNoteOnMessage(const RealTimeMidiMessage& message);
    static bool isNoteOffMessage(const RealTimeMidiMessage& message);
    static bool isDrumPadMessage(const RealTimeMidiMessage& message);
    static DeviceType getDeviceTypeFromMessage(const RealTimeMidiMessage& message);
    
    // Statistics and monitoring
    uint64_t getMessagesReceived() const;
    uint64_t getMessagesSent() const;
    uint64_t getDroppedMessages() const;
    double getInputLatency() const;
    double getOutputLatency() const;
    void resetStatistics();
    
    // Configuration
    void setBufferSize(size_t bufferSize);
    void setLatencyTarget(double milliseconds);
    void enableVelocityCurve(bool enabled);
    void setVelocityCurve(const std::vector<float>& curve);
    
    // Error handling
    std::string getLastErrorString() const;
    MidiError getLastError() const;
    void clearErrors();
    
private:
    // Internal processing
    void processingThreadFunction();
    void handleMidiMessage(const RealTimeMidiMessage& message);
    void handleDeviceConnection(const MidiDeviceInfo& device, bool connected);
    void updateDeviceList();
    
    // Platform-specific initialization
    std::unique_ptr<MidiDeviceInterface> createPlatformInterface();
    
    // M-Audio Oxygen Pro 61 helpers
    bool identifyOxygenPro(const MidiDeviceInfo& device);
    void setupOxygenProPads();
    void setupOxygenProKnobs();
    
    // Message processing helpers
    KeyEvent convertMidiToKeyEvent(const RealTimeMidiMessage& message);
    bool shouldProcessMessage(const RealTimeMidiMessage& message);
    void updateLatencyStatistics(const RealTimeMidiMessage& message);
    
    // Error handling
    MidiError lastError_;
    std::string lastErrorString_;
    std::mutex errorMutex_;
    
    // Configuration
    size_t bufferSize_;
    double latencyTarget_;
    bool velocityCurveEnabled_;
    std::vector<float> velocityCurve_;
    
    // Timing and latency
    std::atomic<double> avgInputLatency_;
    std::atomic<double> avgOutputLatency_;
    std::chrono::high_resolution_clock::time_point lastMessageTime_;
    
    // Key event history for piano synthesis
    std::mutex keyEventHistoryMutex_;
    std::vector<KeyEvent> keyEventHistory_;
    static constexpr size_t MAX_KEY_EVENT_HISTORY = 1000;
};

/**
 * @brief [AI GENERATED] MIDI device factory for creating platform-specific implementations.
 */
class MidiDeviceFactory {
public:
    static std::unique_ptr<MidiDevice> createDevice();
    static std::vector<std::string> getSupportedPlatforms();
    static bool isPlatformSupported();
    static std::string getCurrentPlatform();
};

/**
 * @brief [AI GENERATED] MIDI device discovery helper.
 */
class MidiDeviceDiscovery {
public:
    static std::vector<MidiDeviceInfo> findAllDevices();
    static std::vector<MidiDeviceInfo> findControllerKeyboards();
    static std::vector<MidiDeviceInfo> findSynthesizers();
    static MidiDeviceInfo findBestPianoController();
    static bool isMAudioDevice(const MidiDeviceInfo& device);
    static bool isPianoKeyboard(const MidiDeviceInfo& device);
    static int scoreDeviceForPiano(const MidiDeviceInfo& device);
};