#pragma once
#include <JuceHeader.h>
#include <vector>

class VerticalSpectrumComponent : public juce::Component
{
public:
    VerticalSpectrumComponent();
    ~VerticalSpectrumComponent() override = default;

    // Structure de données simple partagée ou définie ici
    struct BandData {
        float peakDbL = -100.0f;
        float rmsDbL = -100.0f;
        float peakDbR = -100.0f;
        float rmsDbR = -100.0f;
    };

    void setThresholds(float cyan, float green, float yellow, float orange, float red)
    {
        threshCyan = juce::jlimit(-80.0f, 0.0f, cyan);
        threshGreen = juce::jlimit(-80.0f, 0.0f, green);
        threshYellow = juce::jlimit(-80.0f, 0.0f, yellow);
        threshOrange = juce::jlimit(-80.0f, 0.0f, orange);
        threshRed = juce::jlimit(-80.0f, 0.0f, red);
    }

    // L'UI reçoit simplement un tableau prêt à être dessiné
    void updateData(const std::vector<BandData>& newData, const std::vector<float>& frequencies);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    float threshCyan = -70.0f;
    float threshGreen = -60.0f;
    float threshYellow = -50.0f;
    float threshOrange = -32.0f;
    float threshRed = -18.0f;

    std::vector<BandData> currentData;
    std::vector<float> apiFreqs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerticalSpectrumComponent)
};