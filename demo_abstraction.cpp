#include "core/abstraction/input_abstractor.h"
#include "core/abstraction/note_event.h"
#include "core/utils/logger.h"
#include "core/utils/math_utils.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <thread>

using namespace PianoSynth;

class AbstractionDemo {
public:
    AbstractionDemo() {
        logger_ = std::make_unique<Utils::Logger>(Utils::LogLevel::kInfo);
        logger_->setLogToConsole(true);
        
        abstractor_ = std::make_unique<Abstraction::InputAbstractor>();
        abstractor_->initialize();
    }
    
    void demonstrateAbstraction() {
        std::cout << "🎛️  MIDI Abstraction Layer Demonstration" << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << "This demo shows how raw MIDI events are processed through" << std::endl;
        std::cout << "the abstraction layer to create enriched note events with" << std::endl;
        std::cout << "physical modeling parameters." << std::endl;
        std::cout << std::endl;
        
        // Demonstrate different types of MIDI events
        demonstrateNoteEvents();
        demonstratePedalEvents();
        demonstrateVelocityCurves();
        demonstrateTimingPrecision();
        
        std::cout << "\n🎉 Abstraction layer demonstration complete!" << std::endl;
    }
    
private:
    void demonstrateNoteEvents() {
        std::cout << "📝 1. Note Event Processing" << std::endl;
        std::cout << "============================" << std::endl;
        
        // Create raw MIDI note on event (Middle C, velocity 100)
        std::vector<unsigned char> midi_data = {0x90, 60, 100};
        Input::RawMidiEvent raw_event;
        raw_event.data = midi_data;
        raw_event.timestamp = getCurrentTime();
        raw_event.source_port = 0;
        
        std::cout << "📥 Raw MIDI Input:" << std::endl;
        std::cout << "   Status Byte: 0x" << std::hex << (int)midi_data[0] << std::dec 
                  << " (Note On, Channel 1)" << std::endl;
        std::cout << "   Note Number: " << (int)midi_data[1] << " (Middle C)" << std::endl;
        std::cout << "   Velocity: " << (int)midi_data[2] << "/127" << std::endl;
        std::cout << "   Timestamp: " << raw_event.timestamp << std::endl;
        
        // Process through abstraction layer
        std::vector<Input::RawMidiEvent> raw_events = {raw_event};
        auto abstracted_events = abstractor_->processEvents(raw_events);
        
        std::cout << "\n🎛️  Abstracted Output:" << std::endl;
        for (const auto& event : abstracted_events) {
            printNoteEvent(event);
        }
        
        // Now demonstrate note off
        std::cout << "\n📤 Note Off Processing:" << std::endl;
        std::vector<unsigned char> midi_off = {0x80, 60, 64};
        Input::RawMidiEvent off_event;
        off_event.data = midi_off;
        off_event.timestamp = getCurrentTime() + 1.0; // 1 second later
        off_event.source_port = 0;
        
        std::cout << "📥 Raw MIDI: [0x" << std::hex << (int)midi_off[0] << std::dec 
                  << " " << (int)midi_off[1] << " " << (int)midi_off[2] << "]" << std::endl;
        
        std::vector<Input::RawMidiEvent> off_events = {off_event};
        auto off_abstracted = abstractor_->processEvents(off_events);
        
        for (const auto& event : off_abstracted) {
            printNoteEvent(event);
        }
        
        std::cout << std::endl;
    }
    
    void demonstratePedalEvents() {
        std::cout << "🦶 2. Pedal Event Processing" << std::endl;
        std::cout << "============================" << std::endl;
        
        // Sustain pedal on
        std::vector<unsigned char> sustain_on = {0xB0, 64, 127};
        Input::RawMidiEvent pedal_event;
        pedal_event.data = sustain_on;
        pedal_event.timestamp = getCurrentTime();
        pedal_event.source_port = 0;
        
        std::cout << "📥 Raw MIDI Sustain Pedal On:" << std::endl;
        std::cout << "   [0x" << std::hex << (int)sustain_on[0] << std::dec 
                  << " " << (int)sustain_on[1] << " " << (int)sustain_on[2] << "]" << std::endl;
        std::cout << "   (Control Change, Controller 64 = Sustain, Value 127 = On)" << std::endl;
        
        std::vector<Input::RawMidiEvent> pedal_events = {pedal_event};
        auto abstracted_pedal = abstractor_->processEvents(pedal_events);
        
        std::cout << "\n🎛️  Abstracted Pedal Event:" << std::endl;
        for (const auto& event : abstracted_pedal) {
            printNoteEvent(event);
        }
        
        // Now play a note with sustain active
        std::cout << "\n🎵 Playing note with sustain pedal active:" << std::endl;
        std::vector<unsigned char> note_with_sustain = {0x90, 67, 80}; // G4
        Input::RawMidiEvent note_event;
        note_event.data = note_with_sustain;
        note_event.timestamp = getCurrentTime();
        note_event.source_port = 0;
        
        std::vector<Input::RawMidiEvent> note_events = {note_event};
        auto note_abstracted = abstractor_->processEvents(note_events);
        
        for (const auto& event : note_abstracted) {
            std::cout << "   Note with sustain: sustain_pedal=" 
                      << (event.sustain_pedal ? "true" : "false") << std::endl;
            std::cout << "   Damper position: " << event.damper_position << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    void demonstrateVelocityCurves() {
        std::cout << "📈 3. Velocity Curve Processing" << std::endl;
        std::cout << "===============================" << std::endl;
        
        std::cout << "Testing different MIDI velocities and their physical translations:" << std::endl;
        std::cout << std::setw(12) << "MIDI Vel" << std::setw(15) << "Normalized" 
                  << std::setw(18) << "Hammer Vel (m/s)" << std::setw(20) << "String Excitation" << std::endl;
        std::cout << std::string(65, '-') << std::endl;
        
        for (int velocity : {1, 32, 64, 96, 127}) {
            std::vector<unsigned char> midi_data = {0x90, 60, (unsigned char)velocity};
            Input::RawMidiEvent raw_event;
            raw_event.data = midi_data;
            raw_event.timestamp = getCurrentTime();
            raw_event.source_port = 0;
            
            std::vector<Input::RawMidiEvent> events = {raw_event};
            auto abstracted = abstractor_->processEvents(events);
            
            if (!abstracted.empty()) {
                const auto& event = abstracted[0];
                std::cout << std::setw(12) << velocity 
                          << std::setw(15) << std::fixed << std::setprecision(3) << event.velocity
                          << std::setw(18) << std::setprecision(2) << event.hammer_velocity
                          << std::setw(20) << std::setprecision(3) << event.string_excitation << std::endl;
            }
        }
        
        std::cout << std::endl;
    }
    
    void demonstrateTimingPrecision() {
        std::cout << "⏱️  4. Timing Precision" << std::endl;
        std::cout << "=======================" << std::endl;
        
        std::cout << "Demonstrating high-precision timing capture:" << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Simulate rapid note events
        for (int i = 0; i < 5; ++i) {
            std::vector<unsigned char> midi_data = {0x90, (unsigned char)(60 + i), 80};
            Input::RawMidiEvent raw_event;
            raw_event.data = midi_data;
            raw_event.timestamp = getCurrentTime();
            raw_event.source_port = 0;
            
            std::vector<Input::RawMidiEvent> events = {raw_event};
            auto abstracted = abstractor_->processEvents(events);
            
            if (!abstracted.empty()) {
                auto event_time = std::chrono::duration_cast<std::chrono::microseconds>(
                    abstracted[0].press_time - start_time).count();
                
                std::cout << "   Note " << (60 + i) << ": " << event_time << " μs from start" << std::endl;
            }
            
            // Small delay to show timing differences
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        
        std::cout << std::endl;
    }
    
    void printNoteEvent(const Abstraction::NoteEvent& event) {
        std::cout << "   Event Type: ";
        switch (event.type) {
            case Abstraction::NoteEvent::NOTE_ON:
                std::cout << "NOTE_ON";
                break;
            case Abstraction::NoteEvent::NOTE_OFF:
                std::cout << "NOTE_OFF";
                break;
            case Abstraction::NoteEvent::PEDAL_CHANGE:
                std::cout << "PEDAL_CHANGE";
                break;
            default:
                std::cout << "OTHER";
                break;
        }
        std::cout << std::endl;
        
        if (event.type == Abstraction::NoteEvent::NOTE_ON || 
            event.type == Abstraction::NoteEvent::NOTE_OFF) {
            std::cout << "   Note Number: " << event.note_number 
                      << " (" << getMidiNoteName(event.note_number) 
                      << " @ " << std::fixed << std::setprecision(1) 
                      << Utils::MathUtils::midiToFrequency(event.note_number) << " Hz)" << std::endl;
            std::cout << "   Velocity: " << std::setprecision(3) << event.velocity << std::endl;
            std::cout << "   Hammer Velocity: " << std::setprecision(2) << event.hammer_velocity << " m/s" << std::endl;
            std::cout << "   String Excitation: " << std::setprecision(3) << event.string_excitation << " N" << std::endl;
            std::cout << "   Damper Position: " << std::setprecision(2) << event.damper_position << std::endl;
        }
        
        std::cout << "   Pedal States:" << std::endl;
        std::cout << "     Sustain: " << (event.sustain_pedal ? "ON" : "OFF") << std::endl;
        std::cout << "     Soft: " << (event.soft_pedal ? "ON" : "OFF") << std::endl;
        std::cout << "     Sostenuto: " << (event.sostenuto_pedal ? "ON" : "OFF") << std::endl;
        std::cout << "   Pitch Bend: " << std::setprecision(3) << event.pitch_bend << std::endl;
        std::cout << "   Aftertouch: " << std::setprecision(3) << event.aftertouch << std::endl;
    }
    
    std::string getMidiNoteName(int midi_note) {
        const char* note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        int octave = (midi_note / 12) - 1;
        int note = midi_note % 12;
        return std::string(note_names[note]) + std::to_string(octave);
    }
    
    double getCurrentTime() {
        static auto start = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        return duration.count() / 1000000.0; // Convert to seconds
    }
    
    std::unique_ptr<Utils::Logger> logger_;
    std::unique_ptr<Abstraction::InputAbstractor> abstractor_;
};

int main() {
    std::cout << "🎼 Piano Synthesizer - MIDI Abstraction Layer Demo" << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout << "This demonstration shows how the abstraction layer processes" << std::endl;
    std::cout << "raw MIDI events and enriches them with physical modeling parameters." << std::endl;
    std::cout << std::endl;
    std::cout << "The abstraction layer:" << std::endl;
    std::cout << "• Converts MIDI velocity to physical hammer velocity" << std::endl;
    std::cout << "• Calculates string excitation forces" << std::endl;
    std::cout << "• Manages pedal states and damper positions" << std::endl;
    std::cout << "• Provides high-precision timing information" << std::endl;
    std::cout << "• Tracks note durations and relationships" << std::endl;
    std::cout << std::endl;
    
    try {
        AbstractionDemo demo;
        
        std::cout << "Press Enter to start the demonstration..." << std::endl;
        std::cin.get();
        
        demo.demonstrateAbstraction();
        
        std::cout << "💡 Key Takeaways:" << std::endl;
        std::cout << "• Raw MIDI events are transformed into rich musical events" << std::endl;
        std::cout << "• Physical parameters are calculated for realistic synthesis" << std::endl;
        std::cout << "• Pedal states affect note behavior and damper positions" << std::endl;
        std::cout << "• High-precision timing enables accurate musical expression" << std::endl;
        std::cout << "• The abstraction layer provides the bridge between MIDI and physics" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Demo failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}