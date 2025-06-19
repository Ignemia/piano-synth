#include "../../core/utils/math_utils.h"
#include <cmath>

namespace PianoSynth {
namespace Utils {

double MathUtils::midiToFrequency(int midi_note) {
    return 440.0 * std::pow(2.0, (midi_note - 69) / 12.0);
}

double MathUtils::frequencyToMidi(double frequency) {
    return 69.0 + 12.0 * std::log2(frequency / 440.0);
}

double MathUtils::semitoneRatio(double semitones) {
    return std::pow(2.0, semitones / 12.0);
}

double MathUtils::dbToLinear(double db) {
    return std::pow(10.0, db / 20.0);
}

double MathUtils::linearToDb(double linear) {
    return 20.0 * std::log10(linear);
}

double MathUtils::clamp(double value, double min, double max) {
    return std::max(min, std::min(max, value));
}

double MathUtils::lerp(double a, double b, double t) {
    return a + t * (b - a);
}

double MathUtils::normalize(double value, double min, double max) {
    return (value - min) / (max - min);
}

double MathUtils::ease(double t, EaseFunction function) {
    switch (function) {
        case EASE_LINEAR: return t;
        case EASE_QUAD_IN: return t * t;
        case EASE_QUAD_OUT: return 1.0 - (1.0 - t) * (1.0 - t);
        case EASE_CUBIC_IN: return t * t * t;
        case EASE_CUBIC_OUT: return 1.0 - std::pow(1.0 - t, 3.0);
        case EASE_EXPONENTIAL: return t == 0.0 ? 0.0 : std::pow(2.0, 10.0 * (t - 1.0));
        default: return t;
    }
}

void MathUtils::fft(std::vector<std::complex<double>>& data) {
    fftRecursive(data, false);
}

void MathUtils::ifft(std::vector<std::complex<double>>& data) {
    fftRecursive(data, true);
    // Normalize
    for (auto& sample : data) {
        sample /= data.size();
    }
}

void MathUtils::fftRecursive(std::vector<std::complex<double>>& data, bool inverse) {
    size_t n = data.size();
    if (n <= 1) return;
    
    // Divide
    std::vector<std::complex<double>> even(n / 2), odd(n / 2);
    for (size_t i = 0; i < n / 2; i++) {
        even[i] = data[i * 2];
        odd[i] = data[i * 2 + 1];
    }
    
    // Conquer
    fftRecursive(even, inverse);
    fftRecursive(odd, inverse);
    
    // Combine
    for (size_t i = 0; i < n / 2; i++) {
        std::complex<double> t = std::polar(1.0, (inverse ? 2.0 : -2.0) * PI * i / n) * odd[i];
        data[i] = even[i] + t;
        data[i + n / 2] = even[i] - t;
    }
}

} // namespace Utils
} // namespace PianoSynth
