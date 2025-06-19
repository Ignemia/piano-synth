#pragma once

namespace PianoSynth {
namespace Constants {

// Audio settings
constexpr double SAMPLE_RATE = 44100.0;
constexpr int BUFFER_SIZE = 512;
constexpr int CHANNELS = 2; // Stereo

// Piano physical constants
constexpr int NUM_KEYS = 88;
constexpr int LOWEST_KEY = 21; // A0
constexpr int HIGHEST_KEY = 108; // C8

// String physics constants
constexpr double STRING_DENSITY = 7850.0; // kg/m³ (steel)
constexpr double YOUNG_MODULUS = 200e9; // Pa (steel)
constexpr double STRING_TENSION_BASE = 800.0; // N (more realistic base tension)
constexpr double STRING_DAMPING = 0.015; // Increased for better stability
constexpr double STRING_STIFFNESS = 5e-6; // Reduced stiffness for smoother sound

// Hammer physics constants
constexpr double HAMMER_MASS = 0.008; // kg (lighter hammer)
constexpr double HAMMER_STIFFNESS = 5e5; // N/m (softer contact)
constexpr double HAMMER_DAMPING = 50.0; // Ns/m (less damping)
constexpr double HAMMER_CONTACT_TIME = 0.0015; // seconds (longer contact)

// Soundboard constants
constexpr double SOUNDBOARD_AREA = 0.5; // m²
constexpr double SOUNDBOARD_THICKNESS = 0.01; // m
constexpr double SOUNDBOARD_DENSITY = 400.0; // kg/m³ (spruce)
constexpr double SOUNDBOARD_DAMPING = 0.01;

// Resonance constants
constexpr int MAX_HARMONICS = 16; // Reduced to prevent aliasing
constexpr double HARMONIC_DECAY = 0.9; // Slower decay for richer sound
constexpr double SYMPATHETIC_RESONANCE = 0.05; // Reduced for stability

// MIDI constants
constexpr int MIDI_NOTE_ON = 0x90;
constexpr int MIDI_NOTE_OFF = 0x80;
constexpr int MIDI_CONTROL_CHANGE = 0xB0;
constexpr int MIDI_SUSTAIN_PEDAL = 64;

// Processing constants
constexpr int MAX_VOICES = 64; // Reduced for better performance
constexpr double NOTE_OFF_FADE_TIME = 0.25; // seconds (longer fade)
constexpr double VELOCITY_SENSITIVITY = 0.008; // More sensitive

// File I/O constants
constexpr int MP3_BITRATE = 192; // kbps
constexpr int MP3_QUALITY = 5; // LAME quality (0-9)

} // namespace Constants
} // namespace PianoSynth
