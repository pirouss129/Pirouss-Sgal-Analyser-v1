#pragma once
#include <JuceHeader.h>

class HeaderComponent : public juce::Component
{
public:
    HeaderComponent(const juce::String& name, const juce::String& version)
        : pluginName(name), pluginVersion(version)
    {
        // Chargement de l'image logo depuis les ressources binaires
        logoImage = juce::ImageCache::getFromMemory(BinaryData::juce_logo_png, BinaryData::juce_logo_pngSize);
    }

    ~HeaderComponent() override = default;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // 1. Fond général du header (sombre)
        g.fillAll(juce::Colour(0xff0a0a0f));

        // Zone du badge
        auto badgeBounds = bounds.reduced(10.0f, 8.0f);
        float cornerRadius = 8.0f;

        // 2. Dégradé type "GTA VI" : Bleu -> Violet -> Rose -> Orange en diagonale (haut-gauche -> bas-droite)
        juce::ColourGradient bgGradient(
            juce::Colour(0xff00a2ff), // Bleu (Haut-Gauche)
            badgeBounds.getX(), badgeBounds.getY(),
            juce::Colour(0xffff5500), // Orange (Bas-Droite)
            badgeBounds.getRight(), badgeBounds.getBottom(),
            false
        );

        // Ajout des paliers intermédiaires pour le violet et le rose
        bgGradient.addColour(0.33f, juce::Colour(0xff8a2be2)); // Violet électrique
        bgGradient.addColour(0.66f, juce::Colour(0xffff1493)); // Rose fluo

        g.setGradientFill(bgGradient);
        g.fillRoundedRectangle(badgeBounds, cornerRadius);

        // 3. Liseré intérieur noir pour creuser légèrement l'effet
        auto innerBorderBounds = badgeBounds.reduced(1.5f);
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawRoundedRectangle(innerBorderBounds, cornerRadius - 1.0f, 1.5f);

        // 4. Liseré extérieur aux accents néon
        g.setColour(juce::Colour(0xffff1493).withAlpha(0.8f));
        g.drawRoundedRectangle(badgeBounds, cornerRadius, 2.0f);

        // --- AFFICHAGE DU LOGO JUCE À GAUCHE ---
        if (logoImage.isValid())
        {
            float logoHeight = 20.0f;
            float logoWidth = logoHeight * ((float)logoImage.getWidth() / (float)logoImage.getHeight());

            juce::Rectangle<float> logoBounds(badgeBounds.getX() + 15.0f,
                badgeBounds.getCentreY() - (logoHeight * 0.5f),
                logoWidth,
                logoHeight);

            g.drawImage(logoImage, logoBounds);
        }

        // 5. Texte avec la police Eras en blanc et ombre portée
        g.setFont(juce::Font("Eras Demi ITC", 18.0f, juce::Font::bold));
        juce::String displayText = pluginName + " || v " + pluginVersion;

        // Ombre portée noire décalée
        g.setColour(juce::Colours::black.withAlpha(1.8f));
        g.drawText(displayText, badgeBounds.translated(1.0f, 1.5f), juce::Justification::centred, true);

        // Texte principal en gris/blanc
        g.setColour(juce::Colours::whitesmoke);
        g.drawText(displayText, badgeBounds, juce::Justification::centred, true);
    }

    void resized() override
    {
    }

private:
    juce::String pluginName;
    juce::String pluginVersion;
    juce::Image logoImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderComponent)
};