//
// Created by alisdair chauvin on 4/28/26.
//

#ifndef MALFY_CUSTOMCOMBO_H
#define MALFY_CUSTOMCOMBO_H

#pragma once
#include "juce_gui_basics/juce_gui_basics.h"

class CustomCombo : public juce::ComboBox
{
public:
    CustomCombo();
    ~CustomCombo() override;

private:
    struct LAF : public juce::LookAndFeel_V4
    {
        void drawComboBox (juce::Graphics&,
                           int width, int height,
                           bool isButtonDown,
                           int buttonX, int buttonY,
                           int buttonW, int buttonH,
                           juce::ComboBox&) override;

        juce::Font getComboBoxFont (juce::ComboBox&) override;

        void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

        void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;

        void drawPopupMenuItem (juce::Graphics&,
                                const juce::Rectangle<int>& area,
                                bool isSeparator, bool isActive, bool isHighlighted,
                                bool isTicked, bool hasSubMenu,
                                const juce::String& text,
                                const juce::String& shortcutKeyText,
                                const juce::Drawable* icon,
                                const juce::Colour* textColour) override;
    };

    LAF laf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomCombo)
};


#endif //MALFY_CUSTOMCOMBO_H