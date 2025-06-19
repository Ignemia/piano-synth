#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "core/physics/string_model.h"
#include "core/utils/constants.h"

using namespace PianoSynth;

int main() {
    std::cout << "🔍 String Model Output Analysis" << std::endl;
    std::cout << "===============================" << std::endl;
    
    // Create a string model for middle C
    Physics::StringModel string(60);
    string.initialize(44100.0);
    
    std::cout << "String properties:" << std::endl;
    std::cout << "  Fundamental frequency: " << string.getFundamentalFrequency() << " Hz" << std::endl;
    std::cout << "  String length: " << string.getLength() << " m" << std::endl;
    std::cout << "  String tension: " << string.getTension() << " N" << std::endl;
    
    // Strike the string
    std::cout << "\nStriking string with moderate force..." << std::endl;
    string.excite(0.125, 2.0, 0.001);
    
    // Generate samples and analyze
    std::vector<double> samples;
    const int num_samples = 1000;
    
    std::cout << "Generating " << num_samples << " samples..." << std::endl;
    
    for (int i = 0; i < num_samples; ++i) {
        double sample = string.step();
        samples.push_back(sample);
        
        // Print first few samples
        if (i < 20) {
            std::cout << "Sample " << i << ": " << sample << std::endl;
        }
    }
    
    // Analyze the output
    double min_val = *std::min_element(samples.begin(), samples.end());
    double max_val = *std::max_element(samples.begin(), samples.end());
    double peak = std::max(std::abs(min_val), std::abs(max_val));
    
    double sum = 0.0;
    for (double s : samples) {
        sum += s * s;
    }
    double rms = std::sqrt(sum / samples.size());
    
    // Check for problems
    int zero_count = 0;
    int nan_count = 0;
    int inf_count = 0;
    
    for (double s : samples) {
        if (s == 0.0) zero_count++;
        if (std::isnan(s)) nan_count++;
        if (std::isinf(s)) inf_count++;
    }
    
    std::cout << "\n📊 Analysis Results:" << std::endl;
    std::cout << "  Peak amplitude: " << peak << std::endl;
    std::cout << "  RMS amplitude: " << rms << std::endl;
    std::cout << "  Min value: " << min_val << std::endl;
    std::cout << "  Max value: " << max_val << std::endl;
    std::cout << "  Zero samples: " << zero_count << "/" << num_samples << std::endl;
    std::cout << "  NaN samples: " << nan_count << std::endl;
    std::cout << "  Inf samples: " << inf_count << std::endl;
    
    // Check if output is reasonable
    std::cout << "\n🔍 Diagnosis:" << std::endl;
    
    if (peak == 0.0) {
        std::cout << "  ❌ PROBLEM: No output signal! String model is silent." << std::endl;
    } else if (peak < 1e-6) {
        std::cout << "  ⚠️  PROBLEM: Output extremely weak (< 1e-6)" << std::endl;
    } else if (peak > 100.0) {
        std::cout << "  ⚠️  PROBLEM: Output extremely large (> 100)" << std::endl;
    } else {
        std::cout << "  ✅ Output level seems reasonable" << std::endl;
    }
    
    if (nan_count > 0) {
        std::cout << "  ❌ PROBLEM: NaN values detected!" << std::endl;
    }
    
    if (inf_count > 0) {
        std::cout << "  ❌ PROBLEM: Infinite values detected!" << std::endl;
    }
    
    if (zero_count > num_samples * 0.9) {
        std::cout << "  ❌ PROBLEM: >90% of samples are zero!" << std::endl;
    }
    
    // Check for static-like behavior
    double variance = 0.0;
    double mean = 0.0;
    for (double s : samples) {
        mean += s;
    }
    mean /= samples.size();
    
    for (double s : samples) {
        variance += (s - mean) * (s - mean);
    }
    variance /= samples.size();
    
    std::cout << "  Signal variance: " << variance << std::endl;
    
    if (variance < 1e-12) {
        std::cout << "  ❌ PROBLEM: Very low variance - signal might be constant/static" << std::endl;
    }
    
    // Save raw output for inspection
    std::ofstream outfile("string_debug_output.txt");
    outfile << "# String model raw output\n";
    outfile << "# Sample_number\tAmplitude\n";
    for (size_t i = 0; i < samples.size(); ++i) {
        outfile << i << "\t" << samples[i] << "\n";
    }
    outfile.close();
    
    std::cout << "\n📁 Raw output saved to: string_debug_output.txt" << std::endl;
    std::cout << "\n🎯 Summary:" << std::endl;
    
    if (peak > 1e-6 && nan_count == 0 && inf_count == 0 && variance > 1e-12) {
        std::cout << "  ✅ String model appears to be working correctly" << std::endl;
        std::cout << "  🔍 Problem likely in synthesis pipeline or audio processing" << std::endl;
    } else {
        std::cout << "  ❌ String model has fundamental issues" << std::endl;
        std::cout << "  🔧 Needs debugging at the physics level" << std::endl;
    }
    
    return 0;
}