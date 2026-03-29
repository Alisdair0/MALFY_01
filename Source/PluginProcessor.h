#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Oscillator.h"
#include "SynthVoice.h"
#include "SynthSound.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;


    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getState() { return state; }

    // Visualizer
    static constexpr int scopeSize = 512;   // oscilloscope resolution

    std::atomic<int> scopeWritePos { 0 };
    std::array<float, scopeSize> scopeData {};

    // Call this for each sample to feed the oscilloscope
    inline void pushNextSampleIntoScope(float sample)
    {
        int index = scopeWritePos.load(std::memory_order_relaxed);

        scopeData[index] = sample;

        index = (index + 1) % scopeSize;

        scopeWritePos.store(index, std::memory_order_relaxed);
    }

private:
    juce::Synthesiser synth;

    // parameters
    std::atomic<float>* playParam = nullptr;
    std::atomic<float>* masterGainParam = nullptr;
    std::atomic<float>* detuneParam = nullptr;
    std::atomic<float>* pitchChoiceParam = nullptr; // choice index
    std::atomic<float>* panParam = nullptr;
    std::atomic<float>* fmAmountParam = nullptr;

    // envelope
    std::atomic<float>* attackParam  = nullptr;
    std::atomic<float>* decayParam   = nullptr;
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;

    // filter
    std::atomic<float>* filterCutoffParam = nullptr;
    std::atomic<float>* filterResonanceParam = nullptr;
    std::atomic<float>* filterTypeParam = nullptr;

    // waveforms
    juce::ComboBox waveformBox;
    juce::Label waveformLabel { "waveformLabel", "Waveform" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;

    // Connection between processor & editor
    juce::AudioProcessorValueTreeState state;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // OSC1 parameters
    std::atomic<float>* osc1OnParam      = nullptr;
    std::atomic<float>* osc1WaveParam    = nullptr;
    std::atomic<float>* osc1PitchParam   = nullptr;
    std::atomic<float>* osc1DetuneParam  = nullptr;
    std::atomic<float>* osc1GainParam    = nullptr;
    std::atomic<float>* osc1FmParam      = nullptr;

    // OSC2 parameters
    std::atomic<float>* osc2OnParam      = nullptr;
    std::atomic<float>* osc2WaveParam    = nullptr;
    std::atomic<float>* osc2PitchParam   = nullptr;
    std::atomic<float>* osc2DetuneParam  = nullptr;
    std::atomic<float>* osc2GainParam    = nullptr;
    std::atomic<float>* osc2FmParam      = nullptr;

    // Blend
    std::atomic<float>* blendParam       = nullptr;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};