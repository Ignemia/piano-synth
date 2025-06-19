#include "core/physics/string_model.h"
#include "core/utils/math_utils.h"
#include "core/utils/constants.h"
#include <iostream>

int main() {
    std::cout << "=== String Model Debug ===" << std::endl;
    
    int note_number = 60; // Middle C
    std::cout << "Note number: " << note_number << std::endl;
    
    // Check MIDI to frequency conversion
    double expected_freq = PianoSynth::Utils::MathUtils::midiToFrequency(note_number);
    std::cout << "Expected frequency (MIDI->freq): " << expected_freq << " Hz" << std::endl;
    
    // Create string model and check fundamental frequency step by step
    std::cout << "\n=== Constructor Analysis ===" << std::endl;
    
    // Test the calculation manually first
    double diameter = 0.001 + (108 - note_number) * 0.00005;
    double tension = PianoSynth::Constants::STRING_TENSION_BASE * (1.0 + (note_number - 60) * 0.01);
    double cross_sectional_area = PianoSynth::Utils::MathUtils::PI * diameter * diameter / 4.0;
    double linear_density = PianoSynth::Constants::STRING_DENSITY * cross_sectional_area;
    double wave_speed = PianoSynth::Utils::MathUtils::calculateStringWaveSpeed(tension, linear_density);
    double calculated_length = wave_speed / (2.0 * expected_freq);
    
    std::cout << "Manual calculation:" << std::endl;
    std::cout << "  Diameter: " << diameter << " m" << std::endl;
    std::cout << "  Tension: " << tension << " N" << std::endl;
    std::cout << "  Cross-sectional area: " << cross_sectional_area << " m²" << std::endl;
    std::cout << "  Linear density: " << linear_density << " kg/m" << std::endl;
    std::cout << "  Wave speed: " << wave_speed << " m/s" << std::endl;
    std::cout << "  Calculated length: " << calculated_length << " m" << std::endl;
    std::cout << "  Verification frequency: " << wave_speed / (2.0 * calculated_length) << " Hz" << std::endl;
    
    // Now create the actual string model
    std::cout << "\n=== String Model Creation ===" << std::endl;
    PianoSynth::Physics::StringModel string_model(note_number);
    
    std::cout << "After constructor:" << std::endl;
    std::cout << "  Fundamental frequency: " << string_model.getFundamentalFrequency() << " Hz" << std::endl;
    std::cout << "  Length: " << string_model.getLength() << " m" << std::endl;
    std::cout << "  Tension: " << string_model.getTension() << " N" << std::endl;
    
    // Initialize with sample rate
    std::cout << "\n=== After Initialization ===" << std::endl;
    string_model.initialize(44100.0);
    std::cout << "  Fundamental frequency: " << string_model.getFundamentalFrequency() << " Hz" << std::endl;
    std::cout << "  Length: " << string_model.getLength() << " m" << std::endl;
    std::cout << "  Tension: " << string_model.getTension() << " N" << std::endl;
    
    return 0;
}