//
// Created by alisdair chauvin on 4/28/26.
//

#include "CustomCombo.h"

CustomCombo::CustomCombo()
{
    setLookAndFeel (&laf);
    setJustificationType (juce::Justification::centred);

    setColour (juce::ComboBox::backgroundColourId, juce::Colours::black);
    setColour (juce::ComboBox::textColourId, juce::Colours::white);
    setColour (juce::ComboBox::outlineColourId, juce::Colours::white);
    setColour (juce::ComboBox::arrowColourId, juce::Colours::white);
}

CustomCombo::~CustomCombo()
{
    setLookAndFeel (nullptr);
}

//==============================================================================

void CustomCombo::LAF::drawComboBox (juce::Graphics& g,
                                     int width, int height,
                                     bool /*isButtonDown*/,
                                     int /*buttonX*/, int /*buttonY*/,
                                     int /*buttonW*/, int /*buttonH*/,
                                     juce::ComboBox& box)
{
    constexpr float radius = 2.0f;
    constexpr float border = 1.0f;

    const juce::Colour bg { 0xFF000000 };  // black
    const juce::Colour edge { 0xFFFFFFFF };  // white

    juce::Rectangle<float> r (0.0f, 0.0f, (float) width, (float) height);

    g.setColour (bg);
    g.fillRoundedRectangle (r, radius);

    g.setColour (edge);
    g.drawRoundedRectangle (r.reduced (border * 0.5f), radius, border);

    // Arrow
    const float ax = width - 14.0f;
    const float ay = height * 0.5f;

    juce::Path arrow;
    arrow.addTriangle (ax, ay - 2.0f,
                       ax + 6.0f, ay - 2.0f,
                       ax + 3.0f, ay + 3.0f);

    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.fillPath (arrow);
}

juce::Font CustomCombo::LAF::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (14.0f);
}

void CustomCombo::LAF::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (1, 1, box.getWidth() - 16, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
    label.setJustificationType (juce::Justification::centred);
}

//==============================================================================

void CustomCombo::LAF::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::white);
    g.drawRect (0, 0, width, height, 1);
}

void CustomCombo::LAF::drawPopupMenuItem (juce::Graphics& g,
                                          const juce::Rectangle<int>& area,
                                          bool isSeparator, bool /*isActive*/, bool isHighlighted,
                                          bool isTicked, bool /*hasSubMenu*/,
                                          const juce::String& text,
                                          const juce::String& /*shortcutKeyText*/,
                                          const juce::Drawable* /*icon*/,
                                          const juce::Colour* /*textColour*/)
{
    if (isSeparator)
    {
        g.setColour (juce::Colours::white.withAlpha (0.3f));
        g.drawLine ((float) area.getX() + 4.0f, (float) area.getCentreY(),
                    (float) area.getRight() - 4.0f, (float) area.getCentreY());
        return;
    }

    if (isHighlighted)
    {
        g.setColour (juce::Colours::white);
        g.fillRect (area);
        g.setColour (juce::Colours::black);
    }
    else
    {
        g.setColour (juce::Colours::white);
    }

    auto r = area.reduced (6, 0);

    if (isTicked)
    {
        g.fillEllipse ((float) r.getX(), (float) r.getCentreY() - 2.0f, 4.0f, 4.0f);
        r.removeFromLeft (10);
    }

    g.setFont (14.0f);
    g.drawFittedText (text, r, juce::Justification::centredLeft, 1);
}