// Alisdair Chauvin
// 11/17/25

#include "Square.h"

void Square::paint (juce::Graphics& g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::orange);

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("EFFEM", getLocalBounds(), juce::Justification::centred, 1);
}
void Square::resized() {

}