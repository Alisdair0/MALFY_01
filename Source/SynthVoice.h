//
// Created by alisdair chauvin on 12/2/25.
//

#ifndef EFFEM_UNIT_SYNTHVOICE_H
#define EFFEM_UNIT_SYNTHVOICE_H

#pragma once

#include <juce_dsp/juce_dsp.h>
#include "Oscillator.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice() = default;

    // ===== JUCE-required overrides =====
    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int pitchWheelPos) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    void renderNextBlock (juce::AudioBuffer<float>&,
                          int startSample, int numSamples) override;

    void prepare (double sampleRate, int samplesPerBlock, int numChannels);

    // ===== Runtime parameter updates =====
    void updateFromParameters (float gain1, float pitchIndex1, float detune1,
                               float gain2, float pitchIndex2, float detune2,
                               float blendAmount, float fm1, float fm2);

    void updateEnvelope (float attack, float decay, float sustain, float release);
    void updateFilter (float cutoff, float resonance, int type);
    void updateOscillators (int wave1, int wave2);
    void updateOscOnOff (bool o1, bool o2);

private:
    Oscillator osc1, osc2;
    juce::AudioBuffer<float> tempBuffer1, tempBuffer2, mixBuffer;

    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;

    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::ProcessSpec filterSpec;

    // voice state
    float baseFrequency = 440.0f;
    float level = 0.0f;
    bool isActive = false;

    float osc1BaseFreq = 440.0f;
    float osc2BaseFreq = 440.0f;

    bool osc1On = true;
    bool osc2On = true;

    float osc1FM = 0.0f;
    float osc2FM = 0.0f;
    float osc1Gain = 1.0f;
    float osc2Gain = 1.0f;
    float blend = 0.5f;
};

#endif //EFFEM_UNIT_SYNTHVOICE_H