#include "../include/MidiInput.h"
#include "../include/Abstractor.h"
#include "../include/NoteSynth.h"
#include "../include/OutputHandler.h"
#include <iostream>
#include <string>

/**
 * @file main.cpp
 * @brief [AI GENERATED] Entry point selecting demo or realtime mode.
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [options]\n";
        std::cout << "Available test pieces:\n";
        std::cout << "  --demo          Generate all 5 test pieces\n";
        std::cout << "  --rush-e        Rush E virtuosic passages\n";
        std::cout << "  --fur-elise     Für Elise opening melody\n";
        std::cout << "  --beethoven5    Beethoven's 5th Symphony opening\n";
        std::cout << "  --hall-mountain In the Hall of the Mountain King\n";
        std::cout << "  --vivaldi-spring Vivaldi's Spring from Four Seasons\n";
        std::cout << "Key-based synthesis (realistic key press/release):\n";
        std::cout << "  --fur-elise-keys Für Elise with velocity-sensitive keys\n";
        std::cout << "  --rush-e-keys   Rush E with key dynamics\n";
        std::cout << "  --beethoven5-keys Beethoven's 5th with key expression\n";
        std::cout << "  --hall-mountain-keys Hall of Mountain King with key dynamics\n";
        std::cout << "  --vivaldi-spring-keys Vivaldi Spring with key expression\n";
        std::cout << "M-Audio Oxygen Pro 61 specific features:\n";
        std::cout << "  --drum-pattern   Drum pattern using 8 velocity-sensitive pads\n";
        std::cout << "  --mixed-performance Piano + drums mixed performance\n";
        return 1;
    }

    std::string option = argv[1];
    MidiInput midi;
    Abstractor abs;
    NoteSynth synth;
    OutputHandler out;

    std::vector<NoteEvent> notes;
    std::string outputFile;

    // Handle demo option - generate all 5 test pieces
    if (option == "--demo") {
        // Generate all 5 pieces with key-based synthesis
        std::vector<std::pair<std::string, std::string>> pieces = {
            {"fur_elise_demo.wav", "Für Elise"},
            {"rush_e_demo.wav", "Rush E"},
            {"beethoven5_demo.wav", "Beethoven's 5th"},
            {"hall_mountain_demo.wav", "Hall of Mountain King"},
            {"vivaldi_spring_demo.wav", "Vivaldi Spring"}
        };
        
        for (size_t i = 0; i < pieces.size(); ++i) {
            std::vector<KeyEvent> keyEvents;
            switch (i) {
                case 0: keyEvents = midi.generateFurEliseKeys(); break;
                case 1: keyEvents = midi.generateRushEKeys(); break;
                case 2: keyEvents = midi.generateBeethoven5thKeys(); break;
                case 3: keyEvents = midi.generateHallOfMountainKingKeys(); break;
                case 4: keyEvents = midi.generateVivaldiSpringKeys(); break;
            }
            auto pieceNotes = abs.convertKeyEvents(keyEvents);
            auto pieceSamples = synth.synthesize(pieceNotes);
            out.writeWav(pieceSamples, pieces[i].first);
            std::cout << pieces[i].second << " written to " << pieces[i].first << "\n";
        }
        return 0;
    }
    // Handle key-based synthesis options
    else if (option == "--fur-elise-keys") {
        auto keyEvents = midi.generateFurEliseKeys();
        notes = abs.convertKeyEvents(keyEvents);
        outputFile = "fur_elise_keys_output.wav";
        std::cout << "Für Elise (key-based) written to " << outputFile << "\n";
    } else if (option == "--rush-e-keys") {
        auto keyEvents = midi.generateRushEKeys();
        notes = abs.convertKeyEvents(keyEvents);
        outputFile = "rush_e_keys_output.wav";
        std::cout << "Rush E (key-based) written to " << outputFile << "\n";
    } else if (option == "--beethoven5-keys") {
        auto keyEvents = midi.generateBeethoven5thKeys();
        notes = abs.convertKeyEvents(keyEvents);
        outputFile = "beethoven5_keys_output.wav";
        std::cout << "Beethoven's 5th (key-based) written to " << outputFile << "\n";
    } else if (option == "--hall-mountain-keys") {
        auto keyEvents = midi.generateHallOfMountainKingKeys();
        notes = abs.convertKeyEvents(keyEvents);
        outputFile = "hall_mountain_keys_output.wav";
        std::cout << "Hall of Mountain King (key-based) written to " << outputFile << "\n";
    } else if (option == "--vivaldi-spring-keys") {
        auto keyEvents = midi.generateVivaldiSpringKeys();
        notes = abs.convertKeyEvents(keyEvents);
        outputFile = "vivaldi_spring_keys_output.wav";
        std::cout << "Vivaldi Spring (key-based) written to " << outputFile << "\n";
    } else if (option == "--drum-pattern") {
        auto keyEvents = midi.generateDrumPattern();
        notes = abs.convertKeyEvents(keyEvents);
        outputFile = "drum_pattern_output.wav";
        std::cout << "Drum pattern written to " << outputFile << "\n";
    } else if (option == "--mixed-performance") {
        auto keyEvents = midi.generateMixedPerformance();
        notes = abs.convertKeyEvents(keyEvents);
        outputFile = "mixed_performance_output.wav";
        std::cout << "Mixed performance (piano + drums) written to " << outputFile << "\n";
    }
    // Handle legacy MIDI-based synthesis options
    else if (option == "--rush-e") {
        auto midiData = midi.generateRushE();
        notes = abs.convert(midiData);
        outputFile = "rush_e_output.wav";
        std::cout << "Rush E written to " << outputFile << "\n";
    } else if (option == "--fur-elise") {
        auto midiData = midi.generateFurElise();
        notes = abs.convert(midiData);
        outputFile = "fur_elise_output.wav";
        std::cout << "Für Elise written to " << outputFile << "\n";
    } else if (option == "--beethoven5") {
        auto midiData = midi.generateBeethoven5th();
        notes = abs.convert(midiData);
        outputFile = "beethoven5_output.wav";
        std::cout << "Beethoven's 5th written to " << outputFile << "\n";
    } else if (option == "--hall-mountain") {
        auto midiData = midi.generateHallOfMountainKing();
        notes = abs.convert(midiData);
        outputFile = "hall_mountain_output.wav";
        std::cout << "Hall of Mountain King written to " << outputFile << "\n";
    } else if (option == "--vivaldi-spring") {
        auto midiData = midi.generateVivaldiSpring();
        notes = abs.convert(midiData);
        outputFile = "vivaldi_spring_output.wav";
        std::cout << "Vivaldi Spring written to " << outputFile << "\n";
    } else {
        std::cout << "Unknown option: " << option << "\n";
        std::cout << "Use --help or no arguments to see available options.\n";
        return 1;
    }

    auto samples = synth.synthesize(notes);
    out.writeWav(samples, outputFile);

    return 0;
}
