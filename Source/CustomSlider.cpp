//
// Created by alisdair chauvin on 4/22/26.
//

#include "CustomSlider.h"

CustomSlider::CustomSlider()
{
    setSliderStyle  (juce::Slider::LinearVertical);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    setLookAndFeel  (&laf);
}

CustomSlider::~CustomSlider()
{
    setLookAndFeel (nullptr);
}

void CustomSlider::LAF::drawLinearSlider (juce::Graphics& g,
                                          int x, int y, int width, int height,
                                          float sliderPos,
                                          float, float,
                                          juce::Slider::SliderStyle,
                                          juce::Slider&)
{
    constexpr float trackW = 3.0f;
    constexpr float thumbW = 20.0f;
    constexpr float thumbH = 6.0f;
    constexpr float radius = 1.5f;

    const juce::Colour trackBg { 0xFF000000 };
    const juce::Colour fill    { 0xFFFFFFFF };
    const juce::Colour thumb   { 0xFFFFFFFF };
    const juce::Colour edge    { 0xFF000000 };

    const float cx    = x + width * 0.5f;
    const float top   = (float) y;
    const float bot   = (float) (y + height);
    const float halfW = trackW * 0.5f;

    // Track
    g.setColour (trackBg);
    g.fillRoundedRectangle (cx - halfW, top, trackW, bot - top, halfW);

    // Filled portion (sliderPos → bottom)
    g.setColour (fill);
    g.fillRoundedRectangle (cx - halfW, sliderPos, trackW, bot - sliderPos, halfW);

    // Thumb
    juce::Rectangle<float> t { cx - thumbW * 0.5f, sliderPos - thumbH * 0.5f, thumbW, thumbH };

    g.setColour (thumb);
    g.fillRoundedRectangle (t, radius);

    g.setColour (edge);
    g.drawRoundedRectangle (t.reduced (0.5f), radius, 1.0f);
    g.drawLine (cx - thumbW * 0.25f, sliderPos, cx + thumbW * 0.25f, sliderPos, 1.0f);
}

int CustomSlider::LAF::getSliderThumbRadius (juce::Slider&)
{
    return 3;
}