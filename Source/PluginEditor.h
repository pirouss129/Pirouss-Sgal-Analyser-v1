/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/VerticalSpectrumComponent.h"
#include "UI/Header.h"
#include "UI/RotSlider.h"
#include "UI/SgalRotSlider.h"

//==============================================================================
/**
*/
class PiroussSpectrumAnalyzerv1AudioProcessorEditor  : public juce::AudioProcessorEditor, 
    public juce::Timer, // hériter de Timer
    public juce::AudioProcessorValueTreeState::Listener // hériter de APVTS Listener

{
public:
    PiroussSpectrumAnalyzerv1AudioProcessorEditor (PiroussSpectrumAnalyzerv1AudioProcessor&);
    ~PiroussSpectrumAnalyzerv1AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void parameterChanged(const juce::String& parameterID, float newValue) override; // <-- La méthode appelée en cas de changement
    void updateThresholdsFromAPVTS();

    void mouseDoubleClick(const juce::MouseEvent& event);
    void mouseDown(const juce::MouseEvent& event);

private:
    
    juce::String pluginVersion = JucePlugin_VersionString;

    const int defaultWidth = 350;
    const int defaultHeight = 500;
    const int minWidth = 250;
    const int minHeight = 400;
    const int maxWidth = 800;
    const int maxHeight = 1000;

    int headerHeight = 55;
    int commandHeight = 100;

    PiroussSpectrumAnalyzerv1AudioProcessor& audioProcessor;

    VerticalSpectrumComponent spectrumComponent;

    HeaderComponent header{ "Pirouss Sgal Analyser", pluginVersion };

    SgalRotSlider peakDecaySlider{ "Peak Decay" };
    SgalRotSlider rmsWindowSlider{"RMS Window"};
    SgalRotSlider greenThresholdSlider{"Green"};
    SgalRotSlider yellowThresholdSlider{"Yellow"};
    SgalRotSlider orangeThresholdSlider{"Orange"};
    SgalRotSlider redThresholdSlider{"Red"};

    // pointeurs apvts
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rmsWindowAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> greenThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> yellowThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> orangeThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> redThresholdAttachment;

    // Méthode helper pour lier facilement un SgalRotSlider à l'APVTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> createAttachment(
        SgalRotSlider& rotSlider, const juce::String& parameterID);

    // Méthode pour débug qui affiche la taille
    //void paintOverChildren(juce::Graphics& g);
    //bool OKpaintDebug = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PiroussSpectrumAnalyzerv1AudioProcessorEditor)
};


