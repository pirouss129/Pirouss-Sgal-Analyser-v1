/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// --- CONSTRUCTEUR ---
//==============================================================================
PiroussSpectrumAnalyzerv1AudioProcessor::PiroussSpectrumAnalyzerv1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#else
    : AudioProcessor(BusesProperties()),
#endif
    // --- APVTS & PARAMÈTRES ---
    apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    peakDecayParam = apvts.getRawParameterValue("PEAK_DECAY");
    rmsWindowParam = apvts.getRawParameterValue("RMS_WINDOW");

    threshCyanParam = apvts.getRawParameterValue("THRESH_CYAN");
    threshGreenParam = apvts.getRawParameterValue("THRESH_GREEN");
    threshYellowParam = apvts.getRawParameterValue("THRESH_YELLOW");
    threshOrangeParam = apvts.getRawParameterValue("THRESH_ORANGE");
    threshRedParam = apvts.getRawParameterValue("THRESH_RED");
}



PiroussSpectrumAnalyzerv1AudioProcessor::~PiroussSpectrumAnalyzerv1AudioProcessor()
{
}

//==============================================================================
const juce::String PiroussSpectrumAnalyzerv1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PiroussSpectrumAnalyzerv1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PiroussSpectrumAnalyzerv1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PiroussSpectrumAnalyzerv1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PiroussSpectrumAnalyzerv1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PiroussSpectrumAnalyzerv1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PiroussSpectrumAnalyzerv1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void PiroussSpectrumAnalyzerv1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PiroussSpectrumAnalyzerv1AudioProcessor::getProgramName (int index)
{
    return {};
}

void PiroussSpectrumAnalyzerv1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
// --- PREPARE TO PLAY ---
//==============================================================================
void PiroussSpectrumAnalyzerv1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spectrumAnalyser.prepare(sampleRate);
}

void PiroussSpectrumAnalyzerv1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PiroussSpectrumAnalyzerv1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
// --- PROCESS BLOCK ---
//==============================================================================
void PiroussSpectrumAnalyzerv1AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Nettoyage des flux inutilisés
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 1. Récupération instantanée via les pointeurs atomiques mis en cache
    int peakIndex = static_cast<int>(*peakDecayParam);
    int rmsIndex = static_cast<int>(*rmsWindowParam);

    // 2. Tables de correspondance vers les valeurs réelles en secondes
    const float peakValues[] = { 0.2f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };
    const float rmsValues[] = { 0.1f, 0.2f, 0.3f, 0.5f, 1.0f };

    // 3. Transmission au DSP de l'analyseur
    spectrumAnalyser.setReleaseTimes(peakValues[peakIndex], rmsValues[rmsIndex]);

    // 4. Envoi du flux audio dans notre analyseur DSP
    spectrumAnalyser.pushBlock(buffer);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        // ..do something to the data... (si besoin de modifier l'audio)
    }
}

//==============================================================================
bool PiroussSpectrumAnalyzerv1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PiroussSpectrumAnalyzerv1AudioProcessor::createEditor()
{
    return new PiroussSpectrumAnalyzerv1AudioProcessorEditor (*this);
}

//==============================================================================
// --- GET & SET STATE ---
//==============================================================================
void PiroussSpectrumAnalyzerv1AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Optionnel : sauvegarde de la taille de la fenêtre si ton UI est redimensionnable
    if (auto* editor = dynamic_cast<PiroussSpectrumAnalyzerv1AudioProcessorEditor*>(getActiveEditor()))
    {
        apvts.state.setProperty("editorWidth", editor->getWidth(), nullptr);
        apvts.state.setProperty("editorHeight", editor->getHeight(), nullptr);
    }

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PiroussSpectrumAnalyzerv1AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// --- APVTS - PARAMETERS LAYOUT ---
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PiroussSpectrumAnalyzerv1AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Peak Decay : choix crantés en secondes
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "PEAK_DECAY",
        "Peak Decay",
        juce::StringArray{ "0.2s", "0.5s", "0.75s", "1.0s", "1.5s", "2.0s" },
        2 // Index par défaut (ici "0.75s")
    ));

    // RMS Window / Release : choix crantés en secondes
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "RMS_WINDOW",
        "RMS Window",
        juce::StringArray{ "0.1s", "0.2s", "0.3s", "0.5s", "1.0s" },
        2 // Index par défaut (ici "0.3s")
    ));
    // --- Seuils de couleur dynamiques (en dB) ---
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "THRESH_CYAN", "Cyan Threshold", juce::NormalisableRange<float>(-80.0f, 0.0f, 1.0f), -70.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "THRESH_GREEN", "Green Threshold", juce::NormalisableRange<float>(-70.0f, 0.0f, 1.0f), -65.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "THRESH_YELLOW", "Yellow Threshold", juce::NormalisableRange<float>(-70.0f, 0.0f, 1.0f), -50.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "THRESH_ORANGE", "Orange Threshold", juce::NormalisableRange<float>(-70.0f, 0.0f, 1.0), -40.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "THRESH_RED", "Red Threshold", juce::NormalisableRange<float>(-70.0f, 0.0f, 1.0f), -30.0f
    ));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PiroussSpectrumAnalyzerv1AudioProcessor();
}
