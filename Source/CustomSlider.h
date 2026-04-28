//
// Created by alisdair chauvin on 4/22/26.
//

#ifndef MALFY_CUSTOMSLIDER_H
#define MALFY_CUSTOMSLIDER_H

#pragma once
#include "juce_gui_basics/juce_gui_basics.h"

class CustomSlider : public juce::Slider
{
public:
    CustomSlider();
    ~CustomSlider() override;

private:
    struct LAF : public juce::LookAndFeel_V4
    {
        void drawLinearSlider (juce::Graphics&,
                               int x, int y, int width, int height,
                               float sliderPos,
                               float minPos, float maxPos,
                               juce::Slider::SliderStyle,
                               juce::Slider&) override;

        int getSliderThumbRadius (juce::Slider&) override;
    };

    LAF laf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomSlider)
};


#endif //MALFY_CUSTOMSLIDER_H