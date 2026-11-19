#pragma once
#include <JuceHeader.h>

class SgalRotSlider : public juce::Component
{
public:
    SgalRotSlider(const juce::String& labelText)
    {
        auto sliderColor = juce::Colour(0xff8a2be2); // Violet néon
        auto textColor = juce::Colour(0xffff1493); // Rose fluo

        // 1. Configuration du Label (au-dessus)
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font("Eras Demi ITC", 13.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, textColor);
        addAndMakeVisible(label);

        // Application du LookAndFeel contour ondulé / technique
        slider.setLookAndFeel(&techMoogLookAndFeel);

        // 2. Configuration du Slider (Rotatif)
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
        slider.setColour(juce::Slider::textBoxTextColourId, textColor);
        slider.setColour(juce::Slider::textBoxOutlineColourId, textColor);
        addAndMakeVisible(slider);
    }

    ~SgalRotSlider() override
    {
        slider.setLookAndFeel(nullptr);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromTop(20));
        slider.setBounds(bounds);
    }

    juce::Slider& getSlider() { return slider; }

private:
    // --- LOOK AND FEEL CONTOURS ONDULÉS / STYLE SCHÉMA TECHNIQUE ---
    class TechMoogLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
            float sliderPosProportional, float rotaryStartAngle,
            float rotaryEndAngle, juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(4.0f);
            auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
            auto center = bounds.getCentre();

            auto neonPurple = juce::Colour(0xff8a2be2);
            auto darkBg = juce::Colour(0xff0a0a0f);

            // 1. Construction du chemin pour le contour extérieur ondulé (cannelures)
            juce::Path outerKnobPath;
            int numLobes = 9;      // Nombre de crans/ondulations sur le pourtour
            float depth = 3.0f;    // Profondeur des creux de l'ondulation

            const int numSteps = 120;
            for (int i = 0; i <= numSteps; ++i)
            {
                float t = (float)i / (float)numSteps;
                float angle = t * juce::MathConstants<float>::twoPi;

                // Modulation mathématique du rayon pour créer l'effet cranté
                float currentRadius = radius - (depth * 0.5f) + std::sin(angle * numLobes) * (depth * 0.5f);

                float px = center.x + std::sin(angle) * currentRadius;
                float py = center.y - std::cos(angle) * currentRadius;

                if (i == 0)
                    outerKnobPath.startNewSubPath(px, py);
                else
                    outerKnobPath.lineTo(px, py);
            }
            outerKnobPath.closeSubPath();

            // Remplissage de l'intérieur du potard
            g.setColour(darkBg);
            g.fillPath(outerKnobPath);

            // Contour unique ondulé en violet néon
            g.setColour(neonPurple);
            g.strokePath(outerKnobPath, juce::PathStrokeType(1.5f));

            // 2. Chapeau central plat (style insert de bouton)
            auto capRadius = radius * 0.65f;
            g.setColour(darkBg);
            g.fillEllipse(center.x - capRadius, center.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);

            g.setColour(neonPurple.withAlpha(0.8f));
            g.drawEllipse(center.x - capRadius, center.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);

            // 3. Calcul de l'angle pour l'indicateur
            auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            // 4. Ligne indicateur principale en violet néon
            g.setColour(neonPurple);
            juce::Line<float> line(center.x + std::sin(angle) * (capRadius * 0.2f),
                center.y - std::cos(angle) * (capRadius * 0.2f),
                center.x + std::sin(angle) * (capRadius * 0.92f),
                center.y - std::cos(angle) * (capRadius * 0.92f));
            g.drawLine(line, 2.0f);
        }
    };

    juce::Label label;
    juce::Slider slider;
    TechMoogLookAndFeel techMoogLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SgalRotSlider)
};