#include "../include/NoteSynth.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

/**
 * @brief [AI GENERATED] Generate samples using multiple harmonics with
 *        subtle hammer noise.
 */
std::vector<double> NoteSynth::synthesize(const std::vector<NoteEvent>& events,
                                          int sampleRate) const {
    const double kHammerTime = 0.02;
    const double kAttackTime = 0.005;
    const double kReleaseTime = 0.3;
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

        const int iHammerSamples =
            std::min(static_cast<int>(kHammerTime * sampleRate), iCount);
        const int iAttackSamples =
            std::min(static_cast<int>(kAttackTime * sampleRate), iCount);
        for (int i = 0; i < iCount; ++i) {
            const double dPhase = 2.0 * M_PI * e.frequency * static_cast<double>(i) / sampleRate;
            double dValue = 0.0;
            for (int h = 1; h <= 4; ++h) {
                double hAmp = std::exp(-0.001 * h * i) / h;
                dValue += hAmp * std::sin(h * dPhase);
            }
            if (i < iHammerSamples) {
                const double dNoise = static_cast<double>(std::rand()) / RAND_MAX * 2.0 - 1.0;
                const double dHammerEnv = 1.0 - static_cast<double>(i) / iHammerSamples;
                dValue += 0.005 * dHammerEnv * dNoise;
            }
            double dEnvelope = 1.0;
            if (i < iAttackSamples) {
                dEnvelope *= static_cast<double>(i) / iAttackSamples;
            } else if (i >= iHold) {
                const double dRelease = static_cast<double>(i - iHold);
                const double dReleaseLen = static_cast<double>(iRelease);
                dEnvelope = std::exp(-2.0 * dRelease / dReleaseLen);
            }

            samples[iStart + i] += dEnvelope * dValue;
        }
    }
    double dMax = 0.0;
    for (double s : samples) {
        dMax = std::max(dMax, std::abs(s));
    }
    if (dMax > 1.0) {
        const double dScale = 1.0 / dMax;
        for (double& s : samples) {
            s *= dScale;
        }
    }
    return samples;
}
