#include <gtest/gtest.h>
#include "../core/utils/math_utils.h"
#include <cmath>
#include <vector>

using namespace PianoSynth::Utils;

class MathUtilsTest : public ::testing::Test {
protected:
    const double EPSILON = 1e-6;
};

// Test MIDI to frequency conversion
TEST_F(MathUtilsTest, MidiToFrequencyConversion) {
    // Test known values
    EXPECT_NEAR(MathUtils::midiToFrequency(69), 440.0, EPSILON); // A4
    EXPECT_NEAR(MathUtils::midiToFrequency(60), 261.626, 0.001); // C4
    EXPECT_NEAR(MathUtils::midiToFrequency(21), 27.5, 0.1);      // A0 (lowest piano key)
    EXPECT_NEAR(MathUtils::midiToFrequency(108), 4186.01, 0.01); // C8 (highest piano key)
    
    // Test octave relationships
    EXPECT_NEAR(MathUtils::midiToFrequency(81), MathUtils::midiToFrequency(69) * 2.0, EPSILON); // A5 = 2 * A4
    EXPECT_NEAR(MathUtils::midiToFrequency(57), MathUtils::midiToFrequency(69) / 2.0, EPSILON); // A3 = A4 / 2
}

// Test frequency to MIDI conversion
TEST_F(MathUtilsTest, FrequencyToMidiConversion) {
    // Test round-trip conversion
    for (int midi = 21; midi <= 108; ++midi) {
        double freq = MathUtils::midiToFrequency(midi);
        int converted_midi = MathUtils::frequencyToMidi(freq);
        EXPECT_EQ(converted_midi, midi) << "Failed for MIDI note " << midi;
    }
    
    // Test known frequencies
    EXPECT_EQ(MathUtils::frequencyToMidi(440.0), 69);
    EXPECT_EQ(MathUtils::frequencyToMidi(261.626), 60);
}

// Test interpolation functions
TEST_F(MathUtilsTest, InterpolationFunctions) {
    // Linear interpolation
    EXPECT_NEAR(MathUtils::linearInterpolate(0.0, 10.0, 0.0), 0.0, EPSILON);
    EXPECT_NEAR(MathUtils::linearInterpolate(0.0, 10.0, 1.0), 10.0, EPSILON);
    EXPECT_NEAR(MathUtils::linearInterpolate(0.0, 10.0, 0.5), 5.0, EPSILON);
    EXPECT_NEAR(MathUtils::linearInterpolate(5.0, 15.0, 0.3), 8.0, EPSILON);
    
    // Cosine interpolation should be smooth
    double cos_result = MathUtils::cosineInterpolate(0.0, 10.0, 0.5);
    EXPECT_GT(cos_result, 4.0);
    EXPECT_LT(cos_result, 6.0);
    
    // Cubic interpolation
    double cubic_result = MathUtils::cubicInterpolate(0.0, 5.0, 10.0, 15.0, 0.5);
    EXPECT_GT(cubic_result, 0.0);
    EXPECT_LT(cubic_result, 15.0);
}

// Test window functions
TEST_F(MathUtilsTest, WindowFunctions) {
    const int N = 128;
    
    for (int n = 0; n < N; ++n) {
        double hann = MathUtils::hannWindow(n, N);
        double hamming = MathUtils::hammingWindow(n, N);
        double blackman = MathUtils::blackmanWindow(n, N);
        
        // All window functions should be between 0 and 1
        EXPECT_GE(hann, 0.0);
        EXPECT_LE(hann, 1.0);
        EXPECT_GE(hamming, 0.0);
        EXPECT_LE(hamming, 1.0);
        EXPECT_GE(blackman, 0.0);
        EXPECT_LE(blackman, 1.0);
    }
    
    // Window functions should be symmetric
    EXPECT_NEAR(MathUtils::hannWindow(10, N), MathUtils::hannWindow(N-11, N), EPSILON);
    EXPECT_NEAR(MathUtils::hammingWindow(20, N), MathUtils::hammingWindow(N-21, N), EPSILON);
    
    // Window functions should peak in the middle
    double hann_center = MathUtils::hannWindow(N/2, N);
    double hann_edge = MathUtils::hannWindow(0, N);
    EXPECT_GT(hann_center, hann_edge);
}

// Test dB conversion functions
TEST_F(MathUtilsTest, DbConversion) {
    // Test known conversions
    EXPECT_NEAR(MathUtils::dbToLinear(0.0), 1.0, EPSILON);
    EXPECT_NEAR(MathUtils::dbToLinear(20.0), 10.0, EPSILON);
    EXPECT_NEAR(MathUtils::dbToLinear(-20.0), 0.1, EPSILON);
    EXPECT_NEAR(MathUtils::dbToLinear(-40.0), 0.01, EPSILON);
    
    // Test round-trip conversion
    std::vector<double> test_db_values = {-60.0, -40.0, -20.0, -6.0, 0.0, 6.0, 12.0, 20.0};
    for (double db : test_db_values) {
        double linear = MathUtils::dbToLinear(db);
        double converted_db = MathUtils::linearToDb(linear);
        EXPECT_NEAR(converted_db, db, EPSILON) << "Failed for " << db << " dB";
    }
}

// Test clamp functions
TEST_F(MathUtilsTest, ClampFunctions) {
    // Float clamp
    EXPECT_FLOAT_EQ(MathUtils::clamp(5.0f, 0.0f, 10.0f), 5.0f);
    EXPECT_FLOAT_EQ(MathUtils::clamp(-5.0f, 0.0f, 10.0f), 0.0f);
    EXPECT_FLOAT_EQ(MathUtils::clamp(15.0f, 0.0f, 10.0f), 10.0f);
    
    // Double clamp
    EXPECT_DOUBLE_EQ(MathUtils::clamp(5.0, 0.0, 10.0), 5.0);
    EXPECT_DOUBLE_EQ(MathUtils::clamp(-5.0, 0.0, 10.0), 0.0);
    EXPECT_DOUBLE_EQ(MathUtils::clamp(15.0, 0.0, 10.0), 10.0);
}

// Test physical modeling utilities
TEST_F(MathUtilsTest, PhysicalModelingUtilities) {
    // String wave speed calculation
    double tension = 1000.0; // N
    double linear_density = 0.01; // kg/m
    double wave_speed = MathUtils::calculateStringWaveSpeed(tension, linear_density);
    double expected_speed = sqrt(tension / linear_density);
    EXPECT_NEAR(wave_speed, expected_speed, EPSILON);
    
    // String length calculation
    double frequency = 440.0; // Hz
    double length = MathUtils::calculateStringLength(frequency, tension, linear_density);
    double expected_length = wave_speed / (2.0 * frequency);
    EXPECT_NEAR(length, expected_length, EPSILON);
    
    // Hammer contact time
    double hammer_mass = 0.01; // kg
    double string_stiffness = 1e6; // N/m
    double contact_time = MathUtils::calculateHammerContactTime(hammer_mass, string_stiffness);
    double expected_time = MathUtils::PI * sqrt(hammer_mass / string_stiffness);
    EXPECT_NEAR(contact_time, expected_time, EPSILON);
}

// Test random number generation
TEST_F(MathUtilsTest, RandomNumberGeneration) {
    const int num_samples = 1000;
    
    // Test uniform distribution
    std::vector<double> uniform_samples;
    for (int i = 0; i < num_samples; ++i) {
        double sample = MathUtils::randomUniform(0.0, 1.0);
        uniform_samples.push_back(sample);
        
        // All samples should be in range
        EXPECT_GE(sample, 0.0);
        EXPECT_LE(sample, 1.0);
    }
    
    // Test different range
    for (int i = 0; i < 100; ++i) {
        double sample = MathUtils::randomUniform(-5.0, 5.0);
        EXPECT_GE(sample, -5.0);
        EXPECT_LE(sample, 5.0);
    }
    
    // Test Gaussian distribution
    std::vector<double> gaussian_samples;
    for (int i = 0; i < num_samples; ++i) {
        double sample = MathUtils::randomGaussian(0.0, 1.0);
        gaussian_samples.push_back(sample);
    }
    
    // Calculate sample mean and standard deviation
    double mean = 0.0;
    for (double sample : gaussian_samples) {
        mean += sample;
    }
    mean /= gaussian_samples.size();
    
    double variance = 0.0;
    for (double sample : gaussian_samples) {
        variance += (sample - mean) * (sample - mean);
    }
    variance /= (gaussian_samples.size() - 1);
    double std_dev = sqrt(variance);
    
    // Should be approximately N(0,1)
    EXPECT_NEAR(mean, 0.0, 0.1);
    EXPECT_NEAR(std_dev, 1.0, 0.1);
}

// DSP Utils Tests
class DSPUtilsTest : public ::testing::Test {
protected:
    const double EPSILON = 1e-6;
};

// Test soft clipping
TEST_F(DSPUtilsTest, SoftClipping) {
    // Test values below threshold
    EXPECT_NEAR(DSPUtils::softClip(0.5, 0.7), 0.5, EPSILON);
    EXPECT_NEAR(DSPUtils::softClip(-0.3, 0.7), -0.3, EPSILON);
    
    // Test values above threshold should be compressed
    double clipped_positive = DSPUtils::softClip(1.5, 0.7);
    EXPECT_GT(clipped_positive, 0.7);
    EXPECT_LT(clipped_positive, 1.5);
    
    double clipped_negative = DSPUtils::softClip(-1.5, 0.7);
    EXPECT_LT(clipped_negative, -0.7);
    EXPECT_GT(clipped_negative, -1.5);
}

// Test tanh saturation
TEST_F(DSPUtilsTest, TanhSaturation) {
    // Test with different drive values
    double input = 0.5;
    
    double result1 = DSPUtils::tanhSaturation(input, 1.0);
    double result2 = DSPUtils::tanhSaturation(input, 2.0);
    
    // Higher drive should give more saturation
    EXPECT_NE(result1, result2);
    
    // Output should be bounded
    EXPECT_GT(result1, -1.0);
    EXPECT_LT(result1, 1.0);
    EXPECT_GT(result2, -1.0);
    EXPECT_LT(result2, 1.0);
}

// Test ADSR envelope
TEST_F(DSPUtilsTest, ADSREnvelope) {
    double attack = 0.1;
    double decay = 0.2;
    double sustain = 0.7;
    double release = 0.3;
    double note_on_time = 0.0;
    double note_off_time = 1.0;
    
    // Test attack phase
    double attack_value = DSPUtils::adsr(0.05, attack, decay, sustain, release, note_on_time, -1);
    EXPECT_GT(attack_value, 0.0);
    EXPECT_LT(attack_value, 1.0);
    
    // Test sustain phase
    double sustain_value = DSPUtils::adsr(0.5, attack, decay, sustain, release, note_on_time, -1);
    EXPECT_NEAR(sustain_value, sustain, 0.1);
    
    // Test release phase
    double release_value = DSPUtils::adsr(1.1, attack, decay, sustain, release, note_on_time, note_off_time);
    EXPECT_LT(release_value, sustain);
    EXPECT_GT(release_value, 0.0);
    
    // Test end of release
    double end_value = DSPUtils::adsr(2.0, attack, decay, sustain, release, note_on_time, note_off_time);
    EXPECT_NEAR(end_value, 0.0, EPSILON);
}

// Test delay processing
TEST_F(DSPUtilsTest, DelayProcessing) {
    const int buffer_size = 100;
    const int delay_samples = 10;
    const double feedback = 0.3;
    
    std::vector<double> delay_buffer(buffer_size, 0.0);
    int write_index = 0;
    
    // Process impulse
    double output1 = DSPUtils::processDelay(delay_buffer, write_index, 1.0, delay_samples, feedback);
    EXPECT_NEAR(output1, 0.0, EPSILON); // First output should be silence
    
    // Process silence for (delay_samples - 1) steps
    for (int i = 0; i < delay_samples - 1; ++i) {
        DSPUtils::processDelay(delay_buffer, write_index, 0.0, delay_samples, feedback);
    }
    
    // Now we should get the delayed signal
    double delayed_output = DSPUtils::processDelay(delay_buffer, write_index, 0.0, delay_samples, feedback);
    EXPECT_GT(delayed_output, 0.0);
    EXPECT_LE(delayed_output, 1.0);
}

// [AI GENERATED] Test cents to ratio conversion
TEST_F(MathUtilsTest, CentsToRatioConversion) {
    EXPECT_NEAR(MathUtils::centsToRatio(0.0), 1.0, EPSILON);
    EXPECT_NEAR(MathUtils::centsToRatio(1200.0), 2.0, EPSILON);
    EXPECT_NEAR(MathUtils::centsToRatio(-1200.0), 0.5, EPSILON);
    EXPECT_NEAR(MathUtils::centsToRatio(100.0), pow(2.0, 100.0 / 1200.0), EPSILON);
}
