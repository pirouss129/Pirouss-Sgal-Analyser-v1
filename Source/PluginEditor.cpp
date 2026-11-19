/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// --- CONSTRUCTEUR ---
//==============================================================================
PiroussSpectrumAnalyzerv1AudioProcessorEditor::PiroussSpectrumAnalyzerv1AudioProcessorEditor (PiroussSpectrumAnalyzerv1AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(header);
    
    addAndMakeVisible(peakDecaySlider);
    addAndMakeVisible(rmsWindowSlider);
    addAndMakeVisible(greenThresholdSlider);
    addAndMakeVisible(yellowThresholdSlider);
    addAndMakeVisible(orangeThresholdSlider);
    addAndMakeVisible(redThresholdSlider);

    addAndMakeVisible(spectrumComponent);

    // liaison apvts
    peakDecayAttachment = createAttachment(peakDecaySlider, "PEAK_DECAY");
    rmsWindowAttachment = createAttachment(rmsWindowSlider, "RMS_WINDOW");
    greenThresholdAttachment = createAttachment(greenThresholdSlider, "THRESH_GREEN");
    yellowThresholdAttachment = createAttachment(yellowThresholdSlider, "THRESH_YELLOW");
    orangeThresholdAttachment = createAttachment(orangeThresholdSlider, "THRESH_ORANGE");
    redThresholdAttachment = createAttachment(redThresholdSlider, "THRESH_RED");
    // Enregistrement des listeners sur l'APVTS
    audioProcessor.apvts.addParameterListener("THRESH_CYAN", this);
    audioProcessor.apvts.addParameterListener("THRESH_GREEN", this);
    audioProcessor.apvts.addParameterListener("THRESH_YELLOW", this);
    audioProcessor.apvts.addParameterListener("THRESH_ORANGE", this);
    audioProcessor.apvts.addParameterListener("THRESH_RED", this);

    // INTERCEPTION CLICS
    spectrumComponent.setInterceptsMouseClicks(false, false); // le composant laisse passer les clicks, les enfants contenus dans le composant laisse passer les clicks


    // Initialisation immédiate des seuils au lancement
    updateThresholdsFromAPVTS();

    // Restauration de la taille et de la position du plugin à partir des préférences
    const int savedW = audioProcessor.apvts.state.getProperty("editorWidth", defaultWidth);
    const int savedH = audioProcessor.apvts.state.getProperty("editorHeight", defaultHeight);
    setSize(savedW, savedH);
    setResizeLimits(minWidth, minHeight, maxWidth, maxHeight);
    setResizable(true, true);

    //setSize(400, 500); // Taille confortable pour l'analyseur largeur * hauteur
    startTimerHz(30);   // Rafraîchissement IHM à 30 FPS
}

PiroussSpectrumAnalyzerv1AudioProcessorEditor::~PiroussSpectrumAnalyzerv1AudioProcessorEditor()
{
    stopTimer();
    audioProcessor.apvts.removeParameterListener("THRESH_CYAN", this);
    audioProcessor.apvts.removeParameterListener("THRESH_GREEN", this);
    audioProcessor.apvts.removeParameterListener("THRESH_YELLOW", this);
    audioProcessor.apvts.removeParameterListener("THRESH_ORANGE", this);
    audioProcessor.apvts.removeParameterListener("THRESH_RED", this);
}

//==============================================================================
// --- TIMER CALLBACK ---
//==============================================================================
void PiroussSpectrumAnalyzerv1AudioProcessorEditor::timerCallback()
{
    // 1. Récupération des données depuis le DSP du processeur
    auto rawData = audioProcessor.spectrumAnalyser.getBandData();
    auto freqs = audioProcessor.spectrumAnalyser.getApiFrequencies();

    // 2. Conversion vers la structure attendue par l'UI
    std::vector<VerticalSpectrumComponent::BandData> uiData;
    uiData.resize(rawData.size());
    for (size_t i = 0; i < rawData.size(); ++i) {
        uiData[i].peakDbL = rawData[i].peakDbL;
        uiData[i].rmsDbL = rawData[i].rmsDbL;
        uiData[i].peakDbR = rawData[i].peakDbR;
        uiData[i].rmsDbR = rawData[i].rmsDbR;
    }

    // 3. Transmission et redessin
    spectrumComponent.updateData(uiData, freqs);
    spectrumComponent.repaint();
}

//==============================================================================
// --- PAINT  ---
//==============================================================================
void PiroussSpectrumAnalyzerv1AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void PiroussSpectrumAnalyzerv1AudioProcessorEditor::resized()
{
    
    auto area = getLocalBounds();

    // Le header prend les 55 premiers pixels en haut
    header.setBounds(area.removeFromTop(headerHeight));

    // On laisse un espace de 8 pixels tout en bas de la fenêtre
    area.removeFromBottom(8.0f);

    auto commandArea = area.removeFromBottom(commandHeight);
    int commandAreaWidth = commandArea.getWidth();
    
    auto peakDecayArea = commandArea.removeFromLeft(commandAreaWidth/6);
    auto rmsWindowArea = commandArea.removeFromLeft(commandAreaWidth/6);
    auto greenThresholdArea = commandArea.removeFromLeft(commandAreaWidth/6);
    auto yellowThresholdArea = commandArea.removeFromLeft(commandAreaWidth/6);
    auto orangeThresholdArea = commandArea.removeFromLeft(commandAreaWidth/6);
    auto redThresholdArea = commandArea;

    peakDecaySlider.setBounds(peakDecayArea);
    rmsWindowSlider.setBounds(rmsWindowArea);
    greenThresholdSlider.setBounds(greenThresholdArea);
    yellowThresholdSlider.setBounds(yellowThresholdArea);
    orangeThresholdSlider.setBounds(orangeThresholdArea);
    redThresholdSlider.setBounds(redThresholdArea);


    // L'analyseur spectral prend tout le reste de l'espace en dessous
    spectrumComponent.setBounds(area);
}

//==============================================================================
// --- CREATE ATTACHEMENT  --- METHODE PERSO ---
//==============================================================================

std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> PiroussSpectrumAnalyzerv1AudioProcessorEditor::createAttachment(
    SgalRotSlider& sgalSlider, const juce::String& parameterID)
{
    return std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, parameterID, sgalSlider.getSlider());
}

void PiroussSpectrumAnalyzerv1AudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // On met à jour les seuils dès qu'un paramètre concerné change
    if (parameterID.startsWith("THRESH_"))
    {
        updateThresholdsFromAPVTS();
        spectrumComponent.repaint(); // On redessine l'IHM immédiatement
    }
}

void PiroussSpectrumAnalyzerv1AudioProcessorEditor::updateThresholdsFromAPVTS()
{
    float c = *audioProcessor.apvts.getRawParameterValue("THRESH_CYAN");
    float g = *audioProcessor.apvts.getRawParameterValue("THRESH_GREEN");
    float y = *audioProcessor.apvts.getRawParameterValue("THRESH_YELLOW");
    float o = *audioProcessor.apvts.getRawParameterValue("THRESH_ORANGE");
    float r = *audioProcessor.apvts.getRawParameterValue("THRESH_RED");

    spectrumComponent.setThresholds(c, g, y, o, r);
}

/*
#if JUCE_DEBUG
//==============================================================================
// --- DEBUG PAINT OVER CHILDREN AFFICHAGE taille du plug ---
//==============================================================================
void PiroussSpectrumAnalyzerv1AudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    if (OKpaintDebug)
    {
        // 1. Récupération des dimensions actuelles
        const int w = getWidth();
        const int h = getHeight();
        const juce::String debugText = juce::String(w) + " x " + juce::String(h) + " px";

        // 2. Définition du petit badge de debug (ex: 110x24 px) en haut à droite
        auto debugBounds = juce::Rectangle<int>(w - 115, 8, 110, 24);

        // 3. Dessin du fond et de la bordure
        g.setColour(juce::Colours::black.withAlpha(0.85f));
        g.fillRoundedRectangle(debugBounds.toFloat(), 4.0f);

        g.setColour(juce::Colours::yellow);
        g.drawRoundedRectangle(debugBounds.toFloat(), 4.0f, 1.0f);

        // 4. Dessin du texte au centre du rectangle
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(debugText, debugBounds, juce::Justification::centred, false);
    }

    
}
#endif
*/

//==============================================================================
// --- MENU CLIC DROIT ---
//==============================================================================

void PiroussSpectrumAnalyzerv1AudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isPopupMenu()) // si clic droit
    {
        juce::PopupMenu menu; // Préparation du Menu

        menu.addItem(1, "Made with JUCE", true);

#if JucePlugin_Build_Standalone
        // On récupère la fenêtre pour cocher l'item si elle est déjà on top
        bool isCurrentlyOnTop = false;
        if (auto* window = findParentComponentOfClass<juce::ResizableWindow>())
            isCurrentlyOnTop = window->isAlwaysOnTop();

        menu.addItem(2, "Always on top", true, isCurrentlyOnTop);
#endif

        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) // Menu
            {
                if (result == 1)
                {
                    juce::URL("https://juce.com/").launchInDefaultBrowser();
                }

#if JucePlugin_Build_Standalone
                if (result == 2)
                {
                    if (auto* window = findParentComponentOfClass<juce::ResizableWindow>())
                    {
                        bool currentState = window->isAlwaysOnTop();
                        window->setAlwaysOnTop(!currentState);
                    }
                }
#endif // JucePlugin_Build_Standalone

            });
    }
}

//==============================================================================
// --- DOUBLE CLIC ---
//==============================================================================
void PiroussSpectrumAnalyzerv1AudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)
{
    auto clickPos = event.getPosition();

    if (spectrumComponent.getBounds().contains(clickPos))
    {
        setSize(defaultWidth, defaultHeight);
    }
    
}