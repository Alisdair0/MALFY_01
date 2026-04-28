//
// Created by alisdair chauvin on 4/28/26.
//

#include "CustomTitle.h"

void CustomTitle::paint (juce::Graphics& g)
{
    juce::Font font (juce::Font::getDefaultSansSerifFontName(),
                     (float) getHeight() * 0.85f, juce::Font::bold);
    font.setExtraKerningFactor (-0.03f); // try 0.0f or -0.05f

    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText (font, titleText, 0.0f, (float) getHeight() * 0.8f);

    juce::Path p;
    glyphs.createPath (p);

    constexpr int   numEchoes  = 10;
    constexpr float offsetStep = 8.0f;

    // Echoes (filled + stroked, faded)
    for (int i = numEchoes; i >= 1; --i)
    {
        juce::Path echo (p);
        g.setColour (juce::Colours::white);
        g.fillPath (p);
        echo.applyTransform (juce::AffineTransform::translation (i * offsetStep, 0.0f));

        const float alpha = 0.2f / (float) i;
        g.setColour (juce::Colours::white.withAlpha (alpha));
        g.fillPath   (echo);
        g.strokePath (echo, juce::PathStrokeType (1.5f));
    }

    // Main title
    g.setColour (juce::Colours::white);
    g.fillPath (p);
    g.strokePath (p, juce::PathStrokeType (1.5f));
}