#include "VerticalSpectrumComponent.h"

VerticalSpectrumComponent::VerticalSpectrumComponent()
{
}

void VerticalSpectrumComponent::updateData(const std::vector<BandData>& newData, const std::vector<float>& frequencies)
{
    currentData = newData;
    apiFreqs = frequencies;
    repaint();
}

void VerticalSpectrumComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.9f));

    const float width = (float)getWidth();
    const float height = (float)getHeight();
    const float centerX = width * 0.5f;
    const float scaleWidth = 50.0f;

    // 1. Fond de la colonne centrale
    g.setColour(juce::Colour(0xff151515));
    g.fillRect(centerX - scaleWidth * 0.5f, 0.0f, scaleWidth, height);

    const float maxBarLength = (width - scaleWidth) * 0.5f - 10.0f;

    // --- 2. DESSIN DES REPÈRES EN POINTILLÉS (Blancs) ---
    std::vector<float> dbMarkers = { -9.0f, -18.0f, -32.0f, -40.0f, -50.0f };
    g.setColour(juce::Colours::white.withAlpha(0.35f));

    for (float db : dbMarkers)
    {
        float normPos = juce::jmap(db, -80.0f, 0.0f, 0.0f, 1.0f);
        float pixelOffset = normPos * maxBarLength;

        float xLeft = (centerX - scaleWidth * 0.5f) - pixelOffset;
        float xRight = (centerX + scaleWidth * 0.5f) + pixelOffset;

        const float dashLengths[2] = { 2.0f, 4.0f };
        g.drawDashedLine(juce::Line<float>(xLeft, 0.0f, xLeft, height), dashLengths, 2, 1.0f);
        g.drawDashedLine(juce::Line<float>(xRight, 0.0f, xRight, height), dashLengths, 2, 1.0f);

        g.setFont(8.0f);
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        juce::String dbStr = juce::String((int)db);
        g.drawText(dbStr, (int)(xLeft - 15.0f), 2.0f, 30.0f, 10.0f, juce::Justification::centred);
        g.drawText(dbStr, (int)(xRight - 15.0f), 2.0f, 30.0f, 10.0f, juce::Justification::centred);
        g.setColour(juce::Colours::white.withAlpha(0.35f));
    }

    // Fonction de mapping Y
    auto getY = [height](float freq) -> float
        {
            const float logMin = std::log10(20.0f);
            const float logMax = std::log10(20000.0f);
            const float logF = std::log10(juce::jlimit(20.0f, 20000.0f, freq));
            return juce::jmap(logF, logMin, logMax, height - 12.0f, 12.0f);
        };

    if (currentData.empty() || apiFreqs.empty()) return;

    // --- 3. CONFIGURATION DU GRADIENT DE COULEUR GLOBAL ---
    // Le gradient s'étend de -80 dBFS (intérieur) à 0 dBFS (extérieur)
    auto createBarGradient = [&](float startX, float endX, bool isLeft)
        {
            juce::ColourGradient grad(
                juce::Colours::cyan,
                startX, 0.0f,
                juce::Colours::darkred, // Extrémité max à 0 dBFS
                endX, 0.0f,
                false
            );

            grad.addColour(juce::jmap(threshCyan, -80.0f, 0.0f, 0.0f, 1.0f), juce::Colours::cyan);
            grad.addColour(juce::jmap(threshGreen, -80.0f, 0.0f, 0.0f, 1.0f), juce::Colours::green);
            grad.addColour(juce::jmap(threshYellow, -80.0f, 0.0f, 0.0f, 1.0f), juce::Colours::yellow);
            grad.addColour(juce::jmap(threshOrange, -80.0f, 0.0f, 0.0f, 1.0f), juce::Colours::orange);
            grad.addColour(juce::jmap(threshRed, -80.0f, 0.0f, 0.0f, 1.0f), juce::Colours::red);

            return grad;
        };

    // Définition des coordonnées géométriques globales pour les gradients
    float innerLeftX = centerX - scaleWidth * 0.5f;
    float outerLeftX = innerLeftX - maxBarLength;
    float innerRightX = centerX + scaleWidth * 0.5f;
    float outerRightX = innerRightX + maxBarLength;

    juce::ColourGradient gradLeft = createBarGradient(innerLeftX, outerLeftX, true);
    juce::ColourGradient gradRight = createBarGradient(innerRightX, outerRightX, false);

    for (size_t i = 0; i < apiFreqs.size(); ++i)
    {
        float y = getY(apiFreqs[i]);

        float clampedPeakL = juce::jlimit(-80.0f, 0.0f, currentData[i].peakDbL);
        float clampedRmsL = juce::jlimit(-80.0f, 0.0f, currentData[i].rmsDbL);
        float lenPeakL = juce::jmap(clampedPeakL, -80.0f, 0.0f, 0.0f, 1.0f) * maxBarLength;
        float lenRmsL = juce::jmap(clampedRmsL, -80.0f, 0.0f, 0.0f, 1.0f) * maxBarLength;

        float clampedPeakR = juce::jlimit(-80.0f, 0.0f, currentData[i].peakDbR);
        float clampedRmsR = juce::jlimit(-80.0f, 0.0f, currentData[i].rmsDbR);
        float lenPeakR = juce::jmap(clampedPeakR, -80.0f, 0.0f, 0.0f, 1.0f) * maxBarLength;
        float lenRmsR = juce::jmap(clampedRmsR, -80.0f, 0.0f, 0.0f, 1.0f) * maxBarLength;

        // 1. Dessin du PEAK (Fin et semi-transparent avec le gradient)
        float peakHeight = 2.0f;
        g.setGradientFill(gradLeft);
        g.fillRect(innerLeftX - lenPeakL, y - peakHeight * 0.5f, lenPeakL, peakHeight);

        g.setGradientFill(gradRight);
        g.fillRect(innerRightX, y - peakHeight * 0.5f, lenPeakR, peakHeight);

        // 2. Dessin du RMS (Masse principale pleine avec le gradient)
        float rmsHeight = 5.0f;
        g.setGradientFill(gradLeft);
        g.fillRect(innerLeftX - lenRmsL, y - rmsHeight * 0.5f, lenRmsL, rmsHeight);

        g.setGradientFill(gradRight);
        g.fillRect(innerRightX, y - rmsHeight * 0.5f, lenRmsR, rmsHeight);

        // 3. Texte Fréquence au centre (on n'affiche qu'une bande sur deux ou les principales pour ne pas surcharger)
        // Remplace la ligne du bool showLabel par un affichage direct pour chaque i :
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(7.5f);

        juce::String freqText;
        if (apiFreqs[i] >= 1000.0f)
            freqText = juce::String(apiFreqs[i] / 1000.0f, 1) + "k";
        else
            freqText = juce::String((int)apiFreqs[i]);

        g.drawText(freqText, (int)(centerX - scaleWidth * 0.5f), (int)(y - 7.0f), (int)scaleWidth, 14, juce::Justification::centred);
    }
}

void VerticalSpectrumComponent::resized()
{
}