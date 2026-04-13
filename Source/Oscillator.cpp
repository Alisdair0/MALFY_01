#include "Oscillator.h"

Oscillator::Oscillator() {}

void Oscillator::prepare(double sampleRate, int samplesPerBlock, int channels)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = channels;

    osc.prepare(spec);

    // Default waveform
    initWaveform(0);

    // Disable smoothing
    osc.setFrequency(440.0f, true);
}

void Oscillator::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);

    osc.process(context);
}

void Oscillator::setFrequency(float freq)
{
    baseFrequency = freq;        // store the frequency so FM can modify it later
    osc.setFrequency(freq, true); // true = no smoothing
}

void Oscillator::setWaveform(int type)
{
    initWaveform(type);
}

void Oscillator::reset()
{
    // Reset phase to zero
    osc.reset();
}

void Oscillator::initWaveform(int waveformIndex)
{
    switch (waveformIndex)
    {
        case 0: // Sine
            osc.initialise([](float x) { return std::sin(x); }, 128);
            break;

        case 1: // Square
            osc.initialise([](float x)
            {
                return std::sin(x) >= 0.0f ? 1.0f : -1.0f;
            }, 128);
            break;

        case 2: // Saw
            osc.initialise([](float x)
            {
                return juce::jmap(x,
                                  -juce::MathConstants<float>::pi,
                                   juce::MathConstants<float>::pi,
                                  -1.0f, 1.0f);
            }, 128);
            break;

        case 3: // Triangle
            osc.initialise([](float x)
            {
                return asinf(std::sin(x)) * (2.0f / juce::MathConstants<float>::pi);
            }, 128);
            break;

        case 4: // Noise
            osc.initialise([](float)
            {
                return juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
            }, 256);
            break;

        case 5:
            osc.initialise([](float x)
            {
                return std::sin(x) + 0.3f * std::sin(2.0f * x);
            }, 128);
            break;

        case 6:
            osc.initialise([](float x)
            {
                return std::sin(x)
                       + 0.3f * std::sin(2.0f * x)
                       + 0.15f * std::sin(3.0f * x);
            }, 128);
            break;

        default:
            osc.initialise([](float x) { return std::sin(x); }, 128);
            break;
    }
}

void Oscillator::processWithFM (juce::AudioBuffer<float>& buffer,
                               const float* fmBuffer,
                               float fmDepth)
{
    auto numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* out = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float mod = fmBuffer[i];

            float freqOffset = mod * fmDepth * baseFrequency;
            float freq = baseFrequency + freqOffset;

            freq = juce::jlimit(20.0f, 20000.0f, freq);

            if (i % 4 == 0) // update every 4 samples
                osc.setFrequency(freq, false);
        }
    }
}

float Oscillator::processSample(float input)
{
    return osc.processSample(input);
}