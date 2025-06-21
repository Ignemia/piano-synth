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

        // Envelope timing (ADSR) - natural piano with proper sustain
        const double kAttackTime = 0.001; // Very fast attack like hammer strike
        const double kDecayTime = 0.4;    // Moderate decay for natural sound
        const double kSustainLevel = 0.35; // Natural sustain level
        
        const int iAttackSamples = static_cast<int>(kAttackTime * sampleRate);
        const int iDecaySamples = static_cast<int>(kDecayTime * sampleRate);

        // Determine number of harmonics based on frequency (natural piano brightness)
        int iMaxHarmonics;
        if (e.frequency < 130.0) {
            iMaxHarmonics = 15; // Bass: rich harmonic content for warmth
        } else if (e.frequency < 520.0) {
            iMaxHarmonics = 12; // Mid: good harmonic content for brightness
        } else {
            iMaxHarmonics = 8;  // Treble: moderate harmonics for clarity
        }

        // Minimal inharmonicity for natural sound
        const double B = 0.0; // Remove inharmonicity to eliminate beating

        // Velocity-dependent brightness (simulate hammer-string interaction)
        // Use actual velocity from key press event
        const double dVelocity = std::min(1.0, std::max(0.1, e.velocity)); // Clamp velocity to reasonable range
        const int iActiveHarmonics = static_cast<int>(iMaxHarmonics * (0.3 + 0.7 * dVelocity));

        // No hammer noise - clean sine waves only

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
                
                // Velocity-dependent harmonic amplitude scaling
                if (h > 1) {
                    // Higher harmonics affected by velocity (harder strikes = more upper harmonics)
                    dHarmonicAmp *= (0.4 + 0.6 * dVelocity) * std::exp(-0.15 * (h - 1));
                }
                
                // Natural string decay characteristics
                double dHarmonicDecay;
                if (h == 1) {
                    // Fundamental: slow decay for sustain
                    dHarmonicDecay = std::exp(-t * 0.15);
                } else if (h <= 4) {
                    // Low harmonics: moderate decay for warmth
                    dHarmonicDecay = std::exp(-t * (0.2 + 0.1 * h));
                } else {
                    // High harmonics: faster decay but not too fast
                    dHarmonicDecay = std::exp(-t * (0.4 + 0.2 * h));
                }
                dHarmonicAmp *= dHarmonicDecay;
                
                dValue += dHarmonicAmp * std::sin(dPhase);
            }

            // No noise - pure sine waves only

            // Apply velocity-dependent amplitude scaling
            dValue *= dVelocity * 0.8; // Scale by velocity for realistic dynamics

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