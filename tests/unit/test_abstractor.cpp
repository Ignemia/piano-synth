#include "../../include/Abstractor.h"
#include "../../include/MidiInput.h"
#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

/**
 * @brief [AI GENERATED] Comprehensive unit tests for Abstractor class.
 */

class AbstractorTest {
private:
    Abstractor abstractor;
    int testCount = 0;
    int passedTests = 0;
    static constexpr double EPSILON = 1e-6;

public:
    void runAllTests() {
        std::cout << "Running Abstractor unit tests...\n";
        
        // Test MIDI message conversion
        testBasicMidiConversion();
        testMidiFrequencyCalculation();
        testMidiVelocityHandling();
        testEmptyMidiInput();
        
        // Test key event conversion
        testBasicKeyEventConversion();
        testKeyEventMatching();
        testDeviceTypeHandling();
        testChannelSeparation();
        testOverlappingNotes();
        testUnmatchedKeyEvents();
        testPendingKeysHandling();
        testDrumPadDuration();
        
        // Test edge cases
        testEdgeCases();
        testFrequencyAccuracy();
        
        std::cout << "\nAbstractor Tests: " << passedTests << "/" << testCount << " passed\n";
        if (passedTests != testCount) {
            throw std::runtime_error("Some Abstractor tests failed");
        }
    }

private:
    void assert_test(bool condition, const std::string& testName) {
        testCount++;
        if (condition) {
            passedTests++;
            std::cout << "✓ " << testName << "\n";
        } else {
            std::cout << "✗ " << testName << " FAILED\n";
        }
    }
    
    bool isNearlyEqual(double a, double b, double epsilon = EPSILON) {
        return std::abs(a - b) < epsilon;
    }
    
    void testBasicMidiConversion() {
        std::vector<MidiMessage> midiMessages = {
            {60, 1.0, 0.0},  // C4, 1 second, start at 0
            {64, 0.5, 1.0},  // E4, 0.5 seconds, start at 1.0
            {67, 2.0, 1.5}   // G4, 2 seconds, start at 1.5
        };
        
        auto noteEvents = abstractor.convert(midiMessages);
        
        assert_test(noteEvents.size() == 3, "Basic MIDI conversion produces correct count");
        assert_test(noteEvents[0].duration == 1.0, "First note duration preserved");
        assert_test(noteEvents[1].startTime == 1.0, "Second note start time preserved");
        assert_test(noteEvents[2].duration == 2.0, "Third note duration preserved");
        assert_test(noteEvents[0].velocity == 0.7, "Default velocity applied");
    }
    
    void testMidiFrequencyCalculation() {
        std::vector<MidiMessage> midiMessages = {
            {69, 1.0, 0.0},  // A4 (440 Hz reference)
            {57, 1.0, 0.0},  // A3 (220 Hz)
            {81, 1.0, 0.0},  // A5 (880 Hz)
            {60, 1.0, 0.0}   // C4 (261.63 Hz)
        };
        
        auto noteEvents = abstractor.convert(midiMessages);
        
        assert_test(isNearlyEqual(noteEvents[0].frequency, 440.0), "A4 frequency correct (440 Hz)");
        assert_test(isNearlyEqual(noteEvents[1].frequency, 220.0), "A3 frequency correct (220 Hz)");
        assert_test(isNearlyEqual(noteEvents[2].frequency, 880.0), "A5 frequency correct (880 Hz)");
        assert_test(isNearlyEqual(noteEvents[3].frequency, 261.626, 0.01), "C4 frequency correct");
    }
    
    void testMidiVelocityHandling() {
        std::vector<MidiMessage> midiMessages = {
            {60, 1.0, 0.0}
        };
        
        auto noteEvents = abstractor.convert(midiMessages);
        
        assert_test(noteEvents[0].velocity >= 0.0 && noteEvents[0].velocity <= 1.0, 
                   "MIDI velocity in valid range");
        assert_test(noteEvents[0].velocity == 0.7, "Default velocity applied correctly");
    }
    
    void testEmptyMidiInput() {
        std::vector<MidiMessage> emptyMidi;
        auto noteEvents = abstractor.convert(emptyMidi);
        
        assert_test(noteEvents.empty(), "Empty MIDI input produces empty output");
    }
    
    void testBasicKeyEventConversion() {
        std::vector<KeyEvent> keyEvents = {
            {DeviceType::Piano, KeyState::KeyDown, 60, 100, 1, 0.0},
            {DeviceType::Piano, KeyState::KeyUp, 60, 0, 1, 1.0}
        };
        
        auto noteEvents = abstractor.convertKeyEvents(keyEvents);
        
        assert_test(noteEvents.size() == 1, "Key event pair produces one note");
        assert_test(noteEvents[0].duration == 1.0, "Duration calculated correctly");
        assert_test(isNearlyEqual(noteEvents[0].velocity, 100.0/127.0), "Velocity converted correctly");
        assert_test(noteEvents[0].startTime == 0.0, "Start time preserved");
    }
    
    void testKeyEventMatching() {
        std::vector<KeyEvent> keyEvents = {
            {DeviceType::Piano, KeyState::KeyDown, 60, 100, 1, 0.0},
            {DeviceType::Piano, KeyState::KeyDown, 64, 110, 1, 0.5},
            {DeviceType::Piano, KeyState::KeyUp, 60, 0, 1, 1.0},
            {DeviceType::Piano, KeyState::KeyUp, 64, 0, 1, 1.5}
        };
        
        auto noteEvents = abstractor.convertKeyEvents(keyEvents);
        
        assert_test(noteEvents.size() == 2, "Multiple key pairs produce multiple notes");
        
        // Find the C and E notes
        NoteEvent* cNote = nullptr;
        NoteEvent* eNote = nullptr;
        for (auto& note : noteEvents) {
            if (isNearlyEqual(note.frequency, 261.626, 0.01)) cNote = &note;
            if (isNearlyEqual(note.frequency, 329.628, 0.01)) eNote = &note;
        }
        
        assert_test(cNote != nullptr, "C note found in output");
        assert_test(eNote != nullptr, "E note found in output");
        assert_test(cNote->duration == 1.0, "C note duration correct");
        assert_test(eNote->duration == 1.0, "E note duration correct");
    }
    
    void testDeviceTypeHandling() {
        std::vector<KeyEvent> keyEvents = {
            {DeviceType::Piano, KeyState::KeyDown, 60, 100, 1, 0.0},
            {DeviceType::DrumPad, KeyState::KeyDown, 36, 120, 10, 0.0},
            {DeviceType::Piano, KeyState::KeyUp, 60, 0, 1, 1.0},
            {DeviceType::DrumPad, KeyState::KeyUp, 36, 0, 10, 0.2}
        };
        
        auto noteEvents = abstractor.convertKeyEvents(keyEvents);
        
        assert_test(noteEvents.size() == 2, "Piano and drum events both processed");
        
        // Find piano and drum notes
        bool foundPianoNote = false;
        bool foundDrumNote = false;
        for (const auto& note : noteEvents) {
            if (note.duration == 1.0) foundPianoNote = true;
            if (note.duration == 0.2) foundDrumNote = true;
        }
        
        assert_test(foundPianoNote, "Piano note processed correctly");
        assert_test(foundDrumNote, "Drum note processed correctly");
    }
    
    void testChannelSeparation() {
        std::vector<KeyEvent> keyEvents = {
            {DeviceType::Piano, KeyState::KeyDown, 60, 100, 1, 0.0},
            {DeviceType::Piano, KeyState::KeyDown, 60, 110, 2, 0.0}, // Same note, different channel
            {DeviceType::Piano, KeyState::KeyUp, 60, 0, 1, 1.0},
            {DeviceType::Piano, KeyState::KeyUp, 60, 0, 2, 1.5}
        };
        
        auto noteEvents = abstractor.convertKeyEvents(keyEvents);
        
        assert_test(noteEvents.size() == 2, "Same note on different channels creates separate events");
        assert_test(noteEvents[0].duration == 1.0, "Channel 1 note duration correct");
        assert_test(noteEvents[1].duration == 1.5, "Channel 2 note duration correct");
    }
    
    void testOverlappingNotes() {
        std::vector<KeyEvent> keyEvents = {
            {DeviceType::Piano, KeyState::KeyDown, 60, 100, 1, 0.0},
            {DeviceType::Piano, KeyState::KeyDown, 64, 110, 1, 0.5},
            {DeviceType::Piano, KeyState::KeyUp, 64, 0, 1, 1.0},
            {DeviceType::Piano, KeyState::KeyUp, 60, 0, 1, 2.0}
        };
        
        auto noteEvents = abstractor.convertKeyEvents(keyEvents);
        
        assert_test(noteEvents.size() == 2, "Overlapping notes both processed");
        
        // Verify durations
        bool foundShortNote = false;
        bool foundLongNote = false;
        for (const auto& note : noteEvents) {
            if (note.duration == 0.5) foundShortNote = true;
            if (note.duration == 2.0) foundLongNote = true;
        }
        
        assert_test(foundShortNote, "Short overlapping note processed");
        assert_test(foundLongNote, "Long overlapping note processed");
    }
    
    void testUnmatchedKeyEvents() {
        std::vector<KeyEvent> keyEvents = {
            {DeviceType::Piano, KeyState::KeyDown, 60, 100, 1, 0.0},
            {DeviceType::Piano, KeyState::KeyUp, 64, 0, 1, 1.0}  // Mismatched note
        };
        
        auto noteEvents = abstractor.convertKeyEvents(keyEvents);
        
        assert_test(noteEvents.size() == 1, "Unmatched key up ignored, pending key processed");
        assert_test(noteEvents[0].duration == 1.0, "Pending key gets default duration");
    }
    
    void testPendingKeysHandling() {
        std::vector<KeyEvent> keyEvents = {
            {DeviceType::Piano, KeyState::KeyDown, 60, 100, 1, 0.0},
            {DeviceType::Piano, KeyState::KeyDown, 64, 110, 1, 0.5}
            // No key up events - both should be handled as pending
        };
        
        auto noteEvents = abstractor.convertKeyEvents(keyEvents);
        
        assert_test(noteEvents.size() == 2, "Pending keys processed");
        
        bool foundPendingNote = false;
        for (const auto& note : noteEvents) {
            if (note.duration == 1.0) foundPendingNote = true; // Default duration for piano
        }
        assert_test(foundPendingNote, "Pending keys get default duration");
    }
    
    void testDrumPadDuration() {
        std::vector<KeyEvent> keyEvents = {
            {DeviceType::DrumPad, KeyState::KeyDown, 36, 120, 10, 0.0},
            {DeviceType::DrumPad, KeyState::KeyUp, 36, 0, 10, 5.0}  // Very long duration
        };
        
        auto noteEvents = abstractor.convertKeyEvents(keyEvents);
        
        assert_test(noteEvents.size() == 1, "Drum event processed");
        assert_test(noteEvents[0].duration == 0.2, "Long drum duration clamped to short hit");
        
        // Test pending drum pad
        std::vector<KeyEvent> pendingDrumEvents = {
            {DeviceType::DrumPad, KeyState::KeyDown, 38, 100, 10, 0.0}
            // No key up - should get short default duration
        };
        
        auto pendingNotes = abstractor.convertKeyEvents(pendingDrumEvents);
        assert_test(pendingNotes.size() == 1, "Pending drum processed");
        assert_test(pendingNotes[0].duration == 0.2, "Pending drum gets short duration");
    }
    
    void testEdgeCases() {
        // Empty key events
        std::vector<KeyEvent> emptyKeys;
        auto emptyNotes = abstractor.convertKeyEvents(emptyKeys);
        assert_test(emptyNotes.empty(), "Empty key events produce empty output");
        
        // Key up without key down
        std::vector<KeyEvent> orphanKeyUp = {
            {DeviceType::Piano, KeyState::KeyUp, 60, 0, 1, 1.0}
        };
        auto orphanNotes = abstractor.convertKeyEvents(orphanKeyUp);
        assert_test(orphanNotes.empty(), "Orphan key up produces no output");
        
        // Zero duration
        std::vector<KeyEvent> zeroDuration = {
            {DeviceType::Piano, KeyState::KeyDown, 60, 100, 1, 1.0},
            {DeviceType::Piano, KeyState::KeyUp, 60, 0, 1, 1.0}  // Same timestamp
        };
        auto zeroNotes = abstractor.convertKeyEvents(zeroDuration);
        assert_test(zeroNotes.size() == 1, "Zero duration event processed");
        assert_test(zeroNotes[0].duration == 0.0, "Zero duration preserved");
        
        // Negative duration (key up before key down - shouldn't happen but test anyway)
        std::vector<KeyEvent> negativeDuration = {
            {DeviceType::Piano, KeyState::KeyDown, 60, 100, 1, 2.0},
            {DeviceType::Piano, KeyState::KeyUp, 60, 0, 1, 1.0}
        };
        auto negativeNotes = abstractor.convertKeyEvents(negativeDuration);
        assert_test(negativeNotes.size() == 1, "Negative duration event processed");
        assert_test(negativeNotes[0].duration == -1.0, "Negative duration calculated correctly");
    }
    
    void testFrequencyAccuracy() {
        // Test frequency calculation for various MIDI notes
        struct MidiFreqPair {
            int midiNote;
            double expectedFreq;
        };
        
        std::vector<MidiFreqPair> testCases = {
            {21, 27.5},      // A0
            {33, 55.0},      // A1  
            {45, 110.0},     // A2
            {57, 220.0},     // A3
            {69, 440.0},     // A4 (reference)
            {81, 880.0},     // A5
            {93, 1760.0},    // A6
            {108, 4186.01}   // C8
        };
        
        for (const auto& testCase : testCases) {
            std::vector<MidiMessage> midiMessages = {{testCase.midiNote, 1.0, 0.0}};
            auto noteEvents = abstractor.convert(midiMessages);
            
            bool freqCorrect = isNearlyEqual(noteEvents[0].frequency, testCase.expectedFreq, 0.1);
            assert_test(freqCorrect, "MIDI note " + std::to_string(testCase.midiNote) + 
                       " frequency accuracy");
        }
        
        // Test semitone relationships
        std::vector<MidiMessage> semitones = {{60, 1.0, 0.0}, {61, 1.0, 0.0}}; // C4, C#4
        auto semitoneNotes = abstractor.convert(semitones);
        
        double ratio = semitoneNotes[1].frequency / semitoneNotes[0].frequency;
        bool semitoneRatioCorrect = isNearlyEqual(ratio, std::pow(2.0, 1.0/12.0), 0.001);
        assert_test(semitoneRatioCorrect, "Semitone frequency ratio correct");
        
        // Test octave relationships
        std::vector<MidiMessage> octaves = {{60, 1.0, 0.0}, {72, 1.0, 0.0}}; // C4, C5
        auto octaveNotes = abstractor.convert(octaves);
        
        double octaveRatio = octaveNotes[1].frequency / octaveNotes[0].frequency;
        bool octaveRatioCorrect = isNearlyEqual(octaveRatio, 2.0, 0.001);
        assert_test(octaveRatioCorrect, "Octave frequency ratio correct");
    }
};

int main() {
    try {
        AbstractorTest test;
        test.runAllTests();
        std::cout << "All Abstractor unit tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }
}