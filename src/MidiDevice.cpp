#include "../include/MidiDevice.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <mmeapi.h>
#pragma comment(lib, "winmm.lib")
#elif __APPLE__
#include <CoreMIDI/CoreMIDI.h>
#include <CoreFoundation/CoreFoundation.h>
#elif __linux__
#include <alsa/asoundlib.h>
#include <alsa/seq.h>
#include <alsa/seq_midi_event.h>
#endif

/**
 * @brief [AI GENERATED] Cross-platform MIDI device interface implementation.
 */
class CrossPlatformMidiInterface : public MidiDeviceInterface {
private:
    MidiInputCallback inputCallback_;
    DeviceConnectionCallback connectionCallback_;
    std::vector<MidiDeviceInfo> availableDevices_;
    std::vector<int> openInputDevices_;
    std::vector<int> openOutputDevices_;
    
#ifdef _WIN32
    std::vector<HMIDIIN> midiInHandles_;
    std::vector<HMIDIOUT> midiOutHandles_;
#elif __APPLE__
    MIDIClientRef midiClient_;
    MIDIPortRef inputPort_;
    MIDIPortRef outputPort_;
#elif __linux__
    snd_seq_t* seq_;
    int inputPort_;
    int outputPort_;
#endif

public:
    CrossPlatformMidiInterface() {
        initializePlatform();
    }
    
    ~CrossPlatformMidiInterface() {
        closeAllDevices();
        shutdownPlatform();
    }
    
    std::vector<MidiDeviceInfo> getAvailableDevices() override {
        scanDevices();
        return availableDevices_;
    }
    
    MidiDeviceInfo getDeviceInfo(int deviceId) override {
        auto it = std::find_if(availableDevices_.begin(), availableDevices_.end(),
                              [deviceId](const MidiDeviceInfo& device) {
                                  return device.deviceId == deviceId;
                              });
        if (it != availableDevices_.end()) {
            return *it;
        }
        return MidiDeviceInfo{-1, "Unknown", "Unknown", false, false, false, 0};
    }
    
    bool isDeviceConnected(int deviceId) override {
        auto device = getDeviceInfo(deviceId);
        return device.deviceId != -1 && device.isConnected;
    }
    
    MidiError openInputDevice(int deviceId) override {
        if (std::find(openInputDevices_.begin(), openInputDevices_.end(), deviceId) != openInputDevices_.end()) {
            return MidiError::DeviceAlreadyOpen;
        }
        
        if (openInputDevicePlatform(deviceId)) {
            openInputDevices_.push_back(deviceId);
            return MidiError::None;
        }
        return MidiError::DeviceNotFound;
    }
    
    MidiError openOutputDevice(int deviceId) override {
        if (std::find(openOutputDevices_.begin(), openOutputDevices_.end(), deviceId) != openOutputDevices_.end()) {
            return MidiError::DeviceAlreadyOpen;
        }
        
        if (openOutputDevicePlatform(deviceId)) {
            openOutputDevices_.push_back(deviceId);
            return MidiError::None;
        }
        return MidiError::DeviceNotFound;
    }
    
    MidiError closeDevice(int deviceId) override {
        closeInputDevicePlatform(deviceId);
        closeOutputDevicePlatform(deviceId);
        
        openInputDevices_.erase(std::remove(openInputDevices_.begin(), openInputDevices_.end(), deviceId), openInputDevices_.end());
        openOutputDevices_.erase(std::remove(openOutputDevices_.begin(), openOutputDevices_.end(), deviceId), openOutputDevices_.end());
        
        return MidiError::None;
    }
    
    void closeAllDevices() override {
        for (int deviceId : openInputDevices_) {
            closeInputDevicePlatform(deviceId);
        }
        for (int deviceId : openOutputDevices_) {
            closeOutputDevicePlatform(deviceId);
        }
        openInputDevices_.clear();
        openOutputDevices_.clear();
    }
    
    void setInputCallback(MidiInputCallback callback) override {
        inputCallback_ = callback;
    }
    
    MidiError sendMessage(int deviceId, const RealTimeMidiMessage& message) override {
        if (!isValidMidiMessage(message)) {
            return MidiError::InvalidMessage;
        }
        
        // Check if device exists first
        auto device = getDeviceInfo(deviceId);
        if (device.deviceId == -1) {
            return MidiError::DeviceNotFound;
        }
        
        auto data = serializeMidiMessage(message);
        return sendRawMessage(deviceId, data.data(), data.size());
    }
    
    MidiError sendRawMessage(int deviceId, const uint8_t* data, size_t length) override {
        if (std::find(openOutputDevices_.begin(), openOutputDevices_.end(), deviceId) == openOutputDevices_.end()) {
            return MidiError::DeviceNotConnected;
        }
        
        return sendRawMessagePlatform(deviceId, data, length);
    }
    
    void setDeviceConnectionCallback(DeviceConnectionCallback callback) override {
        connectionCallback_ = callback;
    }
    
    void startDeviceMonitoring() override {
        // Platform-specific device monitoring would go here
    }
    
    void stopDeviceMonitoring() override {
        // Platform-specific device monitoring stop would go here
    }
    
    std::string getErrorString(MidiError error) override {
        switch (error) {
            case MidiError::None: return "No error";
            case MidiError::DeviceNotFound: return "Device not found";
            case MidiError::DeviceNotConnected: return "Device not connected";
            case MidiError::DeviceAlreadyOpen: return "Device already open";
            case MidiError::DeviceBusy: return "Device busy";
            case MidiError::InvalidMessage: return "Invalid MIDI message";
            case MidiError::BufferOverflow: return "Buffer overflow";
            case MidiError::SystemError: return "System error";
            case MidiError::NotSupported: return "Operation not supported";
            default: return "Unknown error";
        }
    }
    
    MidiMessageType getMessageType(uint8_t status) override {
        uint8_t messageType = status & 0xF0;
        switch (messageType) {
            case 0x80: return MidiMessageType::NoteOff;
            case 0x90: return MidiMessageType::NoteOn;
            case 0xA0: return MidiMessageType::PolyphonicAftertouch;
            case 0xB0: return MidiMessageType::ControlChange;
            case 0xC0: return MidiMessageType::ProgramChange;
            case 0xD0: return MidiMessageType::ChannelAftertouch;
            case 0xE0: return MidiMessageType::PitchBend;
            case 0xF0: return MidiMessageType::SystemExclusive;
            default: return MidiMessageType::Unknown;
        }
    }
    
    bool isValidMidiMessage(const RealTimeMidiMessage& message) override {
        // Basic validation
        if (message.channel < 1 || message.channel > 16) return false;
        if (message.status < 0x80) return false;
        
        MidiMessageType type = getMessageType(message.status);
        switch (type) {
            case MidiMessageType::NoteOn:
            case MidiMessageType::NoteOff:
                return message.data1 < 128 && message.data2 < 128;
            case MidiMessageType::ControlChange:
                return message.data1 < 128 && message.data2 < 128;
            case MidiMessageType::ProgramChange:
                return message.data1 < 128;
            default:
                return true;
        }
    }

private:
    void initializePlatform() {
#ifdef _WIN32
        // Windows MIDI initialization
#elif __APPLE__
        // macOS Core MIDI initialization
        MIDIClientCreate(CFSTR("PianoSynth"), nullptr, nullptr, &midiClient_);
        MIDIInputPortCreate(midiClient_, CFSTR("Input"), midiInputCallback, this, &inputPort_);
        MIDIOutputPortCreate(midiClient_, CFSTR("Output"), &outputPort_);
#elif __linux__
        // Linux ALSA MIDI initialization
        if (snd_seq_open(&seq_, "default", SND_SEQ_OPEN_DUPLEX, 0) >= 0) {
            snd_seq_set_client_name(seq_, "PianoSynth");
            inputPort_ = snd_seq_create_simple_port(seq_, "Input", 
                SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION);
            outputPort_ = snd_seq_create_simple_port(seq_, "Output",
                SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_APPLICATION);
        }
#endif
    }
    
    void shutdownPlatform() {
#ifdef _WIN32
        // Windows cleanup
#elif __APPLE__
        if (midiClient_) {
            MIDIClientDispose(midiClient_);
        }
#elif __linux__
        if (seq_) {
            snd_seq_close(seq_);
        }
#endif
    }
    
    void scanDevices() {
        availableDevices_.clear();
        
#ifdef _WIN32
        // Windows MIDI device enumeration
        UINT numInputDevices = midiInGetNumDevs();
        for (UINT i = 0; i < numInputDevices; ++i) {
            MIDIINCAPS caps;
            if (midiInGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
                MidiDeviceInfo device;
                device.deviceId = i;
                device.name = caps.szPname;
                device.manufacturer = "Unknown";
                device.isInput = true;
                device.isOutput = false;
                device.isConnected = true;
                device.portCount = 1;
                availableDevices_.push_back(device);
            }
        }
        
        UINT numOutputDevices = midiOutGetNumDevs();
        for (UINT i = 0; i < numOutputDevices; ++i) {
            MIDIOUTCAPS caps;
            if (midiOutGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
                MidiDeviceInfo device;
                device.deviceId = i + 1000; // Offset to distinguish from input devices
                device.name = caps.szPname;
                device.manufacturer = "Unknown";
                device.isInput = false;
                device.isOutput = true;
                device.isConnected = true;
                device.portCount = 1;
                availableDevices_.push_back(device);
            }
        }
#elif __APPLE__
        // macOS Core MIDI device enumeration
        ItemCount sourceCount = MIDIGetNumberOfSources();
        for (ItemCount i = 0; i < sourceCount; ++i) {
            MIDIEndpointRef source = MIDIGetSource(i);
            CFStringRef name;
            MIDIObjectGetStringProperty(source, kMIDIPropertyName, &name);
            
            MidiDeviceInfo device;
            device.deviceId = i;
            
            // Convert CFString to std::string
            CFIndex length = CFStringGetLength(name);
            CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
            char* buffer = new char[maxSize];
            CFStringGetCString(name, buffer, maxSize, kCFStringEncodingUTF8);
            device.name = buffer;
            delete[] buffer;
            CFRelease(name);
            
            device.manufacturer = "Unknown";
            device.isInput = true;
            device.isOutput = false;
            device.isConnected = true;
            device.portCount = 1;
            availableDevices_.push_back(device);
        }
#elif __linux__
        // Linux ALSA MIDI device enumeration
        if (seq_) {
            snd_seq_client_info_t* clientInfo;
            snd_seq_port_info_t* portInfo;
            
            snd_seq_client_info_alloca(&clientInfo);
            snd_seq_port_info_alloca(&portInfo);
            
            snd_seq_client_info_set_client(clientInfo, -1);
            while (snd_seq_query_next_client(seq_, clientInfo) >= 0) {
                int clientId = snd_seq_client_info_get_client(clientInfo);
                
                snd_seq_port_info_set_client(portInfo, clientId);
                snd_seq_port_info_set_port(portInfo, -1);
                while (snd_seq_query_next_port(seq_, portInfo) >= 0) {
                    unsigned int caps = snd_seq_port_info_get_capability(portInfo);
                    if (caps & (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE)) {
                        MidiDeviceInfo device;
                        device.deviceId = clientId * 1000 + snd_seq_port_info_get_port(portInfo);
                        device.name = snd_seq_port_info_get_name(portInfo);
                        device.manufacturer = snd_seq_client_info_get_name(clientInfo);
                        device.isInput = caps & SND_SEQ_PORT_CAP_WRITE;
                        device.isOutput = caps & SND_SEQ_PORT_CAP_READ;
                        device.isConnected = true;
                        device.portCount = 1;
                        availableDevices_.push_back(device);
                    }
                }
            }
        }
#else
        // Generic/mock implementation for unsupported platforms
        MidiDeviceInfo mockDevice;
        mockDevice.deviceId = 0;
        mockDevice.name = "Mock MIDI Device";
        mockDevice.manufacturer = "Generic";
        mockDevice.isInput = true;
        mockDevice.isOutput = true;
        mockDevice.isConnected = true;
        mockDevice.portCount = 1;
        availableDevices_.push_back(mockDevice);
#endif
    }
    
    bool openInputDevicePlatform(int deviceId) {
#ifdef _WIN32
        HMIDIIN handle;
        MMRESULT result = midiInOpen(&handle, deviceId, reinterpret_cast<DWORD_PTR>(midiInputCallback), 
                                    reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
        if (result == MMSYSERR_NOERROR) {
            midiInStart(handle);
            midiInHandles_.push_back(handle);
            return true;
        }
#elif __APPLE__
        // macOS implementation would go here
        return true;
#elif __linux__
        // Linux implementation would go here
        return true;
#endif
        return false;
    }
    
    bool openOutputDevicePlatform(int deviceId) {
#ifdef _WIN32
        HMIDIOUT handle;
        MMRESULT result = midiOutOpen(&handle, deviceId - 1000, 0, 0, CALLBACK_NULL);
        if (result == MMSYSERR_NOERROR) {
            midiOutHandles_.push_back(handle);
            return true;
        }
#elif __APPLE__
        // macOS implementation would go here
        return true;
#elif __linux__
        // Linux implementation would go here
        return true;
#endif
        return false;
    }
    
    void closeInputDevicePlatform(int deviceId) {
#ifdef _WIN32
        // Find and close the corresponding handle
        for (auto it = midiInHandles_.begin(); it != midiInHandles_.end(); ++it) {
            midiInStop(*it);
            midiInClose(*it);
        }
        midiInHandles_.clear();
#endif
    }
    
    void closeOutputDevicePlatform(int deviceId) {
#ifdef _WIN32
        for (auto it = midiOutHandles_.begin(); it != midiOutHandles_.end(); ++it) {
            midiOutClose(*it);
        }
        midiOutHandles_.clear();
#endif
    }
    
    MidiError sendRawMessagePlatform(int deviceId, const uint8_t* data, size_t length) {
#ifdef _WIN32
        if (!midiOutHandles_.empty()) {
            DWORD message = 0;
            for (size_t i = 0; i < std::min(length, size_t(4)); ++i) {
                message |= (data[i] << (i * 8));
            }
            MMRESULT result = midiOutShortMsg(midiOutHandles_[0], message);
            return (result == MMSYSERR_NOERROR) ? MidiError::None : MidiError::SystemError;
        }
#endif
        return MidiError::DeviceNotConnected;
    }
    
    std::vector<uint8_t> serializeMidiMessage(const RealTimeMidiMessage& message) {
        std::vector<uint8_t> data;
        data.push_back(message.status | (message.channel - 1));
        
        MidiMessageType type = getMessageType(message.status);
        switch (type) {
            case MidiMessageType::NoteOn:
            case MidiMessageType::NoteOff:
            case MidiMessageType::ControlChange:
                data.push_back(message.data1);
                data.push_back(message.data2);
                break;
            case MidiMessageType::ProgramChange:
            case MidiMessageType::ChannelAftertouch:
                data.push_back(message.data1);
                break;
            default:
                break;
        }
        return data;
    }

#ifdef _WIN32
    static void CALLBACK midiInputCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
        if (wMsg == MIM_DATA) {
            CrossPlatformMidiInterface* interface = reinterpret_cast<CrossPlatformMidiInterface*>(dwInstance);
            if (interface && interface->inputCallback_) {
                // Parse the MIDI message
                DWORD midiMessage = static_cast<DWORD>(dwParam1);
                RealTimeMidiMessage msg;
                msg.status = midiMessage & 0xFF;
                msg.data1 = (midiMessage >> 8) & 0xFF;
                msg.data2 = (midiMessage >> 16) & 0xFF;
                msg.channel = (msg.status & 0x0F) + 1;
                msg.timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                msg.deviceId = 0; // Would need to map handle to device ID
                
                interface->inputCallback_(msg);
            }
        }
    }
#elif __APPLE__
    static void midiInputCallback(const MIDIPacketList* packetList, void* readProcRefCon, void* srcConnRefCon) {
        CrossPlatformMidiInterface* interface = reinterpret_cast<CrossPlatformMidiInterface*>(readProcRefCon);
        if (interface && interface->inputCallback_) {
            const MIDIPacket* packet = &packetList->packet[0];
            for (UInt32 i = 0; i < packetList->numPackets; ++i) {
                if (packet->length >= 2) {
                    RealTimeMidiMessage msg;
                    msg.status = packet->data[0];
                    msg.data1 = packet->length > 1 ? packet->data[1] : 0;
                    msg.data2 = packet->length > 2 ? packet->data[2] : 0;
                    msg.channel = (msg.status & 0x0F) + 1;
                    msg.timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                    msg.deviceId = 0;
                    
                    interface->inputCallback_(msg);
                }
                packet = MIDIPacketNext(packet);
            }
        }
    }
#endif
};

// MidiDevice implementation
MidiDevice::MidiDevice() 
    : interface_(nullptr)
    , oxygenProDeviceId_(-1)
    , oxygenProConnected_(false)
    , isProcessing_(false)
    , messagesReceived_(0)
    , messagesSent_(0)
    , droppedMessages_(0)
    , lastError_(MidiError::None)
    , bufferSize_(1024)
    , latencyTarget_(10.0)
    , velocityCurveEnabled_(false)
    , avgInputLatency_(0.0)
    , avgOutputLatency_(0.0) {
}

MidiDevice::~MidiDevice() {
    shutdown();
}

bool MidiDevice::initialize() {
    interface_ = std::make_unique<CrossPlatformMidiInterface>();
    
    // Set up callbacks
    interface_->setInputCallback([this](const RealTimeMidiMessage& message) {
        handleMidiMessage(message);
    });
    
    interface_->setDeviceConnectionCallback([this](const MidiDeviceInfo& device, bool connected) {
        handleDeviceConnection(device, connected);
    });
    
    // Start device monitoring
    interface_->startDeviceMonitoring();
    
    return true;
}

void MidiDevice::shutdown() {
    stopRealTimeProcessing();
    
    if (interface_) {
        interface_->stopDeviceMonitoring();
        interface_->closeAllDevices();
        interface_.reset();
    }
}

bool MidiDevice::isInitialized() const {
    return interface_ != nullptr;
}

std::vector<MidiDeviceInfo> MidiDevice::scanForDevices() {
    if (!interface_) return {};
    
    auto devices = interface_->getAvailableDevices();
    connectedDevices_ = devices;
    return devices;
}

MidiDeviceInfo MidiDevice::findDevice(const std::string& namePattern) {
    auto devices = scanForDevices();
    auto it = std::find_if(devices.begin(), devices.end(),
                          [&namePattern](const MidiDeviceInfo& device) {
                              return device.name.find(namePattern) != std::string::npos;
                          });
    
    if (it != devices.end()) {
        return *it;
    }
    return MidiDeviceInfo{-1, "Not Found", "Unknown", false, false, false, 0};
}

MidiDeviceInfo MidiDevice::findMAudioOxygenPro() {
    auto devices = scanForDevices();
    auto it = std::find_if(devices.begin(), devices.end(),
                          [this](const MidiDeviceInfo& device) {
                              return identifyOxygenPro(device);
                          });
    
    if (it != devices.end()) {
        return *it;
    }
    return MidiDeviceInfo{-1, "Oxygen Pro Not Found", "M-Audio", false, false, false, 0};
}

bool MidiDevice::connectToDevice(int deviceId) {
    if (!interface_) return false;
    
    auto device = interface_->getDeviceInfo(deviceId);
    if (device.deviceId == -1) return false;
    
    MidiError error = MidiError::None;
    if (device.isInput) {
        error = interface_->openInputDevice(deviceId);
    }
    if (device.isOutput && error == MidiError::None) {
        error = interface_->openOutputDevice(deviceId);
    }
    
    if (error == MidiError::None) {
        updateDeviceList();
        return true;
    }
    
    lastError_ = error;
    lastErrorString_ = interface_->getErrorString(error);
    return false;
}

bool MidiDevice::connectToDevice(const std::string& deviceName) {
    auto device = findDevice(deviceName);
    if (device.deviceId != -1) {
        return connectToDevice(device.deviceId);
    }
    return false;
}

void MidiDevice::disconnectDevice(int deviceId) {
    if (interface_) {
        interface_->closeDevice(deviceId);
        
        if (deviceId == oxygenProDeviceId_) {
            oxygenProDeviceId_ = -1;
            oxygenProConnected_ = false;
        }
        
        updateDeviceList();
    }
}

void MidiDevice::disconnectAllDevices() {
    if (interface_) {
        interface_->closeAllDevices();
        oxygenProDeviceId_ = -1;
        oxygenProConnected_ = false;
        updateDeviceList();
    }
}

bool MidiDevice::connectToOxygenPro() {
    auto device = findMAudioOxygenPro();
    if (device.deviceId != -1) {
        if (connectToDevice(device.deviceId)) {
            oxygenProDeviceId_ = device.deviceId;
            oxygenProConnected_ = true;
            configureOxygenPro();
            return true;
        }
    }
    return false;
}

bool MidiDevice::isOxygenProConnected() const {
    return oxygenProConnected_;
}

void MidiDevice::configureOxygenPro() {
    if (!oxygenProConnected_) return;
    
    setupOxygenProPads();
    setupOxygenProKnobs();
}

MidiError MidiDevice::sendOxygenProSysEx(const std::vector<uint8_t>& sysexData) {
    if (!oxygenProConnected_ || !interface_) {
        return MidiError::DeviceNotConnected;
    }
    
    return interface_->sendRawMessage(oxygenProDeviceId_, sysexData.data(), sysexData.size());
}

void MidiDevice::setMidiInputCallback(MidiInputCallback callback) {
    inputCallback_ = callback;
}

void MidiDevice::setDeviceConnectionCallback(DeviceConnectionCallback callback) {
    connectionCallback_ = callback;
}

void MidiDevice::startRealTimeProcessing() {
    if (isProcessing_) return;
    
    isProcessing_ = true;
    processingThread_ = std::thread(&MidiDevice::processingThreadFunction, this);
}

void MidiDevice::stopRealTimeProcessing() {
    if (!isProcessing_) return;
    
    isProcessing_ = false;
    if (processingThread_.joinable()) {
        processingThread_.join();
    }
}

bool MidiDevice::isProcessingRealTime() const {
    return isProcessing_;
}

MidiError MidiDevice::sendNoteOn(int deviceId, int channel, int note, int velocity) {
    RealTimeMidiMessage message;
    message.status = 0x90;
    message.data1 = note;
    message.data2 = velocity;
    message.channel = channel;
    message.deviceId = deviceId;
    message.timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    if (interface_) {
        MidiError error = interface_->sendMessage(deviceId, message);
        if (error == MidiError::None) {
            messagesSent_++;
        } else {
            std::lock_guard<std::mutex> lock(errorMutex_);
            lastError_ = error;
            lastErrorString_ = interface_->getErrorString(error);
        }
        return error;
    }
    
    std::lock_guard<std::mutex> lock(errorMutex_);
    lastError_ = MidiError::DeviceNotConnected;
    lastErrorString_ = "Device not connected";
    return MidiError::DeviceNotConnected;
}

MidiError MidiDevice::sendNoteOff(int deviceId, int channel, int note, int velocity) {
    RealTimeMidiMessage message;
    message.status = 0x80;
    message.data1 = note;
    message.data2 = velocity;
    message.channel = channel;
    message.deviceId = deviceId;
    message.timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    if (interface_) {
        MidiError error = interface_->sendMessage(deviceId, message);
        if (error == MidiError::None) {
            messagesSent_++;
        } else {
            std::lock_guard<std::mutex> lock(errorMutex_);
            lastError_ = error;
            lastErrorString_ = interface_->getErrorString(error);
        }
        return error;
    }
    
    std::lock_guard<std::mutex> lock(errorMutex_);
    lastError_ = MidiError::DeviceNotConnected;
    lastErrorString_ = "Device not connected";
    return MidiError::DeviceNotConnected;
}

MidiError MidiDevice::sendControlChange(int deviceId, int channel, int controller, int value) {
    RealTimeMidiMessage message;
    message.status = 0xB0;
    message.data1 = controller;
    message.data2 = value;
    message.channel = channel;
    message.deviceId = deviceId;
    message.timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    if (interface_) {
        MidiError error = interface_->sendMessage(deviceId, message);
        if (error == MidiError::None) {
            messagesSent_++;
        } else {
            std::lock_guard<std::mutex> lock(errorMutex_);
            lastError_ = error;
            lastErrorString_ = interface_->getErrorString(error);
        }
        return error;
    }
    
    std::lock_guard<std::mutex> lock(errorMutex_);
    lastError_ = MidiError::DeviceNotConnected;
    lastErrorString_ = "Device not connected";
    return MidiError::DeviceNotConnected;
}

MidiError MidiDevice::sendProgramChange(int deviceId, int channel, int program) {
    RealTimeMidiMessage message;
    message.status = 0xC0;
    message.data1 = program;
    message.data2 = 0;
    message.channel = channel;
    message.deviceId = deviceId;
    message.timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    if (interface_) {
        MidiError error = interface_->sendMessage(deviceId, message);
        if (error == MidiError::None) {
            messagesSent_++;
        } else {
            std::lock_guard<std::mutex> lock(errorMutex_);
            lastError_ = error;
            lastErrorString_ = interface_->getErrorString(error);
        }
        return error;
    }
    
    std::lock_guard<std::mutex> lock(errorMutex_);
    lastError_ = MidiError::DeviceNotConnected;
    lastErrorString_ = "Device not connected";
    return MidiError::DeviceNotConnected;
}

MidiError MidiDevice::sendPitchBend(int deviceId, int channel, int value) {
    RealTimeMidiMessage message;
    message.status = 0xE0;
    message.data1 = value & 0x7F;
    message.data2 = (value >> 7) & 0x7F;
    message.channel = channel;
    message.deviceId = deviceId;
    message.timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    if (interface_) {
        MidiError error = interface_->sendMessage(deviceId, message);
        if (error == MidiError::None) {
            messagesSent_++;
        } else {
            std::lock_guard<std::mutex> lock(errorMutex_);
            lastError_ = error;
            lastErrorString_ = interface_->getErrorString(error);
        }
        return error;
    }
    
    std::lock_guard<std::mutex> lock(errorMutex_);
    lastError_ = MidiError::DeviceNotConnected;
    lastErrorString_ = "Device not connected";
    return MidiError::DeviceNotConnected;
}

MidiError MidiDevice::sendKeyEvent(int deviceId, const KeyEvent& keyEvent) {
    if (keyEvent.state == KeyState::KeyDown) {
        return sendNoteOn(deviceId, keyEvent.channel, keyEvent.note, keyEvent.velocity);
    } else {
        return sendNoteOff(deviceId, keyEvent.channel, keyEvent.note, 64);
    }
}

std::vector<KeyEvent> MidiDevice::getRecentKeyEvents(double timeWindow) {
    std::lock_guard<std::mutex> lock(keyEventHistoryMutex_);
    std::vector<KeyEvent> recentEvents;
    
    double currentTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    double cutoffTime = currentTime - timeWindow;
    
    for (const auto& event : keyEventHistory_) {
        if (event.timestamp >= cutoffTime) {
            recentEvents.push_back(event);
        }
    }
    
    return recentEvents;
}

void MidiDevice::clearKeyEventHistory() {
    std::lock_guard<std::mutex> lock(keyEventHistoryMutex_);
    keyEventHistory_.clear();
}

// Static utility methods
RealTimeMidiMessage MidiDevice::parseRawMidiMessage(const uint8_t* data, size_t length, double timestamp, int deviceId) {
    RealTimeMidiMessage message;
    message.timestamp = timestamp;
    message.deviceId = deviceId;
    
    if (length >= 1) {
        message.status = data[0] & 0xF0;
        message.channel = (data[0] & 0x0F) + 1;
        message.data1 = length > 1 ? data[1] : 0;
        message.data2 = length > 2 ? data[2] : 0;
    }
    
    return message;
}

std::vector<uint8_t> MidiDevice::serializeMidiMessage(const RealTimeMidiMessage& message) {
    std::vector<uint8_t> data;
    data.push_back(message.status | (message.channel - 1));
    
    uint8_t messageType = message.status & 0xF0;
    switch (messageType) {
        case 0x80: // Note Off
        case 0x90: // Note On
        case 0xA0: // Polyphonic Aftertouch
        case 0xB0: // Control Change
        case 0xE0: // Pitch Bend
            data.push_back(message.data1);
            data.push_back(message.data2);
            break;
        case 0xC0: // Program Change
        case 0xD0: // Channel Aftertouch
            data.push_back(message.data1);
            break;
    }
    
    return data;
}

bool MidiDevice::isNoteOnMessage(const RealTimeMidiMessage& message) {
    return (message.status & 0xF0) == 0x90 && message.data2 > 0;
}

bool MidiDevice::isNoteOffMessage(const RealTimeMidiMessage& message) {
    return ((message.status & 0xF0) == 0x80) || 
           ((message.status & 0xF0) == 0x90 && message.data2 == 0);
}

bool MidiDevice::isDrumPadMessage(const RealTimeMidiMessage& message) {
    return message.channel == 10 && 
           (isNoteOnMessage(message) || isNoteOffMessage(message)) &&
           message.data1 >= 36 && message.data1 <= 51;
}

DeviceType MidiDevice::getDeviceTypeFromMessage(const RealTimeMidiMessage& message) {
    if (isDrumPadMessage(message)) {
        return DeviceType::DrumPad;
    }
    return DeviceType::Piano;
}

// Statistics methods
uint64_t MidiDevice::getMessagesReceived() const {
    return messagesReceived_;
}

uint64_t MidiDevice::getMessagesSent() const {
    return messagesSent_;
}

uint64_t MidiDevice::getDroppedMessages() const {
    return droppedMessages_;
}

double MidiDevice::getInputLatency() const {
    return avgInputLatency_;
}

double MidiDevice::getOutputLatency() const {
    return avgOutputLatency_;
}

void MidiDevice::resetStatistics() {
    messagesReceived_ = 0;
    messagesSent_ = 0;
    droppedMessages_ = 0;
    avgInputLatency_ = 0.0;
    avgOutputLatency_ = 0.0;
}

// Configuration methods
void MidiDevice::setBufferSize(size_t bufferSize) {
    bufferSize_ = bufferSize;
}

void MidiDevice::setLatencyTarget(double milliseconds) {
    latencyTarget_ = milliseconds;
}

void MidiDevice::enableVelocityCurve(bool enabled) {
    velocityCurveEnabled_ = enabled;
}

void MidiDevice::setVelocityCurve(const std::vector<float>& curve) {
    velocityCurve_ = curve;
    velocityCurveEnabled_ = !curve.empty();
}

// Error handling
std::string MidiDevice::getLastErrorString() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(errorMutex_));
    return lastErrorString_;
}

MidiError MidiDevice::getLastError() const {
    return lastError_;
}

void MidiDevice::clearErrors() {
    std::lock_guard<std::mutex> lock(errorMutex_);
    lastError_ = MidiError::None;
    lastErrorString_.clear();
}

// Private methods
void MidiDevice::processingThreadFunction() {
    while (isProcessing_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        // Process queued messages
        std::lock_guard<std::mutex> lock(messageQueueMutex_);
        while (!messageQueue_.empty()) {
            auto message = messageQueue_.front();
            messageQueue_.pop();
            
            if (inputCallback_) {
                inputCallback_(message);
            }
        }
    }
}

void MidiDevice::handleMidiMessage(const RealTimeMidiMessage& message) {
    messagesReceived_++;
    
    if (shouldProcessMessage(message)) {
        updateLatencyStatistics(message);
        
        // Convert to key event and store
        KeyEvent keyEvent = convertMidiToKeyEvent(message);
        
        std::lock_guard<std::mutex> lock(keyEventHistoryMutex_);
        keyEventHistory_.push_back(keyEvent);
        
        // Limit history size
        if (keyEventHistory_.size() > MAX_KEY_EVENT_HISTORY) {
            keyEventHistory_.erase(keyEventHistory_.begin());
        }
        
        // Queue for processing
        std::lock_guard<std::mutex> queueLock(messageQueueMutex_);
        if (messageQueue_.size() < bufferSize_) {
            messageQueue_.push(message);
        } else {
            droppedMessages_++;
        }
    }
}

void MidiDevice::handleDeviceConnection(const MidiDeviceInfo& device, bool connected) {
    if (connectionCallback_) {
        connectionCallback_(device, connected);
    }
    
    // Check if it's the Oxygen Pro
    if (identifyOxygenPro(device)) {
        oxygenProConnected_ = connected;
        if (!connected) {
            oxygenProDeviceId_ = -1;
        }
    }
}

void MidiDevice::updateDeviceList() {
    connectedDevices_ = scanForDevices();
}

bool MidiDevice::identifyOxygenPro(const MidiDeviceInfo& device) {
    return device.name.find("Oxygen Pro") != std::string::npos ||
           device.name.find("M-Audio") != std::string::npos ||
           device.manufacturer.find("M-Audio") != std::string::npos;
}

void MidiDevice::setupOxygenProPads() {
    if (!oxygenProConnected_) return;
    
    // Send SysEx to configure pads for drum mode
    std::vector<uint8_t> padConfig = {
        0xF0, 0x47, 0x00, 0x7F, 0x4A, 0x61, 0x00, 0x01, 0xF7
    };
    sendOxygenProSysEx(padConfig);
}

void MidiDevice::setupOxygenProKnobs() {
    if (!oxygenProConnected_) return;
    
    // Configure knobs for control changes
    for (int knob = 0; knob < 8; ++knob) {
        sendControlChange(oxygenProDeviceId_, 1, 70 + knob, 64);
    }
}

KeyEvent MidiDevice::convertMidiToKeyEvent(const RealTimeMidiMessage& message) {
    KeyEvent keyEvent;
    keyEvent.device = getDeviceTypeFromMessage(message);
    keyEvent.state = isNoteOnMessage(message) ? KeyState::KeyDown : KeyState::KeyUp;
    keyEvent.note = message.data1;
    keyEvent.velocity = message.data2;
    keyEvent.channel = message.channel;
    keyEvent.timestamp = message.timestamp;
    
    return keyEvent;
}

bool MidiDevice::shouldProcessMessage(const RealTimeMidiMessage& message) {
    // Filter out system messages and focus on note/control messages
    uint8_t messageType = message.status & 0xF0;
    return messageType >= 0x80 && messageType <= 0xE0;
}

void MidiDevice::updateLatencyStatistics(const RealTimeMidiMessage& message) {
    double currentTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    double latency = (currentTime - message.timestamp) * 1000.0; // Convert to milliseconds
    
    // Simple moving average
    avgInputLatency_ = (avgInputLatency_ * 0.9) + (latency * 0.1);
    lastMessageTime_ = std::chrono::high_resolution_clock::now();
}

// Factory implementation
std::unique_ptr<MidiDevice> MidiDeviceFactory::createDevice() {
    return std::make_unique<MidiDevice>();
}

std::vector<std::string> MidiDeviceFactory::getSupportedPlatforms() {
    return {"Windows", "macOS", "Linux", "Generic"};
}

bool MidiDeviceFactory::isPlatformSupported() {
#if defined(_WIN32) || defined(__APPLE__) || defined(__linux__)
    return true;
#else
    return false;
#endif
}

std::string MidiDeviceFactory::getCurrentPlatform() {
#ifdef _WIN32
    return "Windows";
#elif __APPLE__
    return "macOS";
#elif __linux__
    return "Linux";
#else
    return "Generic";
#endif
}

// Discovery implementation
std::vector<MidiDeviceInfo> MidiDeviceDiscovery::findAllDevices() {
    auto midiDevice = MidiDeviceFactory::createDevice();
    if (midiDevice->initialize()) {
        return midiDevice->scanForDevices();
    }
    return {};
}

std::vector<MidiDeviceInfo> MidiDeviceDiscovery::findControllerKeyboards() {
    auto devices = findAllDevices();
    std::vector<MidiDeviceInfo> controllers;
    
    for (const auto& device : devices) {
        if (isPianoKeyboard(device)) {
            controllers.push_back(device);
        }
    }
    
    return controllers;
}

std::vector<MidiDeviceInfo> MidiDeviceDiscovery::findSynthesizers() {
    auto devices = findAllDevices();
    std::vector<MidiDeviceInfo> synths;
    
    for (const auto& device : devices) {
        if (device.isOutput && !isPianoKeyboard(device)) {
            synths.push_back(device);
        }
    }
    
    return synths;
}

MidiDeviceInfo MidiDeviceDiscovery::findBestPianoController() {
    auto keyboards = findControllerKeyboards();
    if (keyboards.empty()) {
        return MidiDeviceInfo{-1, "No Piano Controller", "Unknown", false, false, false, 0};
    }
    
    // Score keyboards and return the best one
    int bestScore = -1;
    MidiDeviceInfo bestDevice = keyboards[0];
    
    for (const auto& device : keyboards) {
        int score = scoreDeviceForPiano(device);
        if (score > bestScore) {
            bestScore = score;
            bestDevice = device;
        }
    }
    
    return bestDevice;
}

bool MidiDeviceDiscovery::isMAudioDevice(const MidiDeviceInfo& device) {
    return device.name.find("M-Audio") != std::string::npos ||
           device.manufacturer.find("M-Audio") != std::string::npos ||
           device.name.find("Oxygen") != std::string::npos;
}

bool MidiDeviceDiscovery::isPianoKeyboard(const MidiDeviceInfo& device) {
    std::vector<std::string> pianoKeywords = {
        "Piano", "Keyboard", "Controller", "Keys", "Oxygen", "KeyStation", "MPK"
    };
    
    for (const auto& keyword : pianoKeywords) {
        if (device.name.find(keyword) != std::string::npos) {
            return true;
        }
    }
    
    return device.isInput && device.portCount > 0;
}

int MidiDeviceDiscovery::scoreDeviceForPiano(const MidiDeviceInfo& device) {
    int score = 0;
    
    // Prefer input devices
    if (device.isInput) score += 10;
    
    // Prefer connected devices
    if (device.isConnected) score += 20;
    
    // Prefer M-Audio devices (especially Oxygen Pro)
    if (isMAudioDevice(device)) score += 30;
    if (device.name.find("Oxygen Pro") != std::string::npos) score += 20;
    
    // Prefer devices with "Piano" or "Keyboard" in name
    if (device.name.find("Piano") != std::string::npos) score += 15;
    if (device.name.find("Keyboard") != std::string::npos) score += 10;
    
    // Prefer devices with multiple ports
    score += device.portCount * 5;
    
    return score;
}