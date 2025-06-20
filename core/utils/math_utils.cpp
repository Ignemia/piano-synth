#include "math_utils.h"
#include <random>
#include <algorithm>
#include <cmath>

namespace PianoSynth {
namespace Utils {

// Static random number generator
static std::random_device rd;
static std::mt19937 gen(rd());

double MathUtils::midiToFrequency(int midi_note) {
    return 440.0 * pow(2.0, (midi_note - 69) / 12.0);
}

int MathUtils::frequencyToMidi(double frequency) {
    return static_cast<int>(round(69.0 + 12.0 * log2(frequency / 440.0)));
}

/**
 * \brief [AI GENERATED] Convert cent offsets to a multiplicative ratio.
 */
double MathUtils::centsToRatio(double cents) {
    return pow(2.0, cents / 1200.0);
}

double MathUtils::linearInterpolate(double y1, double y2, double x) {
    return y1 + x * (y2 - y1);
}

double MathUtils::cubicInterpolate(double y0, double y1, double y2, double y3, double x) {
    double a0 = y3 - y2 - y0 + y1;
    double a1 = y0 - y1 - a0;
    double a2 = y2 - y0;
    double a3 = y1;
    return a0 * x * x * x + a1 * x * x + a2 * x + a3;
}

double MathUtils::cosineInterpolate(double y1, double y2, double x) {
    double x2 = (1.0 - cos(x * PI)) / 2.0;
    return y1 * (1.0 - x2) + y2 * x2;
}

double MathUtils::hannWindow(int n, int N) {
    return 0.5 * (1.0 - cos(TWO_PI * n / (N - 1)));
}

double MathUtils::hammingWindow(int n, int N) {
    return 0.54 - 0.46 * cos(TWO_PI * n / (N - 1));
}

double MathUtils::blackmanWindow(int n, int N) {
    double val = 0.42 - 0.5 * cos(TWO_PI * n / (N - 1)) + 0.08 * cos(4.0 * PI * n / (N - 1));
    // Clamp to zero if very close to zero (floating-point error)
    return val < 0.0 && val > -1e-12 ? 0.0 : val;
}

double MathUtils::dbToLinear(double db) {
    return pow(10.0, db / 20.0);
}

double MathUtils::linearToDb(double linear) {
    return 20.0 * log10(std::max(linear, 1e-10));
}

float MathUtils::clamp(float value, float min_val, float max_val) {
    return std::max(min_val, std::min(value, max_val));
}

double MathUtils::clamp(double value, double min_val, double max_val) {
    return std::max(min_val, std::min(value, max_val));
}

double MathUtils::calculateStringWaveSpeed(double tension, double linear_density) {
    return sqrt(tension / linear_density);
}

double MathUtils::calculateStringLength(double frequency, double tension, double linear_density) {
    double wave_speed = calculateStringWaveSpeed(tension, linear_density);
    return wave_speed / (2.0 * frequency); // Fundamental frequency = wave_speed / (2 * length)
}

double MathUtils::calculateHammerContactTime(double hammer_mass, double string_stiffness) {
    return PI * sqrt(hammer_mass / string_stiffness);
}

double MathUtils::randomUniform(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(gen);
}

double MathUtils::randomGaussian(double mean, double std_dev) {
    std::normal_distribution<double> dist(mean, std_dev);
    return dist(gen);
}

// DSP Utils Implementation

/**
 * Apply a simple soft clipping curve that smoothly limits the signal
 * to the range [-1, 1]. [AI GENERATED]
 */
double DSPUtils::softClip(double input, double threshold) {
    double abs_input = std::abs(input);
    if (abs_input <= threshold) {
        return input;
    } else {
        double sign = (input >= 0) ? 1.0 : -1.0;
        // Improved soft clipping with smoother transition
        double excess = abs_input - threshold;
        double compressed = threshold + excess / (1.0 + excess * 2.0);
        compressed = std::min(compressed, 1.0); // Ensure output never exceeds 1
        return sign * compressed;
    }
}

double DSPUtils::tanhSaturation(double input, double drive) {
    // Improved tanh saturation with better scaling
    if (drive <= 0.0) return input;
    double scaled_input = input * drive;
    return tanh(scaled_input) / drive;
}

double DSPUtils::adsr(double time, double attack, double decay, double sustain, double release,
                     double note_on_time, double note_off_time) {
    double envelope = 0.0;
    double relative_time = time - note_on_time;
    
    if (note_off_time < 0) { // Note still on
        if (relative_time < attack) {
            // Attack phase
            envelope = relative_time / attack;
        } else if (relative_time < attack + decay) {
            // Decay phase
            double decay_time = relative_time - attack;
            envelope = 1.0 - (1.0 - sustain) * (decay_time / decay);
        } else {
            // Sustain phase
            envelope = sustain;
        }
    } else { // Note off
        double release_time = time - note_off_time;
        if (release_time < release) {
            // Release phase
            double sustain_level = adsr(note_off_time, attack, decay, sustain, release, note_on_time, -1);
            envelope = sustain_level * (1.0 - release_time / release);
        } else {
            envelope = 0.0;
        }
    }
    
    return MathUtils::clamp(envelope, 0.0, 1.0);
}

double DSPUtils::processDelay(std::vector<double>& delay_buffer, int& write_index,
                             double input, int delay_samples, double feedback) {
    // Calculate read index
    int read_index = write_index - delay_samples;
    if (read_index < 0) {
        read_index += delay_buffer.size();
    }
    
    // Get delayed sample
    double delayed_sample = delay_buffer[read_index];
    
    // Write new sample with feedback
    delay_buffer[write_index] = input + feedback * delayed_sample;
    
    // Update write index
    write_index = (write_index + 1) % delay_buffer.size();
    
    return delayed_sample;
}

void DSPUtils::designLowpass(std::vector<double>& b, std::vector<double>& a, 
                            double cutoff, double sample_rate, int order) {
    // Clamp order to reasonable range (1-4)
    order = static_cast<int>(MathUtils::clamp(static_cast<double>(order), 1.0, 4.0));
    
    // Simple first-order lowpass (higher orders would require more complex design)
    double rc = 1.0 / (2.0 * MathUtils::PI * cutoff);
    double alpha = 1.0 / (1.0 + rc * sample_rate);
    
    // Apply order effect by cascading the same filter
    double effective_alpha = std::pow(alpha, 1.0 / order);
    
    b = {effective_alpha};
    a = {1.0, effective_alpha - 1.0};
}

// Additional DSP functions would be implemented here...

} // namespace Utils
} // namespace PianoSynth
