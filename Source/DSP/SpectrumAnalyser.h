#pragma once
#include <JuceHeader.h>
#include <vector>
#include <array>
#include <atomic>

class SpectrumAnalyser
{
public:
    static constexpr int fftOrder = 11; // 2^11 = 2048 points
    static constexpr int fftSize = 1 << fftOrder;

    struct BandResult
    {
        float peakDbL = -100.0f;
        float rmsDbL = -100.0f;
        float peakDbR = -100.0f;
        float rmsDbR = -100.0f;
    };

    SpectrumAnalyser();

    void prepare(double sampleRate);
    void pushBlock(const juce::AudioBuffer<float>& buffer);
    std::vector<BandResult> getBandData() const;

    const std::vector<float>& getApiFrequencies() const { return apiFrequencies; }

    // méthode pour modifier les release
    void setReleaseTimes(float peakSec, float rmsSec)
    {
        releaseTimeSec = peakSec;
        rmsReleaseTimeSec = rmsSec;
    }

private:
    void calculateBands();
    void applyTilt(float* fftData, int numBins);

    double currentSampleRate = 44100.0;
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    std::vector<float> apiFrequencies;
    std::vector<float> tiltFactors;

    // Buffers circulaires et données FFT pour L et R
    std::array<std::array<float, fftSize>, 2> fifo;
    std::array<std::array<float, fftSize * 2>, 2> fftData;
    int fifoIndex = 0;

    std::array<std::vector<float>, 2> magnitudes;
    std::vector<BandResult> currentResults;
    mutable juce::CriticalSection lock;

    std::vector<float> smoothedPeakL;
    std::vector<float> smoothedPeakR;

    float releaseTimeSec = 0.75f;

    std::vector<float> smoothedRmsL;
    std::vector<float> smoothedRmsR;
    float rmsReleaseTimeSec = 0.3f;

    
};