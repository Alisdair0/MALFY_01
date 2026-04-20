#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

//===================================//
//   WAVEFORM VISUALIZER COMPONENT   //
//===================================//
class WaveformDisplay : public juce::Component,
                        private juce::Timer
{
public:
    enum class ScopeMode
    {
        scrolling = 1,
        triggered = 2
    };

    explicit WaveformDisplay(AudioPluginAudioProcessor& p);

    void paint(juce::Graphics& g) override;
    void resized() override {}

    void setScopeMode (ScopeMode newMode) { scopeMode = newMode; repaint(); }

private:
    AudioPluginAudioProcessor& processor;

    ScopeMode scopeMode { ScopeMode::scrolling };

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};

//==============================================================================//

class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    WaveformDisplay waveformDisplay;
    juce::ComboBox scopeModeBox;
    juce::Label scopeModeLabel;

    AudioPluginAudioProcessor& processorRef;

    // Title
    juce::Label titleLabel;

    // Play
    juce::ToggleButton playButton { "Play" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> playAttachment;

    // Master Gain
    juce::Slider masterGainSlider;
    juce::Label masterGainLabel { "MasterGainLabel", "MASTER GAIN" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterGainAttachment;

    // Detune (vertical)
    juce::Slider detuneSlider;
    juce::Label detuneLabel { "DetuneLabel", "DETUNE (cents)" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment;

    // Pitch Shift (combo)
    juce::ComboBox pitchShiftBox;
    juce::Label pitchShiftLabel { "PitchShiftLabel", "PITCH (semitones)" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pitchShiftAttachment;

    // Pan
    juce::Slider panSlider;
    juce::Label panLabel { "PanLabel", "Pan" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;

    // ADSR sliders + labels
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;

    juce::Label adsrLabel;
    juce::Label attackLabel  { "attackLabel",  "ATCK" };
    juce::Label decayLabel   { "decayLabel",   "DEC" };
    juce::Label sustainLabel { "sustainLabel", "SUST" };
    juce::Label releaseLabel { "releaseLabel", "REL" };

    // ADSR attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    // Filters
    juce::Label filterLabel;
    juce::Label cutoffLabel    { "cutoffLabel",    "CTFF" };
    juce::Label resonanceLabel { "resonanceLabel", "RES" };
    juce::ComboBox filterType;

    juce::Slider cutoffSlider;
    juce::Slider resonanceSlider;

    // Filter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;

    // OSC1
    juce::ToggleButton osc1OnButton;
    juce::ComboBox osc1WaveBox;
    juce::ComboBox osc1PitchBox;
    juce::Slider   detune1Slider;
    juce::Slider   gain1Slider;
    juce::Slider   fm1Slider;

    // OSC1 attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> osc1OnAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc1WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc1PitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc1DetuneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc1GainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc1FmAttachment;

    // OSC1 labels
    juce::Label carrierLabel;
    juce::Label osc1GainLabel      { "osc1GainLabel",      "gain" };
    juce::Label osc1DetuneLabel    { "osc1DetuneLabel",    "detune" };
    juce::Label osc1FmLabel        { "osc1FmLabel",        "FM" };
    juce::Label osc1PitchLabel     { "osc1PitchLabel",     "pitch" };
    juce::Label osc1WaveLabel      { "osc1WaveLabel",      "wave" };

    // OSC2
    juce::ToggleButton osc2OnButton;
    juce::ComboBox osc2WaveBox;
    juce::ComboBox osc2PitchBox;
    juce::Slider   detune2Slider;
    juce::Slider   gain2Slider;
    juce::Slider   fm2Slider;

    // OSC2 attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc2WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc2PitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc2DetuneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc2GainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc2FmAttachment;

    // OSC2 labels
    juce::Label modulatorLabel;
    juce::Label osc2GainLabel      { "osc2GainLabel",      "gain" };
    juce::Label osc2DetuneLabel    { "osc2DetuneLabel",    "detune" };
    juce::Label osc2FmLabel        { "osc2FmLabel",        "FM" };
    juce::Label osc2PitchLabel     { "osc2PitchLabel",     "pitch" };
    juce::Label osc2WaveLabel      { "osc2WaveLabel",      "wave" };

    // Blend
    juce::Slider blendSlider;
    juce::Label blendLabel { "blendLabel", "Osc Blend" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> blendAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
