/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/SpectrumAnalyser.h"

//==============================================================================
/**
*/
class PiroussSpectrumAnalyzerv1AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PiroussSpectrumAnalyzerv1AudioProcessor();
    ~PiroussSpectrumAnalyzerv1AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    // analyseur DSP accessible publiquement (ou via getter) pour l'Editor
    SpectrumAnalyser spectrumAnalyser;

private:

    // --- Cache de pointeurs atomiques pour les paramètres ---
    std::atomic<float>* peakDecayParam = nullptr;
    std::atomic<float>* rmsWindowParam = nullptr;

    std::atomic<float>* threshCyanParam = nullptr;
    std::atomic<float>* threshGreenParam = nullptr;
    std::atomic<float>* threshYellowParam = nullptr;
    std::atomic<float>* threshOrangeParam = nullptr;
    std::atomic<float>* threshRedParam = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PiroussSpectrumAnalyzerv1AudioProcessor)
};
