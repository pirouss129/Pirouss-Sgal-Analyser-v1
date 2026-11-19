#include "SpectrumAnalyser.h"

SpectrumAnalyser::SpectrumAnalyser()
    : forwardFFT(fftOrder),
    window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    apiFrequencies = {
        20.0f,  25.0f,  31.5f,  40.0f,  50.0f,  63.0f,  80.0f,  100.0f,
        125.0f, 160.0f, 200.0f, 250.0f, 315.0f, 400.0f, 500.0f, 630.0f,
        800.0f, 1000.0f, 1250.0f, 1600.0f, 2000.0f, 2500.0f, 3150.0f, 4000.0f,
        5000.0f, 6300.0f, 8000.0f, 10000.0f, 12500.0f, 16000.0f, 20000.0f
    };

    size_t numBands = apiFrequencies.size();
    magnitudes[0].resize(fftSize / 2, 0.0f);
    magnitudes[1].resize(fftSize / 2, 0.0f);
    currentResults.resize(numBands);
    tiltFactors.resize(fftSize / 2, 1.0f);

    smoothedPeakL.assign(numBands, -100.0f);
    smoothedPeakR.assign(numBands, -100.0f);
    smoothedRmsL.assign(numBands, -100.0f);
    smoothedRmsR.assign(numBands, -100.0f);
}

void SpectrumAnalyser::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    const int numBins = fftSize / 2;
    const float binWidth = static_cast<float>(sampleRate) / static_cast<float>(fftSize);

    // Calcul des facteurs de Tilt (+3 dB/octave par rapport à 1 kHz)
    for (int bin = 0; bin < numBins; ++bin)
    {
        float freq = static_cast<float>(bin) * binWidth;
        if (freq < 1.0f) { tiltFactors[bin] = 1.0f; continue; }
        float octaves = std::log2(freq / 1000.0f);
        tiltFactors[bin] = juce::Decibels::decibelsToGain(octaves * 3.0f);
    }
}

void SpectrumAnalyser::pushBlock(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = std::min(buffer.getNumChannels(), 2);

    for (int i = 0; i < numSamples; ++i)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float sample = buffer.getReadPointer(ch)[i];
            fifo[ch][static_cast<size_t>(fifoIndex)] = sample;
        }

        fifoIndex++;

        if (fifoIndex >= fftSize)
        {
            fifoIndex = 0;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                std::copy(fifo[ch].begin(), fifo[ch].end(), fftData[ch].begin());
                window.multiplyWithWindowingTable(fftData[ch].data(), fftSize);
                forwardFFT.performFrequencyOnlyForwardTransform(fftData[ch].data());

                const int numBins = fftSize / 2;
                for (int b = 0; b < numBins; ++b)
                {
                    fftData[ch][b] /= static_cast<float>(fftSize);
                    magnitudes[ch][b] = fftData[ch][b];
                }
                applyTilt(magnitudes[ch].data(), numBins);
            }

            calculateBands();
        }
    }
}

void SpectrumAnalyser::applyTilt(float* data, int numBins)
{
    for (int i = 0; i < numBins; ++i)
        data[i] *= tiltFactors[i];
}

void SpectrumAnalyser::calculateBands()
{
    size_t numBands = apiFrequencies.size();
    const int numBins = fftSize / 2;
    const float binWidth = static_cast<float>(currentSampleRate) / static_cast<float>(fftSize);

    std::vector<float> instMagL(numBands), instMagR(numBands);

    // Échantillonnage direct et interpolé pour chaque bande ISO
    for (size_t b = 0; b < numBands; ++b)
    {
        float freq = apiFrequencies[b];
        float exactBin = freq / binWidth;

        auto interpolate = [&](const std::vector<float>& mags) -> float {
            if (exactBin <= 0.0f) return mags[0];
            if (exactBin >= numBins - 1) return mags[numBins - 1];

            int lowBin = static_cast<int>(std::floor(exactBin));
            int highBin = lowBin + 1;
            float frac = exactBin - lowBin;

            return mags[lowBin] + frac * (mags[highBin] - mags[lowBin]);
            };

        instMagL[b] = interpolate(magnitudes[0]);
        instMagR[b] = interpolate(magnitudes[1]);
    }

    float dt = static_cast<float>(fftSize) / static_cast<float>(currentSampleRate);

    float releaseRateDbPerSec = 10.0f / releaseTimeSec;
    float maxDropDbPeak = releaseRateDbPerSec * dt;
    float rmsAlpha = 1.0f - std::exp(-dt / rmsReleaseTimeSec);

    const juce::ScopedLock sl(lock);
    for (size_t b = 0; b < numBands; ++b)
    {
        float instantPeakDbL = juce::Decibels::gainToDecibels(instMagL[b], -100.0f);
        float instantPeakDbR = juce::Decibels::gainToDecibels(instMagR[b], -100.0f);
        float instantRmsDbL = instantPeakDbL;
        float instantRmsDbR = instantPeakDbR;

        // --- PEAK ---
        if (instantPeakDbL >= smoothedPeakL[b]) smoothedPeakL[b] = instantPeakDbL;
        else smoothedPeakL[b] = std::max(-100.0f, smoothedPeakL[b] - maxDropDbPeak);

        if (instantPeakDbR >= smoothedPeakR[b]) smoothedPeakR[b] = instantPeakDbR;
        else smoothedPeakR[b] = std::max(-100.0f, smoothedPeakR[b] - maxDropDbPeak);

        // --- RMS ---
        smoothedRmsL[b] += rmsAlpha * (instantRmsDbL - smoothedRmsL[b]);
        smoothedRmsR[b] += rmsAlpha * (instantRmsDbR - smoothedRmsR[b]);

        currentResults[b].peakDbL = smoothedPeakL[b];
        currentResults[b].rmsDbL = smoothedRmsL[b];
        currentResults[b].peakDbR = smoothedPeakR[b];
        currentResults[b].rmsDbR = smoothedRmsR[b];
    }
}

std::vector<SpectrumAnalyser::BandResult> SpectrumAnalyser::getBandData() const
{
    const juce::ScopedLock sl(lock);
    return currentResults;
}