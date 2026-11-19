#pragma once
#include <JuceHeader.h>

class RotSlider : public juce::Component
{
public:
    RotSlider(const juce::String& labelText)
    {
        auto color = juce::Colour(0xff8a2be2);

        // 1. Configuration du Label (au-dessus)
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font("Eras Demi ITC", 13.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, color);
        addAndMakeVisible(label);

        // 2. Configuration du Slider (Rotatif)
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffff1493).darker(0.5f));
        //slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff8a2be2));

        // 3. Configuration TextBox
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
        slider.setColour(juce::Slider::textBoxTextColourId, color);
        slider.setColour(juce::Slider::textBoxOutlineColourId, color);
        addAndMakeVisible(slider);
    }

    ~RotSlider() override = default;

    void resized() override
    {
        auto bounds = getLocalBounds();

        // On réserve 20 pixels en haut pour le label
        label.setBounds(bounds.removeFromTop(20));

        // Le reste de la hauteur et de la largeur est occupé par le slider
        slider.setBounds(bounds);
    }

    // Méthode très pratique pour récupérer le slider et le lier à l'APVTS (AudioProcessorValueTreeState)
    juce::Slider& getSlider() { return slider; }

private:
    juce::Label label;
    juce::Slider slider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotSlider)
};