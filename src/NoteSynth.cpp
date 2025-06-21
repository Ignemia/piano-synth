#include "../include/NoteSynth.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

/**
 * @brief [AI GENERATED] Generate realistic piano samples with inharmonicity,
 *        velocity-dependent brightness, and proper harmonic decay.
 */
std::vector<double> NoteSynth::synthesize(const std::vector<NoteEvent>& events,
                                          int sampleRate) const {
    // Calculate total duration including release (with sustain pedal)
    const double kReleaseTime = 4.0;
    double dTotalDuration = 0.0;
    for (const auto& e : events) {
        double dEnd = e.startTime + e.duration + kReleaseTime;
        dTotalDuration = std::max(dTotalDuration, dEnd);
    }

    const int iTotalSamples = static_cast<int>(dTotalDuration * sampleRate);
    std::vector<double> samples(iTotalSamples, 0.0);

    for (const auto& e : events) {
        const int iStart = static_cast<int>(e.startTime * sampleRate);
        const int iHold = static_cast<int>(e.duration * sampleRate);
        const int iRelease = static_cast<int>(kReleaseTime * sampleRate);
        const int iCount = iHold + iRelease;

        // Envelope timing (ADSR) - grand piano with sustain pedal
        const double kAttackTime = 0.002; // Very fast attack like hammer strike
        const double kDecayTime = 0.8;    // Longer decay for string resonance
        const double kSustainLevel = 0.4;  // Higher sustain with pedal pressed
        
        const int iAttackSamples = static_cast<int>(kAttackTime * sampleRate);
        const int iDecaySamples = static_cast<int>(kDecayTime * sampleRate);

        // Determine number of harmonics based on frequency (grand piano characteristics)
        int iMaxHarmonics;
        if (e.frequency < 130.0) {
            iMaxHarmonics = 30; // Bass: rich harmonic content like real grand piano
        } else if (e.frequency < 520.0) {
            iMaxHarmonics = 15; // Mid: moderate harmonics
        } else {
            iMaxHarmonics = 8;  // Treble: fewer harmonics as frequency increases
        }

        // Inharmonicity factor (grand piano string stiffness)
        const double B = (e.frequency / 27.5) * 0.00003; // Realistic inharmonicity for grand piano

        // Velocity-dependent brightness (simulate hammer-string interaction)
        // Use default velocity since NoteEvent doesn't have amplitude field
        const double dVelocity = 0.7; // Moderate velocity for realistic sound
        const int iActiveHarmonics = static_cast<int>(iMaxHarmonics * (0.4 + 0.6 * dVelocity));

        // Hammer noise parameters
        const double kHammerTime = 0.01;
        const int iHammerSamples = static_cast<int>(kHammerTime * sampleRate);

        for (int i = 0; i < iCount; ++i) {
            const double t = static_cast<double>(i) / sampleRate;
            
            // ADSR Envelope
            double dEnvelope = 1.0;
            if (i < iAttackSamples) {
                // Attack
                dEnvelope = static_cast<double>(i) / iAttackSamples;
            } else if (i < iAttackSamples + iDecaySamples) {
                // Decay
                double dDecayProgress = static_cast<double>(i - iAttackSamples) / iDecaySamples;
                dEnvelope = 1.0 - (1.0 - kSustainLevel) * dDecayProgress;
            } else if (i < iHold) {
                // Sustain
                dEnvelope = kSustainLevel;
            } else {
                // Release
                double dReleaseProgress = static_cast<double>(i - iHold) / iRelease;
                dEnvelope = kSustainLevel * std::exp(-3.0 * dReleaseProgress);
            }

            double dValue = 0.0;
            
            // Generate harmonics with inharmonicity and realistic decay
            for (int h = 1; h <= iActiveHarmonics; ++h) {
                // Inharmonic frequency: f_n = f_0 * n * sqrt(1 + B * n^2)
                const double dInharmonicFreq = e.frequency * h * std::sqrt(1.0 + B * h * h);
                
                // Phase for this harmonic
                const double dPhase = 2.0 * M_PI * dInharmonicFreq * t;
                
                // Harmonic amplitude with velocity dependence
                double dHarmonicAmp = 1.0 / h; // Basic 1/n falloff
                
                // Grand piano harmonic shaping (velocity-dependent brightness)
                if (h > 1) {
                    // Higher harmonics boosted by velocity (hammer hardness effect)
                    dHarmonicAmp *= (0.3 + 0.9 * dVelocity * std::exp(-0.2 * (h - 1)));
                }
                
                // Grand piano string decay characteristics (with sustain pedal)
                double dHarmonicDecay;
                if (h == 1) {
                    // Fundamental: very slow decay with sustain pedal
                    dHarmonicDecay = std::exp(-t * 0.02);
                } else if (h <= 5) {
                    // Low harmonics: slow decay with sustain pedal
                    dHarmonicDecay = std::exp(-t * (0.03 + 0.02 * h));
                } else {
                    // High harmonics: moderate decay even with sustain pedal
                    dHarmonicDecay = std::exp(-t * (0.08 + 0.04 * h));
                }
                dHarmonicAmp *= dHarmonicDecay;
                
                // Add subtle string resonance modulation
                double dResonance = 1.0 + 0.02 * std::sin(2.0 * M_PI * 5.0 * t) * std::exp(-t * 2.0);
                
                dValue += dHarmonicAmp * std::sin(dPhase) * dResonance;
            }

            // Add hammer noise and soundboard resonance
            if (i < iHammerSamples) {
                const double dNoise = static_cast<double>(std::rand()) / RAND_MAX * 2.0 - 1.0;
                const double dHammerEnv = (1.0 - static_cast<double>(i) / iHammerSamples);
                const double dNoiseAmp = 0.002 * dVelocity * dHammerEnv;
                dValue += dNoiseAmp * dNoise;
            }
            
            // Soundboard resonance (adds body to the sound)
            if (t < 0.1) {
                double dSoundboard = 0.1 * std::sin(2.0 * M_PI * e.frequency * 0.5 * t) * std::exp(-t * 8.0);
                dValue += dSoundboard;
            }

            // Apply overall amplitude scaling
            dValue *= 0.8; // Moderate scaling for realistic grand piano volume

            samples[iStart + i] += dEnvelope * dValue;
        }
    }

    // Normalize to prevent clipping
    double dMax = 0.0;
    for (double s : samples) {
        dMax = std::max(dMax, std::abs(s));
    }
    if (dMax > 0.95) {
        const double dScale = 0.95 / dMax;
        for (double& s : samples) {
            s *= dScale;
        }
    }

    return samples;
}