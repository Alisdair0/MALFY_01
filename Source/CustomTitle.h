//
// Created by alisdair chauvin on 4/28/26.
//

#ifndef MALFY_CUSTOMTITLE_H
#define MALFY_CUSTOMTITLE_H

#pragma once
#include "juce_gui_basics/juce_gui_basics.h"

class CustomTitle : public juce::Component
{
public:
    CustomTitle() = default;
    void setText (const juce::String& t) { titleText = t; repaint(); }
    void paint (juce::Graphics&) override;

private:
    juce::String titleText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomTitle)
};


#endif //MALFY_CUSTOMTITLE_H