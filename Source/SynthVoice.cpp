//
// Created by alisdair chauvin on 12/2/25.
//

#include "SynthVoice.h"
#include "SynthSound.h"

//==============================================================================
bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

//==============================================================================
void SynthVoice::prepare (double sampleRate, int samplesPerBlock, int numChannels)
{
    osc1.prepare(sampleRate, samplesPerBlock, numChannels);
    osc2.prepare(sampleRate, samplesPerBlock, numChannels);

    adsr.setSampleRate(sampleRate);

    filterSpec.sampleRate = sampleRate;
    filterSpec.maximumBlockSize = samplesPerBlock;
    filterSpec.numChannels = numChannels;
    filter.prepare(filterSpec);

    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

//==============================================================================
void SynthVoice::startNote (int midiNoteNumber, float velocity,
                            juce::SynthesiserSound*, int)
{
    baseFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    level = velocity;

    osc1.reset();
    osc2.reset();

    isActive = true;
    adsr.noteOn();
}

//==============================================================================
void SynthVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
        adsr.noteOff();
    else
    {
        adsr.reset();
        isActive = false;
        clearCurrentNote();
    }
}

//==============================================================================
// Converts choice index → semitone offset
static float indexToSemitone(int index)
{
    switch (index)
    {
        case 0: return -12.f;
        case 1: return -7.f;
        case 2: return  0.f;
        case 3: return +7.f;
        case 4: return +12.f;
    }
    return 0.f;
}

//==============================================================================
void SynthVoice::updateFromParameters(float gain1, float pitchIndex1, float detune1,
                                      float gain2, float pitchIndex2, float detune2,
                                      float blendAmount)
{
    float semis1 = indexToSemitone((int)pitchIndex1);
    float semis2 = indexToSemitone((int)pitchIndex2);

    float pitchRatio1  = std::pow(2.f, semis1 / 12.f);
    float pitchRatio2  = std::pow(2.f, semis2 / 12.f);

    float detuneRatio1 = std::pow(2.f, detune1 / 1200.f);
    float detuneRatio2 = std::pow(2.f, detune2 / 1200.f);

    osc1.setGain(gain1);
    osc2.setGain(gain2);

    osc1.setFrequency(baseFrequency * pitchRatio1 * detuneRatio1);
    osc2.setFrequency(baseFrequency * pitchRatio2 * detuneRatio2);

    blend = juce::jlimit(0.f, 1.f, blendAmount);
}

//==============================================================================
void SynthVoice::updateEnvelope(float a, float d, float s, float r)
{
    adsrParams.attack  = a;
    adsrParams.decay   = d;
    adsrParams.sustain = s;
    adsrParams.release = r;
    adsr.setParameters(adsrParams);
}

//==============================================================================
void SynthVoice::updateFilter(float cutoff, float resonance, int type)
{
    filter.setCutoffFrequency(cutoff);
    filter.setResonance(resonance);

    switch (type)
    {
        case 0: filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);  break;
        case 1: filter.setType(juce::dsp::StateVariableTPTFilterType::highpass); break;
        case 2: filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass); break;
    }
}

//==============================================================================
void SynthVoice::updateOscillators(int wave1, int wave2, float blendAmount)
{
    osc1.setWaveform(wave1);
    osc2.setWaveform(wave2);

    blend = juce::jlimit(0.f, 1.f, blendAmount);
}

void SynthVoice::updateOscOnOff(bool o1, bool o2)
{
    osc1On = o1;
    osc2On = o2;
}

//==============================================================================
// FM + Mixing + Envelope + Filtering
void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                 int startSample, int numSamples)
{
    if (!isActive)
        return;

    const int numChannels = outputBuffer.getNumChannels();

    tempBuffer1.setSize(numChannels, numSamples, false, false, true);
    tempBuffer2.setSize(numChannels, numSamples, false, false, true);
    mixBuffer  .setSize(numChannels, numSamples, false, false, true);

    tempBuffer1.clear();
    tempBuffer2.clear();
    mixBuffer.clear();

    osc1.process(tempBuffer1);
    osc2.process(tempBuffer2);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* dst = mixBuffer.getWritePointer(ch);
        auto* o1  = tempBuffer1.getReadPointer(ch);
        auto* o2  = tempBuffer2.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float s1 = osc1On ? o1[i] : 0.0f;
            float s2 = osc2On ? o2[i] : 0.0f;

            // Absolute mute if gain is too low (prevents saw bleed)
            if (std::abs(s1) < 1e-6f) s1 = 0.0f;
            if (std::abs(s2) < 1e-6f) s2 = 0.0f;

            float mixed = s1 * (1.0f - blend)
                        + s2 * blend;

            dst[i] = mixed * level;
        }
    }

    // Apply envelope
    adsr.applyEnvelopeToBuffer(mixBuffer, 0, numSamples);

    // Filter
    juce::dsp::AudioBlock<float> block(mixBuffer);
    filter.process(juce::dsp::ProcessContextReplacing<float>(block));

    // Add to output buffer
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* dst = outputBuffer.getWritePointer(ch, startSample);
        auto* src = mixBuffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
            dst[i] += src[i];
    }

    if (!adsr.isActive())
    {
        isActive = false;
        clearCurrentNote();
    }
}
