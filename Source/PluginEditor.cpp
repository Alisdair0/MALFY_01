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
void WaveformDisplay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::slategrey);
    g.drawRect(0, 0, getWidth(), getHeight());

    auto bounds = getLocalBounds().toFloat();
    auto width  = bounds.getWidth();
    auto height = bounds.getHeight();

    auto& data = processor.scopeData;
    int writePos = processor.scopeWritePos.load();

    g.setColour(juce::Colours::slategrey.withAlpha(0.6f));
    g.drawRect(bounds, 1.0f);

    g.setColour(juce::Colours::dimgrey.withAlpha(0.18f));

    for (int i = 1; i < 4; ++i)
    {
        float y = height * static_cast<float>(i) / 4.0f;
        g.drawHorizontalLine(static_cast<int>(y), 0.0f, width);
    }

    for (int i = 1; i < 8; ++i)
    {
        float x = width * static_cast<float>(i) / 8.0f;
        g.drawVerticalLine(static_cast<int>(x), 0.0f, height);
    }

    juce::Path p;

    if (scopeMode == ScopeMode::scrolling)
    {
        for (int i = 0; i < AudioPluginAudioProcessor::scopeSize; ++i)
        {
            auto index = (writePos + i) % AudioPluginAudioProcessor::scopeSize;

            float x = juce::jmap(static_cast<float>(i),
                                 0.f, static_cast<float>(AudioPluginAudioProcessor::scopeSize - 1),
                                 0.f, static_cast<float>(getWidth()));

            float y = juce::jmap(juce::jlimit(-1.0f, 1.0f,
                                              data[static_cast<size_t>(index)] * 2.5f),
                                 -1.f, 1.f,
                                 static_cast<float>(getHeight()) - 6.0f,
                                 6.0f);

            if (i == 0)
                p.startNewSubPath(x, y);
            else
                p.lineTo(x, y);
        }
    }
    else if (scopeMode == ScopeMode::triggered)
    {
        constexpr int scopeSize = AudioPluginAudioProcessor::scopeSize;

        // How many samples we want to draw across the screen
        const int samplesToDraw = 256;

        // Build a linear copy: orderedData[0] = oldest, orderedData[scopeSize - 1] = newest
        std::array<float, scopeSize> orderedData{};

        for (int i = 0; i < scopeSize; ++i)
            orderedData[(size_t) i] = data[(size_t) ((writePos + i) % scopeSize)];

        // Search backwards for a stable trigger before that.
        int endIndex = scopeSize - 1;
        int nominalStart = juce::jmax(0, endIndex - samplesToDraw + 1);

        // Search backwards from nominalStart to find a rising zero crossing
        int triggerIndex = nominalStart;

        for (int i = nominalStart; i > 1; --i)
        {
            float prevSample = orderedData[(size_t) (i - 1)];
            float currSample = orderedData[(size_t) i];

            if (prevSample < -0.01f && currSample >= 0.01f)
            {
                triggerIndex = i;
                break;
            }
        }

        // Draw a full window
        if (triggerIndex + samplesToDraw > scopeSize)
            triggerIndex = scopeSize - samplesToDraw;

        if (samplesToDraw < 2)
            return;

        for (int i = 0; i < samplesToDraw; ++i)
        {
            float x = juce::jmap((float) i,
                                 0.0f, (float) (samplesToDraw - 1),
                                 0.0f, (float) getWidth());

            float y = juce::jmap(juce::jlimit(-1.0f, 1.0f,
                                              orderedData[(size_t) (triggerIndex + i)] * 2.5f),
                                 -1.0f, 1.0f,
                                 (float) getHeight() - 6.0f,
                                 6.0f);

            if (i == 0)
                p.startNewSubPath(x, y);
            else
                p.lineTo(x, y);
        }
    }

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

    // Title
    titleLabel.setText("MALFY", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    //titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));

    addAndMakeVisible(titleLabel);

    // Oscilloscope
    addAndMakeVisible(waveformDisplay);

    scopeModeLabel.setText("Scope", juce::dontSendNotification);
    scopeModeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(scopeModeLabel);

    scopeModeBox.addItem("Scroll", 1);
    scopeModeBox.addItem("Triggered", 2);
    scopeModeBox.setSelectedId(1);

    scopeModeBox.onChange = [this]
    {
        auto id = scopeModeBox.getSelectedId();

        if (id == 1)
            waveformDisplay.setScopeMode(WaveformDisplay::ScopeMode::scrolling);
        else if (id == 2)
            waveformDisplay.setScopeMode(WaveformDisplay::ScopeMode::triggered);
    };

    addAndMakeVisible(scopeModeBox);

    // =============== //
    // GLOBAL CONTROLS //
    // =============== //

    // Master Gain (global)
    masterGainSlider.setSliderStyle (juce::Slider::LinearVertical);
    masterGainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (masterGainSlider);
    addAndMakeVisible (masterGainLabel);
    masterGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, "masterGain", masterGainSlider);

    configureSliderTwoDecimals(detuneSlider);
    configureSliderTwoDecimals(masterGainSlider);

    // ============== //
    //    ENVELOPE    //
    // ============== //

    adsrLabel.setText("ENV", juce::dontSendNotification);
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

    attackSlider.setNumDecimalPlacesToDisplay(2);
    decaySlider.setNumDecimalPlacesToDisplay(2);
    sustainSlider.setNumDecimalPlacesToDisplay(2);
    releaseSlider.setNumDecimalPlacesToDisplay(2);

    // ============== //
    // FILTER SECTION //
    // ============== //

    filterLabel.setText("FLTR", juce::dontSendNotification);
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

    osc1GainLabel.setText("gain", juce::dontSendNotification);
    osc1DetuneLabel.setText("detune", juce::dontSendNotification);
    osc1FmLabel.setText("FM", juce::dontSendNotification);

    carrierLabel.setText("CARRIER", juce::dontSendNotification);
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
    modulatorLabel.setText("MODULATOR", juce::dontSendNotification);
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
    // ===== Colours =====
    const auto backgroundColour = juce::Colour(0xff1e1e1e);

    const auto carrierFill  = juce::Colour(0xff001933);
    const auto modFill      = juce::Colour(0xff28643C);
    const auto adsrFill     = juce::Colour(0xff4C197F);
    const auto filterFill   = juce::Colour(0xff7F6B00);

    const auto carrierBorder = juce::Colours::darkred;
    const auto modBorder     = juce::Colours::darkblue;
    const auto adsrBorder    = juce::Colours::darkorange;
    const auto filterBorder  = juce::Colours::darkcyan;

    // ===== Layout constants =====
    constexpr int outerPadding      = 10;
    constexpr int sectionSpacing    = 10;
    constexpr int titleBarHeight    = 30;
    constexpr int topSectionHeight  = 270;
    constexpr int bottomHeight   = 240;
    constexpr float cornerSize      = 0.0f;
    constexpr float borderThickness = 2.0f;

    g.fillAll(backgroundColour);

    auto bounds = getLocalBounds().reduced(outerPadding);

    // Title
    bounds.removeFromTop(titleBarHeight);
    bounds.removeFromTop(sectionSpacing);

    // Top section (waveform + master) - skip drawing panel backgrounds here
    bounds.removeFromTop(topSectionHeight);
    bounds.removeFromTop(sectionSpacing);

    // Bottom row: 4 equal panels
    auto bottomArea = bounds.removeFromBottom(bottomHeight);
    const int panelWidth = (bottomArea.getWidth() - (sectionSpacing * 3)) / 4;

    auto carrierArea = bottomArea.removeFromLeft(panelWidth);
    bottomArea.removeFromLeft(sectionSpacing);

    auto modArea = bottomArea.removeFromLeft(panelWidth);
    bottomArea.removeFromLeft(sectionSpacing);

    auto adsrArea = bottomArea.removeFromLeft(panelWidth);
    bottomArea.removeFromLeft(sectionSpacing);

    auto filterArea = bottomArea;

    auto drawPanel = [&] (juce::Rectangle<int> area,
                          juce::Colour fill,
                          juce::Colour border)
    {
        g.setColour(fill);
        g.fillRect(area.toFloat());

        g.setColour(border);
        g.drawRoundedRectangle(area.toFloat(), cornerSize, borderThickness);
    };

    drawPanel(carrierArea, carrierFill, carrierBorder);
    drawPanel(modArea,     modFill,     modBorder);
    drawPanel(adsrArea,    adsrFill,    adsrBorder);
    drawPanel(filterArea,  filterFill,  filterBorder);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // ===== Layout constants =====
    constexpr int outerPadding      = 10;
    constexpr int sectionSpacing    = 10;
    constexpr int titleBarHeight    = 30;
    constexpr int topSectionHeight  = 270;
    constexpr int masterColumnWidth = 110;
    constexpr int panelHeaderHeight = 20;
    constexpr int comboRowHeight    = 40;
    constexpr int smallLabelHeight  = 18;
    constexpr int sliderRowHeight   = 105;
    constexpr int valueRowHeight    = 28;

    auto bounds = getLocalBounds().reduced(outerPadding);

    // ===== Title =====
    auto titleArea = bounds.removeFromTop(titleBarHeight);
    titleLabel.setBounds(titleArea);

    bounds.removeFromTop(sectionSpacing);

    // ===== Top area: waveform + master =====
    auto topArea = bounds.removeFromTop(topSectionHeight);

    auto masterArea = topArea.removeFromRight(masterColumnWidth);
    topArea.removeFromRight(sectionSpacing);

    waveformDisplay.setBounds(topArea);

    scopeModeLabel.setBounds(masterArea.removeFromTop(20));
    scopeModeBox.setBounds(masterArea.removeFromTop(24));
    masterArea.removeFromTop(8);
    masterGainSlider.setBounds(masterArea);

    auto masterContent = masterArea.reduced(10, 20);
    masterGainLabel.setBounds(masterContent.removeFromBottom(smallLabelHeight));
    masterGainSlider.setBounds(masterContent);

    bounds.removeFromTop(sectionSpacing);

    // ===== Bottom row: 4 equal panels =====
    constexpr int bottomHeight = 240;
    auto bottomArea = bounds.removeFromBottom(bottomHeight);

    const int panelWidth = (bottomArea.getWidth() - (sectionSpacing * 3)) / 4;

    auto carrierArea   = bottomArea.removeFromLeft(panelWidth);
    bottomArea.removeFromLeft(sectionSpacing);

    auto modArea       = bottomArea.removeFromLeft(panelWidth);
    bottomArea.removeFromLeft(sectionSpacing);

    auto adsrArea      = bottomArea.removeFromLeft(panelWidth);
    bottomArea.removeFromLeft(sectionSpacing);

    auto filterArea    = bottomArea;

    // =========================================================
    // Carrier
    // =========================================================
    {
        auto area = carrierArea.reduced(10);

        auto header = area.removeFromTop(panelHeaderHeight);
        auto buttonArea = header.removeFromRight(50);
        carrierLabel.setBounds(header);
        osc1OnButton.setBounds(buttonArea);

        auto comboRow = area.removeFromTop(comboRowHeight);
        auto waveBoxArea = comboRow.removeFromLeft(comboRow.getWidth() / 2);
        osc1WaveBox.setBounds(waveBoxArea.reduced(4));
        osc1PitchBox.setBounds(comboRow.reduced(4));

        auto comboLabels = area.removeFromTop(smallLabelHeight);
        auto waveLabelArea = comboLabels.removeFromLeft(comboLabels.getWidth() / 2);
        osc1WaveLabel.setBounds(waveLabelArea);
        osc1PitchLabel.setBounds(comboLabels);

        auto sliderRow = area.removeFromTop(sliderRowHeight);
        auto detuneArea = sliderRow.removeFromLeft(sliderRow.getWidth() / 3);
        auto gainArea   = sliderRow.removeFromLeft(sliderRow.getWidth() / 2);
        auto fmArea     = sliderRow;

        detune1Slider.setBounds(detuneArea.reduced(4));
        gain1Slider.setBounds(gainArea.reduced(4));
        fm1Slider.setBounds(fmArea.reduced(4));

        auto valueRow = area.removeFromTop(valueRowHeight);
        auto detuneValue = valueRow.removeFromLeft(valueRow.getWidth() / 3);
        auto gainValue   = valueRow.removeFromLeft(valueRow.getWidth() / 2);
        auto fmValue     = valueRow;

        osc1DetuneLabel.setBounds(detuneValue.reduced(4, 0));
        osc1GainLabel.setBounds(gainValue.reduced(4, 0));
        osc1FmLabel.setBounds(fmValue.reduced(4, 0));
    }

    // =========================================================
    // Modulator
    // =========================================================
    {
        auto area = modArea.reduced(10);

        auto header = area.removeFromTop(panelHeaderHeight);
        modulatorLabel.setBounds(header);

        auto comboRow = area.removeFromTop(comboRowHeight);
        auto waveBoxArea = comboRow.removeFromLeft(comboRow.getWidth() / 2);
        osc2WaveBox.setBounds(waveBoxArea.reduced(4));
        osc2PitchBox.setBounds(comboRow.reduced(4));

        auto comboLabels = area.removeFromTop(smallLabelHeight);
        auto waveLabelArea = comboLabels.removeFromLeft(comboLabels.getWidth() / 2);
        osc2WaveLabel.setBounds(waveLabelArea);
        osc2PitchLabel.setBounds(comboLabels);
    }

    // =========================================================
    // ADSR
    // =========================================================
    {
        auto area = adsrArea.reduced(10);

        adsrLabel.setBounds(area.removeFromTop(panelHeaderHeight));

        auto sliderRow = area.removeFromTop(120);
        const int w = sliderRow.getWidth() / 4;

        attackSlider.setBounds(sliderRow.removeFromLeft(w).reduced(4));
        decaySlider.setBounds(sliderRow.removeFromLeft(w).reduced(4));
        sustainSlider.setBounds(sliderRow.removeFromLeft(w).reduced(4));
        releaseSlider.setBounds(sliderRow.reduced(4));

        auto labelRow = area.removeFromTop(valueRowHeight);
        const int lw = labelRow.getWidth() / 4;

        attackLabel.setBounds(labelRow.removeFromLeft(lw));
        decayLabel.setBounds(labelRow.removeFromLeft(lw));
        sustainLabel.setBounds(labelRow.removeFromLeft(lw));
        releaseLabel.setBounds(labelRow);
    }

    // =========================================================
    // Filter
    // =========================================================
    {
        auto area = filterArea.reduced(10);

        filterLabel.setBounds(area.removeFromTop(panelHeaderHeight));
        filterType.setBounds(area.removeFromTop(30));

        auto sliderRow = area.removeFromTop(140);
        auto cutoffArea = sliderRow.removeFromLeft(sliderRow.getWidth() / 2);

        cutoffSlider.setBounds(cutoffArea.reduced(4));
        resonanceSlider.setBounds(sliderRow.reduced(4));

        auto labelRow = area.removeFromTop(valueRowHeight);
        auto cutoffLabelArea = labelRow.removeFromLeft(labelRow.getWidth() / 2);

        cutoffLabel.setBounds(cutoffLabelArea);
        resonanceLabel.setBounds(labelRow);
    }
}
