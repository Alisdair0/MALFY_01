#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SynthVoice.h"

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
    static constexpr int scopeSize = 1024;   // oscilloscope resolution

    std::atomic<int> scopeWritePos { 0 };
    std::array<float, scopeSize> scopeData {};

    // Trigger display buffer
    static constexpr int triggeredSize = 256;
    std::array<float, triggeredSize> triggeredBuffer{};
    std::atomic<bool> triggeredBufferReady { false };
    std::atomic<float> triggerPhaseOffset { 0.0f };

    std::atomic<float> scopeRMS { 0.0f };

    // Call this for each sample to feed the oscilloscope
    inline void pushNextSampleIntoScope (float sample)
{
    // Ring buffer (scrolling mode)
    auto index = static_cast<std::size_t> (scopeWritePos.load (std::memory_order_relaxed));
    scopeData[index] = sample;
    index = (index + 1) % static_cast<std::size_t> (scopeSize);
    scopeWritePos.store (static_cast<int> (index), std::memory_order_relaxed);

    // RMS
    rmsAccumulator += sample * sample;
    ++rmsSampleCount;

    if (rmsSampleCount >= rmsWindowSize)
    {
        scopeRMS.store (std::sqrt (rmsAccumulator / static_cast<float> (rmsWindowSize)),
                        std::memory_order_relaxed);
        rmsAccumulator = 0.0f;
        rmsSampleCount = 0;
    }

    // Triggered capture
    if (triggerCapturing)
    {
        triggeredBuffer[static_cast<std::size_t> (triggerFillCount)] = sample;
        ++triggerFillCount;

        if (triggerFillCount >= triggeredSize)
        {
            triggerCapturing = false;
            triggerFillCount = 0;
            triggerHoldoff   = triggeredSize;
            triggeredBufferReady.store (true, std::memory_order_release);
        }
    }
    else
    {
        if (triggerHoldoff > 0)
        {
            --triggerHoldoff;
        }
        else
        {
            // Rising zero crossing — interpolate fractional offset
            if (triggerPrevSample < 0.0f && sample >= 0.0f)
            {
                // How far between prev and curr the zero actually sits, in [0, 1)
                const float denom = triggerPrevSample - sample; // negative - positive = negative
                triggerPhaseOffset = (denom != 0.0f)
                                     ? triggerPrevSample / denom  // lands in [0, 1)
                                     : 0.0f;

                triggerCapturing = true;
                triggerFillCount = 0;
            }
        }
    }

    triggerPrevSample = sample;
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
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // OSC1 parameters
    std::atomic<float>* osc1OnParam      = nullptr;
    std::atomic<float>* osc1WaveParam    = nullptr;
    std::atomic<float>* osc1PitchParam   = nullptr;
    std::atomic<float>* osc1DetuneParam  = nullptr;
    std::atomic<float>* osc1GainParam    = nullptr;
    std::atomic<float>* osc1FMParam      = nullptr;

    // OSC2 parameters
    std::atomic<float>* osc2OnParam      = nullptr;
    std::atomic<float>* osc2WaveParam    = nullptr;
    std::atomic<float>* osc2PitchParam   = nullptr;
    std::atomic<float>* osc2DetuneParam  = nullptr;
    std::atomic<float>* osc2GainParam    = nullptr;
    std::atomic<float>* osc2FMParam      = nullptr;

    // Blend
    std::atomic<float>* blendParam       = nullptr;

    // Trigger capture state
    int   triggerFillCount  { 0 };
    int   triggerHoldoff    { 0 };
    float triggerPrevSample { 0.0f };
    bool  triggerCapturing  { false };

    // RMS state
    static constexpr int rmsWindowSize = 256;
    float rmsAccumulator { 0.0f };
    int   rmsSampleCount { 0 };


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};