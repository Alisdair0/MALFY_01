//
// Created by alisdair chauvin on 12/2/25.
//

#include "SynthVoice.h"
#include "SynthSound.h"

SynthVoice::SynthVoice()
    : filterSpec{}
{}

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
    filterSpec.maximumBlockSize = static_cast<uint32_t>(samplesPerBlock);
    filterSpec.numChannels = static_cast<juce::uint32>(numChannels);
    filter.prepare(filterSpec);

    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

//==============================================================================
void SynthVoice::startNote (int midiNoteNumber, float velocity,
                            juce::SynthesiserSound*, int)
{
    baseFrequency = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
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
        case 0: return +12.f;
        case 1: return +7.f;
        case 2: return  0.f;
        case 3: return -7.f;
        case 4: return -12.f;
        default: return  0.f;
    }
}

//==============================================================================
void SynthVoice::updateFromParameters(float gain1, float pitchIndex1, float detune1,
                                      float gain2, float pitchIndex2, float detune2,
                                      float blendAmount, float fm1, float fm2)
{
    float semis1 = indexToSemitone(static_cast<int>(pitchIndex1));
    float semis2 = indexToSemitone(static_cast<int>(pitchIndex2));

    float pitchRatio1  = std::pow(2.f, semis1 / 12.f);
    float pitchRatio2  = std::pow(2.f, semis2 / 12.f);

    float detuneRatio1 = std::pow(2.f, detune1 / 1200.f);
    float detuneRatio2 = std::pow(2.f, detune2 / 1200.f);

    osc1Gain = gain1;
    osc2Gain = gain2;

    osc1BaseFreq = baseFrequency * pitchRatio1 * detuneRatio1;
    osc2BaseFreq = baseFrequency * pitchRatio2 * detuneRatio2;

    osc1.setFrequency(osc1BaseFreq);
    osc2.setFrequency(osc2BaseFreq);

    osc1FM = fm1;
    osc2FM = fm2;

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
        default: break;
    }
}

//==============================================================================
void SynthVoice::updateOscillators(int wave1, int wave2)
{
    osc1.setWaveform(wave1);
    osc2.setWaveform(wave2);
}

void SynthVoice::updateOscOnOff(bool o1)
{
    osc1On = o1;
}

//==============================================================================
// FM + Mixing + Envelope + Filtering
void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                 int startSample, int numSamples)
{
    if (!isActive)
        return;

    const int numChannels = outputBuffer.getNumChannels();

    mixBuffer.setSize(numChannels, numSamples, false, false, true);
    mixBuffer.clear();

    for (int i = 0; i < numSamples; ++i)
    {
        float modSample =  osc2.processSample(0.0f);

        // FM index style scaling
        float fmIndex = osc1FM / 100.0f;

        // stronger deviation based on modulator frequency
        float fmDepthHz = osc2BaseFreq * fmIndex * 4.0f;

        // Apply frequency modulation & clamp range
        float currentFreq = osc1BaseFreq + (modSample * fmDepthHz);
        currentFreq = juce::jlimit(1.0f, 20000.0f, currentFreq);

        // Update frequency per sample
        osc1.setFrequency(currentFreq);

        // Generate the carrier output after modulation and trim output
        float s1 = osc1On ? osc1.processSample(0.0f) * osc1Gain : 0.0f;
        float out = s1 * level * 0.3f;
        for (int ch = 0; ch < numChannels; ++ch)
            mixBuffer.setSample(ch, i, out);
    }

    adsr.applyEnvelopeToBuffer(mixBuffer, 0, numSamples);

    juce::dsp::AudioBlock<float> block(mixBuffer);
    filter.process(juce::dsp::ProcessContextReplacing<float>(block));

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

