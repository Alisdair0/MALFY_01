#include "PluginProcessor.h"
#include "PluginEditor.h"

static void configureSliderTwoDecimals(juce::Slider& s)
{
    s.setNumDecimalPlacesToDisplay(2);
}
// ============================================================ //

WaveformDisplay::WaveformDisplay(AudioPluginAudioProcessor& p)
    : processor(p)
{
    startTimerHz(60); // redraw at 60fps
}
void WaveformDisplay::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::slategrey);
    g.drawRect(0, 0, getWidth(), getHeight());

    auto bounds = getLocalBounds().toFloat();
    auto width  = bounds.getWidth();
    auto height = bounds.getHeight();

    auto& data = processor.scopeData;
    int writePos = processor.scopeWritePos.load();

    // Subtle border
    g.setColour(juce::Colours::slategrey.withAlpha(0.6f));
    g.drawRect(bounds, 1.0f);

    // Subtle grid
    g.setColour(juce::Colours::dimgrey.withAlpha(0.18f));

    for (int i = 1; i < 4; ++i)
    {
        float y = height * (float) i / 4.0f;
        g.drawHorizontalLine((int) y, 0.0f, width);
    }

    for (int i = 1; i < 8; ++i)
    {
        float x = width * (float) i / 8.0f;
        g.drawVerticalLine((int) x, 0.0f, height);
    }

    juce::Path p;
    p.startNewSubPath(0.0f, static_cast<float>(getHeight()) / 2.0f);

    for (int i = 0; i < AudioPluginAudioProcessor::scopeSize; ++i)
    {
        auto index = (writePos + i) % AudioPluginAudioProcessor::scopeSize;
        float x = juce::jmap((float)i, 0.f, (float)AudioPluginAudioProcessor::scopeSize - 1, 0.f, (float)getWidth());
        float y = juce::jmap(juce::jlimit(-1.0f, 1.0f, data[static_cast<size_t>(index)] * 2.5f),
                     -1.f, 1.f,
                     (float)getHeight() - 6.0f,
                     6.0f);

        if (i == 0)
            p.startNewSubPath(x, y);
        else
            p.lineTo(x, y);
    }

    // Glow layers
    g.setColour(juce::Colours::yellow.withAlpha(0.10f));
    g.strokePath(p, juce::PathStrokeType(10.0f));

    g.setColour(juce::Colours::orange.withAlpha(0.25f));
    g.strokePath(p, juce::PathStrokeType(6.0f));

    g.setColour(juce::Colours::orange.withAlpha(0.95f));
    g.strokePath(p, juce::PathStrokeType(2.2f));
}

void WaveformDisplay::timerCallback()
{
    repaint();
}

// ============================================================================== //
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), waveformDisplay(p), processorRef (p)
{
    // Window size
    setSize (800, 600);

    auto& state = p.getState();

    addAndMakeVisible(waveformDisplay);

    // =============== //
    // GLOBAL CONTROLS //
    // =============== //

    // Master Gain (global)
    masterGainSlider.setSliderStyle (juce::Slider::LinearVertical);
    masterGainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (masterGainSlider);
    addAndMakeVisible (masterGainLabel);
    masterGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "masterGain", masterGainSlider);

    configureSliderTwoDecimals(detuneSlider);
    configureSliderTwoDecimals(masterGainSlider);

    // ADSR
    adsrLabel.setText("Envelope", juce::dontSendNotification);
    adsrLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(adsrLabel);

    attackSlider.setSliderStyle(juce::Slider::LinearVertical);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(attackLabel);

    attackAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "attack", attackSlider);

    decaySlider.setSliderStyle(juce::Slider::LinearVertical);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    addAndMakeVisible(decaySlider);
    addAndMakeVisible(decayLabel);

    decayAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "decay", decaySlider);

    sustainSlider.setSliderStyle(juce::Slider::LinearVertical);
    sustainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    addAndMakeVisible(sustainSlider);
    addAndMakeVisible(sustainLabel);

    sustainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "sustain", sustainSlider);

    releaseSlider.setSliderStyle(juce::Slider::LinearVertical);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(releaseLabel);

    releaseAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "release", releaseSlider);

    configureSliderTwoDecimals(attackSlider);
    configureSliderTwoDecimals(decaySlider);
    configureSliderTwoDecimals(sustainSlider);
    configureSliderTwoDecimals(releaseSlider);

    // ============== //
    // FILTER SECTION //
    // ============== //

    filterLabel.setText("Filter", juce::dontSendNotification);
    filterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterLabel);

    filterType.addItem("Lowpass", 1);
    filterType.addItem("Highpass", 2);
    filterType.addItem("Bandpass", 3);
    addAndMakeVisible(filterType);

    filterTypeAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            state, "filterType", filterType);

    cutoffSlider.setSliderStyle(juce::Slider::LinearVertical);
    cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(cutoffSlider);
    addAndMakeVisible(cutoffLabel);

    cutoffAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "filterCutoff", cutoffSlider);

    resonanceSlider.setSliderStyle(juce::Slider::LinearVertical);
    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(resonanceSlider);
    addAndMakeVisible(resonanceLabel);

    resonanceAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "filterResonance", resonanceSlider);

    // ============ //
    // OSCILLATOR 1 //
    // ============ //

    addAndMakeVisible(osc1OnButton);
    osc1OnButton.setButtonText("On");

    osc1OnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
    state, "osc1On", osc1OnButton);

    osc1WaveBox.addItemList({ "Sine","Square","Saw","Triangle","Noise","Add1","Add2" }, 1);
    addAndMakeVisible(osc1WaveBox);

    osc1PitchBox.addItem("+12", 1);
    osc1PitchBox.addItem("+7", 2);
    osc1PitchBox.addItem("0",  3);
    osc1PitchBox.addItem("-7", 4);
    osc1PitchBox.addItem("-12",5);
    addAndMakeVisible(osc1PitchBox);

    addAndMakeVisible(osc1GainLabel);
    addAndMakeVisible(osc1DetuneLabel);
    addAndMakeVisible(osc1FmLabel);
    addAndMakeVisible(osc1PitchLabel);
    addAndMakeVisible(osc1WaveLabel);

    osc1GainLabel.setText("Gain", juce::dontSendNotification);
    osc1DetuneLabel.setText("Detune", juce::dontSendNotification);
    osc1FmLabel.setText("FM", juce::dontSendNotification);

    carrierLabel.setText("Carrier", juce::dontSendNotification);
    carrierLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(carrierLabel);

    detune1Slider.setSliderStyle(juce::Slider::LinearVertical);
    detune1Slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(detune1Slider);

    gain1Slider.setSliderStyle(juce::Slider::LinearVertical);
    gain1Slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(gain1Slider);

    fm1Slider.setSliderStyle(juce::Slider::LinearVertical);
    fm1Slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(fm1Slider);

    osc1WaveAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            state, "osc1Wave", osc1WaveBox);

    osc1PitchAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            state, "osc1Pitch", osc1PitchBox);

    osc1DetuneAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "osc1Detune", detune1Slider);

    osc1GainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "osc1Gain", gain1Slider);

    osc1FmAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "osc1FM", fm1Slider);

    // ============ //
    // OSCILLATOR 2 //
    // ============ //

    osc2OnButton.setButtonText("On");
    addAndMakeVisible(osc2OnButton);

    osc2WaveBox.addItemList({ "Sine","Square","Saw","Triangle","Noise","Add1","Add2" }, 1);
    addAndMakeVisible(osc2WaveBox);

    osc2PitchBox.addItem("+12", 1);
    osc2PitchBox.addItem("+7", 2);
    osc2PitchBox.addItem("0",  3);
    osc2PitchBox.addItem("-7", 4);
    osc2PitchBox.addItem("-12",5);
    addAndMakeVisible(osc2PitchBox);

    addAndMakeVisible(osc2GainLabel);
    addAndMakeVisible(osc2DetuneLabel);
    addAndMakeVisible(osc2FmLabel);
    addAndMakeVisible(osc2PitchLabel);
    addAndMakeVisible(osc2WaveLabel);
    modulatorLabel.setText("Modulator", juce::dontSendNotification);
    modulatorLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(modulatorLabel);

    detune2Slider.setSliderStyle(juce::Slider::LinearVertical);
    detune2Slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(detune2Slider);

    gain2Slider.setSliderStyle(juce::Slider::LinearVertical);
    gain2Slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(gain2Slider);

    fm2Slider.setSliderStyle(juce::Slider::LinearVertical);
    fm2Slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(fm2Slider);

    osc2WaveAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            state, "osc2Wave", osc2WaveBox);

    osc2PitchAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            state, "osc2Pitch", osc2PitchBox);

    osc2DetuneAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "osc2Detune", detune2Slider);

    osc2GainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "osc2Gain", gain2Slider);

    osc2FmAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "osc2FM", fm2Slider);

    // ============ //
    // BLEND SLIDER //
    // ============ //

    blendLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(blendLabel);

    blendSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    blendSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(blendSlider);

    blendAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "oscBlend", blendSlider);

    // =============== LABEL STYLING ================= //
    for (auto* label : {
        &masterGainLabel, &detuneLabel, &pitchShiftLabel,
        &panLabel, &attackLabel, &decayLabel,
        &sustainLabel, &releaseLabel, &filterLabel,
        &cutoffLabel, &resonanceLabel, &blendLabel,
        &osc1GainLabel, &osc1DetuneLabel, &osc1FmLabel,
        &osc1PitchLabel, &osc1WaveLabel,
        &osc2GainLabel, &osc2DetuneLabel, &osc2FmLabel,
        &osc2PitchLabel, &osc2WaveLabel
    })
    {
        label->setColour (juce::Label::textColourId, juce::Colours::white);
        label->setJustificationType (juce::Justification::centred);
    }
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() = default;

// ============================================================================== //

void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
    const int numControls = 4;
    const int margin = 10;
    juce::Rectangle<int> area = (getLocalBounds());
    const int areaWidth = area.getWidth();
    const int areaHeight = area.getHeight() / 2;
    const int controlHeight = areaHeight - margin;
    const int rectangleWidth = (areaWidth - (margin * (numControls + 1))) / numControls;

    juce::Rectangle<int> masterBorder;
    masterBorder.setBounds(margin, controlHeight, rectangleWidth, areaHeight);
    g.setColour(juce::Colours::darkred);
    g.drawRoundedRectangle(masterBorder.toFloat(), 0.0f,2.0f);
    g.setColour(juce::Colour(0xff001933));
    g.fillRect(masterBorder.toFloat());

    juce::Rectangle<int> oscBorder;
    oscBorder.setBounds((margin * 2) + rectangleWidth, controlHeight, rectangleWidth, areaHeight);
    g.setColour(juce::Colours::darkblue);
    g.drawRoundedRectangle(oscBorder.toFloat(), 0.0f,2.0f);
    g.setColour(juce::Colour(0xff28643C));
    g.fillRect(oscBorder.toFloat());

    juce::Rectangle<int> adsrBorder;
    adsrBorder.setBounds((margin * 3) + (rectangleWidth * 2), controlHeight, rectangleWidth, areaHeight);
    g.setColour(juce::Colours::darkorange);
    g.drawRoundedRectangle(adsrBorder.toFloat(), 0.0f,2.0f);
    g.setColour(juce::Colour(0xff4C197F));
    g.fillRect(adsrBorder.toFloat());


    // Cyan band border
    juce::Rectangle<int> filterBorder;
    filterBorder.setBounds((margin * 4) + (rectangleWidth * 3), controlHeight, rectangleWidth, areaHeight);
    g.setColour(juce::Colours::darkcyan);
    g.drawRoundedRectangle(filterBorder.toFloat(), 0.0f,2.0f);
    g.setColour(juce::Colour(0xff7F6B00));
    g.fillRect(filterBorder.toFloat());
}

void AudioPluginAudioProcessorEditor::resized() {
    const int margin = 10;
    const int waveformBottomGap = 15;
    auto area = getLocalBounds();

    // Split Window
    auto topArea = area.removeFromTop(area.getHeight() / 2);
    topArea.removeFromBottom(waveformBottomGap);
    waveformDisplay.setBounds(topArea.reduced(margin));

    // Control Zone
    const int numControls = 4;
    const int areaWidth = area.getWidth();
    const int areaHeight = area.getHeight();
    const int rectangleWidth = (areaWidth - (margin * (numControls + 1))) / numControls;

    // Recreate your 4 sections
    juce::Rectangle<int> masterArea (margin, area.getY(), rectangleWidth, areaHeight);
    juce::Rectangle<int> oscArea ((margin * 2) + rectangleWidth, area.getY(), rectangleWidth, areaHeight);
    juce::Rectangle<int> adsrArea ((margin * 3) + (rectangleWidth * 2), area.getY(), rectangleWidth, areaHeight);
    juce::Rectangle<int> filterArea ((margin * 4) + (rectangleWidth * 3), area.getY(), rectangleWidth, areaHeight);

    // Master (Red)
    masterGainSlider.setBounds(masterArea.reduced(20).removeFromTop(120));
    masterGainLabel.setBounds(masterGainSlider.getX(),
                              masterGainSlider.getBottom(),
                              masterGainSlider.getWidth(), 20);

    // Oscillators (Blue)
    auto osc = oscArea.reduced(10);

    // Split vertically (OSC1 on top, OSC2 below)
    auto osc1Area = osc.removeFromTop(static_cast<int>(osc.getHeight() / 1.5));
    osc.removeFromTop(10);
    auto osc2Area = osc;

    // ---------- OSC 1 ----------
    {
        auto header = osc1Area.removeFromTop(20);

        auto labelArea = header.removeFromLeft(header.getWidth() * 2 / 3);
        carrierLabel.setBounds(labelArea);

        osc1OnButton.setBounds(header.reduced(2));

        auto top = osc1Area.removeFromTop(40);
        int tW = top.getWidth() / 2;

        osc1WaveBox.setBounds(top.removeFromLeft(tW).reduced(5));
        osc1PitchBox.setBounds(top.reduced(5));

        auto boxLabels = osc1Area.removeFromTop(10);
        int bL = boxLabels.getWidth() / 2;

        osc1WaveLabel.setBounds(boxLabels.removeFromLeft(bL));
        osc1PitchLabel.setBounds(boxLabels);

        auto labelSpace = osc1Area.removeFromBottom(24);

        auto sliders = osc1Area.removeFromTop(140);
        int w = sliders.getWidth() / 3;

        detune1Slider.setBounds(sliders.removeFromLeft(w).reduced(5));
        gain1Slider.setBounds(sliders.removeFromLeft(w).reduced(5));
        fm1Slider.setBounds(sliders.reduced(5));

        int wL = labelSpace.getWidth() / 3;

        osc1DetuneLabel.setBounds(labelSpace.removeFromLeft(wL));
        osc1GainLabel.setBounds(labelSpace.removeFromLeft(wL));
        osc1FmLabel.setBounds(labelSpace);
    }

    // ---------- OSC 2 ----------
    {
        auto header = osc2Area.removeFromTop(20);
        modulatorLabel.setBounds(header);

        auto top = osc2Area.removeFromTop(40);
        int tW = top.getWidth() / 2;

        osc2WaveBox.setBounds(top.removeFromLeft(tW).reduced(5));
        osc2PitchBox.setBounds(top.reduced(5));

        auto boxLabels = osc2Area.removeFromTop(10);
        int bL = boxLabels.getWidth() / 2;

        osc2WaveLabel.setBounds(boxLabels.removeFromLeft(bL));
        osc2PitchLabel.setBounds(boxLabels);

        // auto sliders = osc2Area.removeFromTop(100);
        // int w = sliders.getWidth() / 3;
        //
        // detune2Slider.setBounds(sliders.removeFromLeft(w).reduced(5));
        //
        // auto labels = osc2Area.removeFromTop(20);
        // int wL = labels.getWidth() / 3;
        //
        // osc2DetuneLabel.setBounds(labels.removeFromLeft(wL));
    }

    // ADSR (yellow)
    auto adsr = adsrArea.reduced(10);

    auto sliders = adsr.removeFromTop(120);
    int width = sliders.getWidth() / 4;

    attackSlider.setBounds(sliders.removeFromLeft(width).reduced(5));
    decaySlider.setBounds(sliders.removeFromLeft(width).reduced(5));
    sustainSlider.setBounds(sliders.removeFromLeft(width).reduced(5));
    releaseSlider.setBounds(sliders.reduced(5));

    // Labels aligned under each slider
    auto labels = adsr.removeFromTop(20);
    int wL = labels.getWidth() / 4;

    attackLabel.setBounds(labels.removeFromLeft(wL));
    decayLabel.setBounds(labels.removeFromLeft(wL));
    sustainLabel.setBounds(labels.removeFromLeft(wL));
    releaseLabel.setBounds(labels);

    // Filter (Cyan)
    auto filter = filterArea.reduced(10);

    // Top controls (fixed small height)
    filterLabel.setBounds(filter.removeFromTop(20));
    filterType.setBounds(filter.removeFromTop(25));

    // Give sliders a guaranteed size
    auto sliderArea = filter.removeFromTop(140);
    int filterWidth = sliderArea.getWidth() / 2;

    cutoffSlider.setBounds(sliderArea.removeFromLeft(filterWidth).reduced(5));
    resonanceSlider.setBounds(sliderArea.reduced(5));

    // Labels BELOW sliders (don’t steal slider space)
    auto labelArea = filter.removeFromTop(20);
    cutoffLabel.setBounds(labelArea.removeFromLeft(labelArea.getWidth() / 2));
    resonanceLabel.setBounds(labelArea);
}
