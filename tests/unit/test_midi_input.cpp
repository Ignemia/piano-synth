#include "../../include/MidiInput.h"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <set>

/**
 * @brief [AI GENERATED] Comprehensive unit tests for MidiInput class.
 */

class MidiInputTest {
private:
    MidiInput midi;
    int testCount = 0;
    int passedTests = 0;

public:
    void runAllTests() {
        std::cout << "Running MidiInput unit tests...\n";
        
        // Test MIDI message generation
        testRushEGeneration();
        testFurEliseGeneration();
        testBeethoven5thGeneration();
        testHallOfMountainKingGeneration();
        testVivaldiSpringGeneration();
        
        // Test key event generation
        testRushEKeysGeneration();
        testFurEliseKeysGeneration();
        testBeethoven5thKeysGeneration();
        testHallOfMountainKingKeysGeneration();
        testVivaldiSpringKeysGeneration();
        
        // Test drum functionality
        testDrumPatternGeneration();
        testMixedPerformanceGeneration();
        testDrumMapping();
        
        // Test helper functions
        testHelperFunctions();
        
        // Test edge cases
        testEdgeCases();
        
        std::cout << "\nMidiInput Tests: " << passedTests << "/" << testCount << " passed\n";
        if (passedTests != testCount) {
            throw std::runtime_error("Some MidiInput tests failed");
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
    
    void testRushEGeneration() {
        auto messages = midi.generateRushE();
        
        assert_test(!messages.empty(), "Rush E generates messages");
        assert_test(messages.size() > 50, "Rush E has sufficient complexity");
        assert_test(messages[0].startTime == 0.0, "Rush E starts at time 0");
        
        // Check for rapid E note patterns characteristic of Rush E
        int eNoteCount = 0;
        for (const auto& msg : messages) {
            if (msg.note == 64 || msg.note == 76 || msg.note == 52) { // E notes in different octaves
                eNoteCount++;
            }
        }
        assert_test(eNoteCount >= 10, "Rush E contains multiple E notes");
        
        // Verify note range spans keyboard
        int minNote = 128, maxNote = 0;
        for (const auto& msg : messages) {
            minNote = std::min(minNote, msg.note);
            maxNote = std::max(maxNote, msg.note);
        }
        assert_test(maxNote - minNote > 24, "Rush E spans wide note range");
    }
    
    void testFurEliseGeneration() {
        auto messages = midi.generateFurElise();
        
        assert_test(!messages.empty(), "Fur Elise generates messages");
        assert_test(messages[0].startTime == 0.0, "Fur Elise starts at time 0");
        
        // Check for the famous opening E-D#-E pattern
        bool hasOpeningPattern = false;
        if (messages.size() >= 3) {
            hasOpeningPattern = (messages[0].note == 76 && // E5
                               messages[1].note == 75 && // D#5
                               messages[2].note == 76);  // E5
        }
        assert_test(hasOpeningPattern, "Fur Elise has correct opening pattern");
        
        // Verify it has both melody and bass notes
        bool hasBass = false, hasTreble = false;
        for (const auto& msg : messages) {
            if (msg.note < 60) hasBass = true;
            if (msg.note > 72) hasTreble = true;
        }
        assert_test(hasBass && hasTreble, "Fur Elise has both bass and treble");
    }
    
    void testBeethoven5thGeneration() {
        auto messages = midi.generateBeethoven5th();
        
        assert_test(!messages.empty(), "Beethoven 5th generates messages");
        
        // Check for the famous G-G-G-Eb pattern
        bool hasFamousMotif = false;
        if (messages.size() >= 4) {
            hasFamousMotif = (messages[0].note == 55 && // G3
                            messages[1].note == 55 && // G3
                            messages[2].note == 55 && // G3
                            messages[3].note == 51);  // Eb3
        }
        assert_test(hasFamousMotif, "Beethoven 5th has correct opening motif");
        
        // Verify dramatic range and long notes
        bool hasLongNotes = false;
        for (const auto& msg : messages) {
            if (msg.duration >= 1.0) hasLongNotes = true;
        }
        assert_test(hasLongNotes, "Beethoven 5th has dramatic long notes");
    }
    
    void testHallOfMountainKingGeneration() {
        auto messages = midi.generateHallOfMountainKing();
        
        assert_test(!messages.empty(), "Hall of Mountain King generates messages");
        
        // Check for accelerating pattern (decreasing durations)
        bool hasAcceleration = false;
        if (messages.size() > 20) {
            double firstDuration = messages[0].duration;
            double laterDuration = messages[messages.size() - 5].duration;
            hasAcceleration = (laterDuration < firstDuration);
        }
        assert_test(hasAcceleration, "Hall of Mountain King has acceleration");
        
        // Verify multiple octave layers
        std::set<int> uniqueNotes;
        for (const auto& msg : messages) {
            uniqueNotes.insert(msg.note % 12); // Note class regardless of octave
        }
        assert_test(uniqueNotes.size() >= 6, "Hall of Mountain King uses varied scale");
    }
    
    void testVivaldiSpringGeneration() {
        auto messages = midi.generateVivaldiSpring();
        
        assert_test(!messages.empty(), "Vivaldi Spring generates messages");
        
        // Verify violin-like high register usage
        bool hasHighNotes = false;
        for (const auto& msg : messages) {
            if (msg.note > 84) hasHighNotes = true; // Above C6
        }
        assert_test(hasHighNotes, "Vivaldi Spring uses high violin register");
        
        // Check for ornamental passages (short rapid notes)
        int shortNotes = 0;
        for (const auto& msg : messages) {
            if (msg.duration <= 0.15) shortNotes++;
        }
        assert_test(shortNotes >= 5, "Vivaldi Spring has ornamental passages");
    }
    
    void testRushEKeysGeneration() {
        auto keyEvents = midi.generateRushEKeys();
        
        assert_test(!keyEvents.empty(), "Rush E keys generates events");
        assert_test(keyEvents.size() % 2 == 0, "Rush E keys has paired events");
        
        // Verify key down/up pairing
        int keyDownCount = 0, keyUpCount = 0;
        for (const auto& event : keyEvents) {
            if (event.state == KeyState::KeyDown) keyDownCount++;
            if (event.state == KeyState::KeyUp) keyUpCount++;
        }
        assert_test(keyDownCount == keyUpCount, "Rush E keys has balanced key events");
        
        // Check for building velocity (Rush E should get louder)
        bool hasVelocityBuildup = false;
        if (keyEvents.size() > 10) {
            int earlyVelocity = keyEvents[2].velocity;
            int lateVelocity = keyEvents[keyEvents.size() - 4].velocity;
            hasVelocityBuildup = (lateVelocity > earlyVelocity);
        }
        assert_test(hasVelocityBuildup, "Rush E keys has velocity buildup");
    }
    
    void testFurEliseKeysGeneration() {
        auto keyEvents = midi.generateFurEliseKeys();
        
        assert_test(!keyEvents.empty(), "Fur Elise keys generates events");
        
        // Verify all events are piano events
        bool allPianoEvents = true;
        for (const auto& event : keyEvents) {
            if (event.device != DeviceType::Piano) {
                allPianoEvents = false;
                break;
            }
        }
        assert_test(allPianoEvents, "Fur Elise keys are all piano events");
        
        // Check for reasonable velocity range
        int minVel = 127, maxVel = 0;
        for (const auto& event : keyEvents) {
            if (event.state == KeyState::KeyDown) {
                minVel = std::min(minVel, event.velocity);
                maxVel = std::max(maxVel, event.velocity);
            }
        }
        assert_test(maxVel > minVel && minVel > 30 && maxVel < 127, "Fur Elise has musical velocity range");
    }
    
    void testBeethoven5thKeysGeneration() {
        auto keyEvents = midi.generateBeethoven5thKeys();
        
        assert_test(!keyEvents.empty(), "Beethoven 5th keys generates events");
        
        // Check for forte dynamics (high velocities)
        bool hasForteVelocities = false;
        for (const auto& event : keyEvents) {
            if (event.state == KeyState::KeyDown && event.velocity > 100) {
                hasForteVelocities = true;
                break;
            }
        }
        assert_test(hasForteVelocities, "Beethoven 5th has forte dynamics");
    }
    
    void testHallOfMountainKingKeysGeneration() {
        auto keyEvents = midi.generateHallOfMountainKingKeys();
        
        assert_test(!keyEvents.empty(), "Hall Mountain King keys generates events");
        
        // Check for crescendo (increasing velocities)
        bool hasCrescendo = false;
        if (keyEvents.size() > 20) {
            int earlyVel = 0, lateVel = 0;
            for (size_t i = 0; i < 10; ++i) {
                if (keyEvents[i].state == KeyState::KeyDown) {
                    earlyVel = std::max(earlyVel, keyEvents[i].velocity);
                }
            }
            for (size_t i = keyEvents.size() - 10; i < keyEvents.size(); ++i) {
                if (keyEvents[i].state == KeyState::KeyDown) {
                    lateVel = std::max(lateVel, keyEvents[i].velocity);
                }
            }
            hasCrescendo = (lateVel > earlyVel + 20);
        }
        assert_test(hasCrescendo, "Hall Mountain King has crescendo");
    }
    
    void testVivaldiSpringKeysGeneration() {
        auto keyEvents = midi.generateVivaldiSpringKeys();
        
        assert_test(!keyEvents.empty(), "Vivaldi Spring keys generates events");
        
        // Check for delicate touch (moderate velocities)
        bool hasDelicateTouch = true;
        for (const auto& event : keyEvents) {
            if (event.state == KeyState::KeyDown && event.velocity > 110) {
                hasDelicateTouch = false;
                break;
            }
        }
        assert_test(hasDelicateTouch, "Vivaldi Spring has delicate baroque touch");
    }
    
    void testDrumPatternGeneration() {
        auto keyEvents = midi.generateDrumPattern();
        
        assert_test(!keyEvents.empty(), "Drum pattern generates events");
        
        // Verify all events are drum events
        bool allDrumEvents = true;
        for (const auto& event : keyEvents) {
            if (event.device != DeviceType::DrumPad) {
                allDrumEvents = false;
                break;
            }
        }
        assert_test(allDrumEvents, "Drum pattern events are all drum events");
        
        // Check for channel 10 (standard drum channel)
        bool hasChannel10 = false;
        for (const auto& event : keyEvents) {
            if (event.channel == 10) {
                hasChannel10 = true;
                break;
            }
        }
        assert_test(hasChannel10, "Drum pattern uses channel 10");
        
        // Verify rhythmic timing
        bool hasRhythmicTiming = false;
        if (keyEvents.size() > 4) {
            double timeDiff = keyEvents[4].timestamp - keyEvents[0].timestamp;
            // Check for beat spacing - should be around 0.5 seconds (beatDuration)
            hasRhythmicTiming = (timeDiff > 0.3 && timeDiff < 0.7);
        }
        assert_test(hasRhythmicTiming, "Drum pattern has rhythmic timing");
    }
    
    void testMixedPerformanceGeneration() {
        auto keyEvents = midi.generateMixedPerformance();
        
        assert_test(!keyEvents.empty(), "Mixed performance generates events");
        
        // Verify both piano and drum events exist
        bool hasPiano = false, hasDrums = false;
        for (const auto& event : keyEvents) {
            if (event.device == DeviceType::Piano) hasPiano = true;
            if (event.device == DeviceType::DrumPad) hasDrums = true;
        }
        assert_test(hasPiano && hasDrums, "Mixed performance has both piano and drums");
        
        // Check chronological ordering
        bool isOrdered = true;
        for (size_t i = 1; i < keyEvents.size(); ++i) {
            if (keyEvents[i].timestamp < keyEvents[i-1].timestamp) {
                isOrdered = false;
                break;
            }
        }
        assert_test(isOrdered, "Mixed performance events are chronologically ordered");
    }
    
    void testDrumMapping() {
        // Test drum mapping functions
        for (int pad = 0; pad < 8; ++pad) {
            const DrumMapping& mapping = MidiInput::getDrumMapping(pad);
            assert_test(mapping.padNumber == pad, "Drum mapping pad number correct");
            assert_test(mapping.midiNote >= 36 && mapping.midiNote <= 51, "Drum MIDI note in valid range");
            assert_test(mapping.name != nullptr, "Drum mapping has name");
            
            int midiNote = MidiInput::getMidiNoteForPad(pad);
            assert_test(midiNote == mapping.midiNote, "getMidiNoteForPad matches mapping");
            
            const char* name = MidiInput::getDrumName(pad);
            assert_test(name == mapping.name, "getDrumName matches mapping");
        }
        
        // Test edge cases
        const DrumMapping& invalidMapping = MidiInput::getDrumMapping(99);
        assert_test(invalidMapping.padNumber == 0, "Invalid pad returns default mapping");
    }
    
    void testHelperFunctions() {
        // Test createPianoEvent
        KeyEvent pianoEvent = midi.createPianoEvent(KeyState::KeyDown, 60, 100, 1.5, 1);
        assert_test(pianoEvent.device == DeviceType::Piano, "createPianoEvent sets correct device");
        assert_test(pianoEvent.state == KeyState::KeyDown, "createPianoEvent sets correct state");
        assert_test(pianoEvent.note == 60, "createPianoEvent sets correct note");
        assert_test(pianoEvent.velocity == 100, "createPianoEvent sets correct velocity");
        assert_test(pianoEvent.channel == 1, "createPianoEvent sets correct channel");
        assert_test(pianoEvent.timestamp == 1.5, "createPianoEvent sets correct timestamp");
        
        // Test createDrumEvent
        KeyEvent drumEvent = midi.createDrumEvent(KeyState::KeyDown, 2, 120, 2.0, 10);
        assert_test(drumEvent.device == DeviceType::DrumPad, "createDrumEvent sets correct device");
        assert_test(drumEvent.state == KeyState::KeyDown, "createDrumEvent sets correct state");
        assert_test(drumEvent.note == 42, "createDrumEvent maps pad to correct MIDI note"); // Pad 2 = Hi-hat = 42
        assert_test(drumEvent.velocity == 120, "createDrumEvent sets correct velocity");
        assert_test(drumEvent.channel == 10, "createDrumEvent sets correct channel");
        assert_test(drumEvent.timestamp == 2.0, "createDrumEvent sets correct timestamp");
    }
    
    void testEdgeCases() {
        // Test convertToKeyEvents with empty input
        std::vector<MidiMessage> emptyMidi;
        auto emptyKeys = midi.convertToKeyEvents(emptyMidi);
        assert_test(emptyKeys.empty(), "convertToKeyEvents handles empty input");
        
        // Test generateDemo
        auto demoMessages = midi.generateDemo();
        assert_test(!demoMessages.empty(), "generateDemo produces output");
        
        // Test extreme values
        KeyEvent extremeEvent = midi.createPianoEvent(KeyState::KeyUp, 127, 0, 0.0, 16);
        assert_test(extremeEvent.note == 127, "Handles extreme note values");
        assert_test(extremeEvent.velocity == 0, "Handles zero velocity");
        assert_test(extremeEvent.timestamp == 0.0, "Handles zero timestamp");
        assert_test(extremeEvent.channel == 16, "Handles max channel");
    }
};

int main() {
    try {
        MidiInputTest test;
        test.runAllTests();
        std::cout << "All MidiInput unit tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }
}