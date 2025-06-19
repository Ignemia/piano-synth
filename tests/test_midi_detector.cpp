#include <gtest/gtest.h>
#include "input/midi_detector.h"
#include <memory>
#include <algorithm>

using namespace PianoSynth::Input;

class MidiDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        detector_ = std::make_unique<MidiDetector>();
    }

    void TearDown() override {
        detector_.reset();
    }

    std::unique_ptr<MidiDetector> detector_;
};

// Test device pattern matching
TEST_F(MidiDetectorTest, DevicePatternMatching) {
    // Test piano device patterns
    EXPECT_TRUE(detector_->isPianoDevice("M-AUDIO Oxygen Pro 61"));
    EXPECT_TRUE(detector_->isPianoDevice("oxygen pro"));
    EXPECT_TRUE(detector_->isPianoDevice("Digital Piano"));
    EXPECT_TRUE(detector_->isPianoDevice("KEYBOARD"));
    EXPECT_TRUE(detector_->isPianoDevice("m-audio"));
    
    // Test case insensitivity
    EXPECT_TRUE(detector_->isPianoDevice("OXYGEN PRO 61"));
    EXPECT_TRUE(detector_->isPianoDevice("piano"));
    
    // Test non-piano devices
    EXPECT_FALSE(detector_->isPianoDevice("Audio Interface"));
    EXPECT_FALSE(detector_->isPianoDevice("Microphone"));
    EXPECT_FALSE(detector_->isPianoDevice(""));
    
    // Test drum device patterns
    EXPECT_TRUE(detector_->isDrumDevice("Drum Kit"));
    EXPECT_TRUE(detector_->isDrumDevice("percussion"));
    EXPECT_TRUE(detector_->isDrumDevice("Electronic Pad"));
    EXPECT_TRUE(detector_->isDrumDevice("TRIGGER"));
    
    // Test controller device patterns
    EXPECT_TRUE(detector_->isControllerDevice("MIDI Controller"));
    EXPECT_TRUE(detector_->isControllerDevice("mixer"));
    EXPECT_TRUE(detector_->isControllerDevice("Fader Box"));
    EXPECT_TRUE(detector_->isControllerDevice("knob"));
}

// Test device detection (this test may not work without actual MIDI devices)
TEST_F(MidiDetectorTest, DeviceDetection) {
    auto devices = detector_->detectDevices();
    
    // We can't assume any specific devices are connected,
    // but the function should not crash and should return a vector
    EXPECT_NO_THROW(devices = detector_->detectDevices());
    
    // Each detected device should have proper categorization
    for (const auto& device : devices) {
        EXPECT_FALSE(device.name.empty());
        
        // A device can be multiple types, but at least one categorization
        // should make sense based on the name
        bool properly_categorized = true;
        
        // If it has "piano" in the name, it should be detected as piano
        std::string lower_name = device.name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        
        if (lower_name.find("piano") != std::string::npos) {
            EXPECT_TRUE(device.is_piano) << "Device '" << device.name << "' contains 'piano' but not detected as piano";
        }
        
        if (lower_name.find("drum") != std::string::npos) {
            EXPECT_TRUE(device.is_drum) << "Device '" << device.name << "' contains 'drum' but not detected as drum";
        }
    }
}

// Test device opening and closing (mock test)
TEST_F(MidiDetectorTest, DeviceOpenClose) {
    // Initially, no device should be open
    EXPECT_FALSE(detector_->isDeviceOpen());
    
    // Try to open an invalid port (should fail gracefully)
    EXPECT_FALSE(detector_->openDevice(999));
    EXPECT_FALSE(detector_->isDeviceOpen());
    
    // Test closing when no device is open
    EXPECT_NO_THROW(detector_->closeDevice());
    
    // If there are available devices, test opening the first one
    auto devices = detector_->detectDevices();
    if (!devices.empty()) {
        bool opened = detector_->openDevice(devices[0].port);
        if (opened) {
            EXPECT_TRUE(detector_->isDeviceOpen());
            
            // Test closing
            detector_->closeDevice();
            EXPECT_FALSE(detector_->isDeviceOpen());
        }
    }
}

// Test edge cases and error handling
TEST_F(MidiDetectorTest, EdgeCasesAndErrorHandling) {
    // Test with empty device names
    EXPECT_FALSE(detector_->isPianoDevice(""));
    EXPECT_FALSE(detector_->isDrumDevice(""));
    EXPECT_FALSE(detector_->isControllerDevice(""));
    
    // Test with whitespace
    EXPECT_FALSE(detector_->isPianoDevice("   "));
    EXPECT_FALSE(detector_->isDrumDevice("\t\n"));
    
    // Test with special characters
    EXPECT_FALSE(detector_->isPianoDevice("!@#$%^&*()"));
    
    // Test very long device names
    std::string long_name(1000, 'x');
    EXPECT_FALSE(detector_->isPianoDevice(long_name));
    
    // Test device name with piano pattern in the middle
    EXPECT_TRUE(detector_->isPianoDevice("USB MIDI Keyboard Controller"));
    EXPECT_TRUE(detector_->isPianoDevice("Roland Digital Piano FP-30"));
}

// Test multiple device categorization
TEST_F(MidiDetectorTest, MultipleDeviceCategorization) {
    // Some devices might fit multiple categories
    std::string multi_device = "M-AUDIO Oxygen Pro 61 MIDI Controller";
    
    // This device should be both piano and controller
    EXPECT_TRUE(detector_->isPianoDevice(multi_device));
    EXPECT_TRUE(detector_->isControllerDevice(multi_device));
    
    // But not drum
    EXPECT_FALSE(detector_->isDrumDevice(multi_device));
    
    // Test a device that could be drum and controller
    std::string drum_controller = "Electronic Drum Controller";
    EXPECT_TRUE(detector_->isDrumDevice(drum_controller));
    EXPECT_TRUE(detector_->isControllerDevice(drum_controller));
    EXPECT_FALSE(detector_->isPianoDevice(drum_controller));
}

// Test pattern matching specificity
TEST_F(MidiDetectorTest, PatternMatchingSpecificity) {
    // Test that partial matches work correctly
    EXPECT_TRUE(detector_->isPianoDevice("Yamaha P-125 Digital Piano"));
    EXPECT_TRUE(detector_->isPianoDevice("Casio CDP-135 keyboard"));
    
    // Test that we don't get false positives
    EXPECT_FALSE(detector_->isPianoDevice("Audio Keyboard (Computer)"));
    EXPECT_FALSE(detector_->isDrumDevice("Bass Guitar"));
    
    // Test the M-AUDIO Oxygen Pro 61 specifically (the target device)
    EXPECT_TRUE(detector_->isPianoDevice("M-AUDIO Oxygen Pro 61"));
    EXPECT_TRUE(detector_->isPianoDevice("M-Audio Oxygen Pro 61"));
    EXPECT_TRUE(detector_->isPianoDevice("MAUDIO OXYGEN PRO 61"));
    EXPECT_TRUE(detector_->isPianoDevice("Oxygen Pro 61"));
    EXPECT_TRUE(detector_->isPianoDevice("oxygen pro"));
}
