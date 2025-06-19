#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cstdint>

// Simple working oscillator to replace the broken string model
class SimpleStringOscillator {
private:
    double frequency_;
    double sample_rate_;
    double phase_;
    double amplitude_;
    double decay_rate_;
    double time_;
    
public:
    SimpleStringOscillator(double freq, double sample_rate) 
        : frequency_(freq), sample_rate_(sample_rate), phase_(0.0), 
          amplitude_(0.0), decay_rate_(0.5), time_(0.0) {}
    
    void strike(double force) {
        amplitude_ = force;
        phase_ = 0.0;
        time_ = 0.0;
    }
    
    double step() {
        double dt = 1.0 / sample_rate_;
        time_ += dt;
        
        // Generate basic sine wave
        double fundamental = sin(2.0 * M_PI * frequency_ * time_);
        
        // Add some harmonics for richer sound
        double harmonic2 = 0.3 * sin(2.0 * M_PI * frequency_ * 2.0 * time_);
        double harmonic3 = 0.1 * sin(2.0 * M_PI * frequency_ * 3.0 * time_);
        
        double signal = fundamental + harmonic2 + harmonic3;
        
        // Apply exponential decay
        double envelope = amplitude_ * exp(-decay_rate_ * time_);
        
        return signal * envelope;
    }
};

// Test the simple oscillator vs complex string model
int main() {
    std::cout << "🎹 Simple String Oscillator Test" << std::endl;
    std::cout << "=================================" << std::endl;
    
    const double sample_rate = 44100.0;
    const double frequency = 261.626; // Middle C
    const int num_samples = 2000;
    
    SimpleStringOscillator osc(frequency, sample_rate);
    osc.strike(0.5); // Moderate strike
    
    std::vector<double> samples;
    samples.reserve(num_samples);
    
    std::cout << "Generating " << num_samples << " samples at " << frequency << " Hz..." << std::endl;
    
    for (int i = 0; i < num_samples; ++i) {
        double sample = osc.step();
        samples.push_back(sample);
        
        // Print first 20 samples
        if (i < 20) {
            std::cout << "Sample " << i << ": " << sample << std::endl;
        }
    }
    
    // Analyze the output
    double min_val = *std::min_element(samples.begin(), samples.end());
    double max_val = *std::max_element(samples.begin(), samples.end());
    double peak = std::max(std::abs(min_val), std::abs(max_val));
    
    double rms = 0.0;
    for (double s : samples) {
        rms += s * s;
    }
    rms = std::sqrt(rms / samples.size());
    
    // Count zero crossings to estimate frequency
    int zero_crossings = 0;
    for (size_t i = 1; i < samples.size(); ++i) {
        if ((samples[i] > 0) != (samples[i-1] > 0)) {
            zero_crossings++;
        }
    }
    
    double estimated_freq = (zero_crossings / 2.0) * sample_rate / num_samples;
    
    std::cout << "\n📊 Analysis Results:" << std::endl;
    std::cout << "  Peak amplitude: " << peak << std::endl;
    std::cout << "  RMS amplitude: " << rms << std::endl;
    std::cout << "  Zero crossings: " << zero_crossings << std::endl;
    std::cout << "  Estimated frequency: " << estimated_freq << " Hz" << std::endl;
    std::cout << "  Expected frequency: " << frequency << " Hz" << std::endl;
    std::cout << "  Frequency error: " << std::abs(estimated_freq - frequency) << " Hz" << std::endl;
    
    // Check if it's working correctly
    std::cout << "\n🔍 Diagnosis:" << std::endl;
    
    if (peak == 0.0) {
        std::cout << "  ❌ PROBLEM: No output signal!" << std::endl;
    } else if (peak < 0.01) {
        std::cout << "  ⚠️  PROBLEM: Output very weak" << std::endl;
    } else {
        std::cout << "  ✅ Output level good" << std::endl;
    }
    
    if (std::abs(estimated_freq - frequency) < 5.0) {
        std::cout << "  ✅ Frequency accurate" << std::endl;
    } else {
        std::cout << "  ❌ PROBLEM: Frequency inaccurate" << std::endl;
    }
    
    if (zero_crossings > 10) {
        std::cout << "  ✅ Oscillating properly" << std::endl;
    } else {
        std::cout << "  ❌ PROBLEM: Not oscillating" << std::endl;
    }
    
    // Save to file for inspection
    std::ofstream outfile("simple_oscillator_output.txt");
    outfile << "# Simple oscillator output\n";
    outfile << "# Sample_number\tAmplitude\n";
    for (size_t i = 0; i < samples.size(); ++i) {
        outfile << i << "\t" << samples[i] << "\n";
    }
    outfile.close();
    
    // Create a simple WAV file
    std::ofstream wav_file("simple_oscillator_test.wav", std::ios::binary);
    
    // Calculate WAV header values
    uint32_t sample_rate_int = static_cast<uint32_t>(sample_rate);
    uint32_t subchunk2_size = samples.size() * 2;
    uint32_t chunk_size = 36 + subchunk2_size;
    uint32_t byte_rate = sample_rate_int * 1 * 2;
    
    // WAV header
    struct {
        char chunk_id[4] = {'R', 'I', 'F', 'F'};
        uint32_t chunk_size;
        char format[4] = {'W', 'A', 'V', 'E'};
        char subchunk1_id[4] = {'f', 'm', 't', ' '};
        uint32_t subchunk1_size = 16;
        uint16_t audio_format = 1; // PCM
        uint16_t num_channels = 1; // Mono
        uint32_t sample_rate_val;
        uint32_t byte_rate_val;
        uint16_t block_align = 2;
        uint16_t bits_per_sample = 16;
        char subchunk2_id[4] = {'d', 'a', 't', 'a'};
        uint32_t subchunk2_size_val;
    } header;
    
    header.chunk_size = chunk_size;
    header.sample_rate_val = sample_rate_int;
    header.byte_rate_val = byte_rate;
    header.subchunk2_size_val = subchunk2_size;
    
    wav_file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    // Write audio data
    for (double sample : samples) {
        int16_t int_sample = static_cast<int16_t>(std::clamp(sample * 10000.0, -32767.0, 32767.0));
        wav_file.write(reinterpret_cast<const char*>(&int_sample), sizeof(int_sample));
    }
    wav_file.close();
    
    std::cout << "\n📁 Files created:" << std::endl;
    std::cout << "  - simple_oscillator_output.txt (raw data)" << std::endl;
    std::cout << "  - simple_oscillator_test.wav (audio file)" << std::endl;
    
    std::cout << "\n🎯 Summary:" << std::endl;
    if (peak > 0.01 && zero_crossings > 10 && std::abs(estimated_freq - frequency) < 5.0) {
        std::cout << "  ✅ Simple oscillator working correctly!" << std::endl;
        std::cout << "  🔧 This can replace the broken string model" << std::endl;
    } else {
        std::cout << "  ❌ Issues with simple oscillator too" << std::endl;
    }
    
    return 0;
}