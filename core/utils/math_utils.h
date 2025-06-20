#pragma once

#include <cmath>
#include <complex>
#include <vector>

namespace PianoSynth {
namespace Utils {

/**
 * Mathematical utilities for physical modeling and audio processing
 */
class MathUtils {
public:
    // Constants
    static constexpr double PI = 3.14159265358979323846;
    static constexpr double TWO_PI = 2.0 * PI;
    static constexpr double HALF_PI = PI / 2.0;
    
    // MIDI to frequency conversion
    static double midiToFrequency(int midi_note);
    static int frequencyToMidi(double frequency);

    /**
     * \brief [AI GENERATED] Convert tuning offset in cents to a frequency ratio.
     */
    static double centsToRatio(double cents);
    
    // Interpolation
    static double linearInterpolate(double y1, double y2, double x);
    static double cubicInterpolate(double y0, double y1, double y2, double y3, double x);
    static double cosineInterpolate(double y1, double y2, double x);
    
    // Window functions
    static double hannWindow(int n, int N);
    static double hammingWindow(int n, int N);
    static double blackmanWindow(int n, int N);
    
    // Audio utilities
    static double dbToLinear(double db);
    static double linearToDb(double linear);
    static float clamp(float value, float min_val, float max_val);
    static double clamp(double value, double min_val, double max_val);
    
    // Physical modeling utilities
    static double calculateStringWaveSpeed(double tension, double linear_density);
    static double calculateStringLength(double frequency, double tension, double linear_density);
    static double calculateHammerContactTime(double hammer_mass, double string_stiffness);
    
    // Numerical methods
    static double rk4Step(double (*f)(double, double), double t, double y, double h);
    static std::vector<double> solveThomasAlgorithm(const std::vector<double>& a,
                                                   const std::vector<double>& b,
                                                   const std::vector<double>& c,
                                                   const std::vector<double>& d);
    
    // Random number generation
    static double randomUniform(double min = 0.0, double max = 1.0);
    static double randomGaussian(double mean = 0.0, double std_dev = 1.0);
    
private:
    MathUtils() = delete; // Static class
};

/**
 * DSP utilities for audio processing
 */
class DSPUtils {
public:
    // Filter design
    static void designLowpass(std::vector<double>& b, std::vector<double>& a, 
                             double cutoff, double sample_rate, int order = 2);
    static void designHighpass(std::vector<double>& b, std::vector<double>& a,
                              double cutoff, double sample_rate, int order = 2);
    static void designBandpass(std::vector<double>& b, std::vector<double>& a,
                              double low_cutoff, double high_cutoff, 
                              double sample_rate, int order = 2);
    
    // FFT (simple implementation for analysis)
    static void fft(std::vector<std::complex<double>>& data);
    static void ifft(std::vector<std::complex<double>>& data);
    
    // Convolution
    static std::vector<double> convolve(const std::vector<double>& signal,
                                       const std::vector<double>& impulse);
    
    // Delay effects
    static double processDelay(std::vector<double>& delay_buffer, int& write_index,
                              double input, int delay_samples, double feedback = 0.0);
    
    // Saturation/distortion
    static double softClip(double input, double threshold = 0.7);
    static double tanhSaturation(double input, double drive = 1.0);
    
    // Envelope functions
    static double adsr(double time, double attack, double decay, double sustain, double release,
                      double note_on_time, double note_off_time);
    
private:
    DSPUtils() = delete; // Static class
    
    static void fftRecursive(std::vector<std::complex<double>>& data, bool inverse);
};

} // namespace Utils
} // namespace PianoSynth
