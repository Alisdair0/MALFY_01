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
    explicit WaveformDisplay(AudioPluginAudioProcessor& p);

    void paint(juce::Graphics& g) override;
    void resized() override {}

private:
    AudioPluginAudioProcessor& processor;

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
    AudioPluginAudioProcessor& processorRef;

    // Play
    juce::ToggleButton playButton { "Play" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> playAttachment;

    // Master Gain
    juce::Slider masterGainSlider;
    juce::Label masterGainLabel { "MasterGainLabel", "Master Gain" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterGainAttachment;

    // Detune (vertical)
    juce::Slider detuneSlider;
    juce::Label detuneLabel { "DetuneLabel", "Detune (cents)" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment;

    // Pitch Shift (combo)
    juce::ComboBox pitchShiftBox;
    juce::Label pitchShiftLabel { "PitchShiftLabel", "Pitch (semitones)" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pitchShiftAttachment;

    // Pan
    juce::Slider panSlider;
    juce::Label panLabel { "PanLabel", "Pan" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;

    // ADSR sliders + labels
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;

    juce::Label attackLabel  { "attackLabel",  "Attack" };
    juce::Label decayLabel   { "decayLabel",   "Decay" };
    juce::Label sustainLabel { "sustainLabel", "Sustain" };
    juce::Label releaseLabel { "releaseLabel", "Release" };

    // ADSR attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    // Filters
    juce::Label filterLabel;
    juce::Label cutoffLabel    { "cutoffLabel",    "Cutoff" };
    juce::Label resonanceLabel { "resonanceLabel", "Resonance" };
    juce::ComboBox filterType;

    juce::Slider cutoffSlider;
    juce::Slider resonanceSlider;

    // Filter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;

    // OSC1
    juce::ComboBox osc1WaveBox;
    juce::ComboBox osc1PitchBox;
    juce::Slider   detune1Slider;
    juce::Slider   gain1Slider;
    juce::Slider   fm1Slider;

    // OSC1 attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc1WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc1PitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc1DetuneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc1GainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc1FmAttachment;

    // OSC1 labels
    juce::Label osc1GainLabel      { "osc1GainLabel",      "Gain" };
    juce::Label osc1DetuneLabel    { "osc1DetuneLabel",    "Detune" };
    juce::Label osc1FmLabel        { "osc1FmLabel",        "FM" };
    juce::Label osc1PitchLabel     { "osc1PitchLabel",     "Pitch" };
    juce::Label osc1WaveLabel      { "osc1WaveLabel",      "Wave" };

    // OSC2
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
    juce::Label osc2GainLabel      { "osc2GainLabel",      "Gain" };
    juce::Label osc2DetuneLabel    { "osc2DetuneLabel",    "Detune" };
    juce::Label osc2FmLabel        { "osc2FmLabel",        "FM" };
    juce::Label osc2PitchLabel     { "osc2PitchLabel",     "Pitch" };
    juce::Label osc2WaveLabel      { "osc2WaveLabel",      "Wave" };

    // Blend
    juce::Slider blendSlider;
    juce::Label blendLabel { "blendLabel", "Osc Blend" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> blendAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
