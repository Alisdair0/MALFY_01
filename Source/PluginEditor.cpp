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

    auto& data = processor.scopeData;
    int writePos = processor.scopeWritePos.load();

    juce::Path p;
    p.startNewSubPath(0.0f, getHeight() / 2.0f);

    for (int i = 0; i < AudioPluginAudioProcessor::scopeSize; ++i)
    {
        int index = (writePos + i) % AudioPluginAudioProcessor::scopeSize;
        float x = juce::jmap((float)i, 0.f, (float)AudioPluginAudioProcessor::scopeSize - 1, 0.f, (float)getWidth());
        float y = juce::jmap(data[index], -1.f, 1.f, (float)getHeight(), 0.f);

        if (i == 0)
            p.startNewSubPath(x, y);
        else
            p.lineTo(x, y);
    }

    g.setColour(juce::Colours::orange);
    g.strokePath(p, juce::PathStrokeType(2.0f));
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

    // =========== //
    // PLAY BUTTON //
    // =========== //

    addAndMakeVisible (playButton);
    playAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ButtonAttachment>(
            state, "play", playButton);

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
    attackSlider.setSliderStyle(juce::Slider::LinearVertical);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 16);
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(attackLabel);

    attackAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "attack", attackSlider);

    decaySlider.setSliderStyle(juce::Slider::LinearVertical);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 16);
    addAndMakeVisible(decaySlider);
    addAndMakeVisible(decayLabel);

    decayAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "decay", decaySlider);

    sustainSlider.setSliderStyle(juce::Slider::LinearVertical);
    sustainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 16);
    addAndMakeVisible(sustainSlider);
    addAndMakeVisible(sustainLabel);

    sustainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "sustain", sustainSlider);

    releaseSlider.setSliderStyle(juce::Slider::LinearVertical);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 16);
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
    g.fillAll(juce::Colours::black);
    const int numControls = 4;
    const int margin = 10;
    juce::Rectangle<int> area = (getLocalBounds());
    const int areaWidth = area.getWidth();
    const int areaHeight = area.getHeight() / 2;
    const int controlHeight = areaHeight - margin;
    const int rectangleWidth = (areaWidth - (margin * (numControls + 1))) / numControls;

    juce::Rectangle<int> masterBorder;
    masterBorder.setBounds(margin, controlHeight, rectangleWidth, areaHeight);
    g.setColour(juce::Colours::red);
    g.drawRoundedRectangle(masterBorder.toFloat(), 0.0f,2.0f);

    juce::Rectangle<int> oscBorder;
    oscBorder.setBounds((margin * 2) + rectangleWidth, controlHeight, rectangleWidth, areaHeight);
    g.setColour(juce::Colours::blue);
    g.drawRoundedRectangle(oscBorder.toFloat(), 0.0f,2.0f);

    juce::Rectangle<int> adsrBorder;
    adsrBorder.setBounds((margin * 3) + (rectangleWidth * 2), controlHeight, rectangleWidth, areaHeight);
    g.setColour(juce::Colours::yellow);
    g.drawRoundedRectangle(adsrBorder.toFloat(), 0.0f,2.0f);

    // Cyan band border
    juce::Rectangle<int> filterBorder;
    filterBorder.setBounds((margin * 4) + (rectangleWidth * 3), controlHeight, rectangleWidth, areaHeight);
    g.setColour(juce::Colours::cyan);
    g.drawRoundedRectangle(filterBorder.toFloat(), 0.0f,2.0f);
}

void AudioPluginAudioProcessorEditor::resized() {
    const int margin = 10;
    auto area = getLocalBounds();

    auto layoutColumn = [] (juce::Rectangle<int> area,
                        std::vector<juce::Component*> comps)
    {
        int h = area.getHeight() / (int) comps.size();

        for (auto* c : comps)
            c->setBounds(area.removeFromTop(h).reduced(2));
    };

    // Split Window
    auto topArea = area.removeFromTop(area.getHeight() / 2);
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
    auto osc1Area = osc.removeFromTop(osc.getHeight() / 2);
    auto osc2Area = osc;

    // ---------- OSC 1 ----------
    {
        auto top = osc1Area.removeFromTop(50);
        layoutColumn(top, { &osc1WaveBox, &osc1PitchBox });

        auto sliders = osc1Area.removeFromTop(100);
        int w = sliders.getWidth() / 3;

        detune1Slider.setBounds(sliders.removeFromLeft(w).reduced(5));
        gain1Slider.setBounds(sliders.removeFromLeft(w).reduced(5));
        fm1Slider.setBounds(sliders.reduced(5));

        auto labels = osc1Area.removeFromTop(20);
        int wL = labels.getWidth() / 3;

        osc1DetuneLabel.setBounds(labels.removeFromLeft(wL));
        osc1GainLabel.setBounds(labels.removeFromLeft(wL));
        osc1FmLabel.setBounds(labels);
    }

    // ---------- OSC 2 ----------
    {
        auto top = osc2Area.removeFromTop(50);
        layoutColumn(top, { &osc2WaveBox, &osc2PitchBox });

        auto sliders = osc2Area.removeFromTop(100);
        int w = sliders.getWidth() / 3;

        detune2Slider.setBounds(sliders.removeFromLeft(w).reduced(5));
        gain2Slider.setBounds(sliders.removeFromLeft(w).reduced(5));
        fm2Slider.setBounds(sliders.reduced(5));

        auto labels = osc2Area.removeFromTop(20);
        int wL = labels.getWidth() / 3;

        osc2DetuneLabel.setBounds(labels.removeFromLeft(wL));
        osc2GainLabel.setBounds(labels.removeFromLeft(wL));
        osc2FmLabel.setBounds(labels);
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

    // Labels
    auto filterLabels = filter.removeFromTop(20);
    cutoffLabel.setBounds(filterLabels.removeFromLeft(filterLabels.getWidth() / 2));
    resonanceLabel.setBounds(filterLabels);
}