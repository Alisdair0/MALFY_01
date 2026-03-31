#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor (BusesProperties()
        #if ! JucePlugin_IsMidiEffect
         #if ! JucePlugin_IsSynth
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
         #endif
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
        #endif
      ), state (*this, nullptr, "parameters", createParameters())
{
    synth.clearVoices();
    for (int i = 0; i < 8; ++i)         // 8-voice polyphony
        synth.addVoice (new SynthVoice);

    synth.clearSounds();
    synth.addSound (new SynthSound);
}


AudioPluginAudioProcessor::~AudioPluginAudioProcessor() = default;

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int numCh = getTotalNumOutputChannels();
    synth.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
            v->prepare(sampleRate, samplesPerBlock, numCh);   // USE numCh
    }

    // ======== Parameters ============ //

    // Buttons & sliders
    playParam = state.getRawParameterValue ("play");
    masterGainParam = state.getRawParameterValue ("masterGain");
    pitchChoiceParam = state.getRawParameterValue ("pitchShift");
    panParam = state.getRawParameterValue ("pan");

    // Envelopes
    attackParam  = state.getRawParameterValue ("attack");
    decayParam   = state.getRawParameterValue ("decay");
    sustainParam = state.getRawParameterValue ("sustain");
    releaseParam = state.getRawParameterValue ("release");

    // Filter
    filterCutoffParam    = state.getRawParameterValue("filterCutoff");
    filterResonanceParam = state.getRawParameterValue("filterResonance");
    filterTypeParam      = state.getRawParameterValue("filterType");

    // osc 1
    osc1OnParam     = state.getRawParameterValue("osc1On");
    osc1WaveParam   = state.getRawParameterValue("osc1Wave");
    osc1PitchParam  = state.getRawParameterValue("osc1Pitch");
    osc1DetuneParam = state.getRawParameterValue("osc1Detune");
    osc1GainParam   = state.getRawParameterValue("osc1Gain");
    osc1FMParam   = state.getRawParameterValue("osc1FM");

    // osc 2
    osc2OnParam     = state.getRawParameterValue("osc2On");
    osc2WaveParam   = state.getRawParameterValue("osc2Wave");
    osc2PitchParam  = state.getRawParameterValue("osc2Pitch");
    osc2DetuneParam = state.getRawParameterValue("osc2Detune");
    osc2GainParam   = state.getRawParameterValue("osc2Gain");
    osc2FMParam   = state.getRawParameterValue("osc2FM");

    // blend
    blendParam      = state.getRawParameterValue("oscBlend");
}


void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    // ===================== ADSR / FILTER / GLOBAL ===================== //

    float attack  = *state.getRawParameterValue("attack");
    float decay   = *state.getRawParameterValue("decay");
    float sustain = *state.getRawParameterValue("sustain");
    float release = *state.getRawParameterValue("release");

    float cutoff    = *state.getRawParameterValue("filterCutoff");
    float resonance = *state.getRawParameterValue("filterResonance");
    int   filterType= (int)*state.getRawParameterValue("filterType");

    float pan = *state.getRawParameterValue("pan");

    // ===================== MAP RAW PARAMETERS ===================== //

    // On/off (bool)
    bool osc1On  = osc1OnParam ? (bool)*osc1OnParam : true;
    bool osc2On  = osc2OnParam ? (bool)*osc2OnParam : true;

    // Waveforms (choice -> int)
    int wave1 = osc1WaveParam ? (int) std::round(*osc1WaveParam) : 0;
    int wave2 = osc2WaveParam ? (int) std::round(*osc2WaveParam) : 0;

    // Pitch (choice -> int)
    int pitch1 = osc1PitchParam ? (int) std::round(*osc1PitchParam) : 4;  // index 4 = "0"
    int pitch2 = osc2PitchParam ? (int) std::round(*osc2PitchParam) : 4;

    float detune1 = osc1DetuneParam ? osc1DetuneParam->load() : 0.0f;
    float detune2 = osc2DetuneParam ? osc2DetuneParam->load() : 0.0f;

    float gain1   = osc1GainParam   ? osc1GainParam->load()   : 0.8f;
    float gain2   = osc2GainParam   ? osc2GainParam->load()   : 0.8f;

    float osc1FM = osc1FMParam ? osc1FMParam->load() : 0.0f;
    float osc2FM = osc2FMParam ? osc2FMParam->load() : 0.0f;

    float blend   = blendParam      ? blendParam->load()      : 0.5f;

    // ===================== UPDATE ALL VOICES ===================== //

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            // waveforms + blend
            v->updateOscillators(wave1, wave2, blend);

            // oscillator on/off states
            v->updateOscOnOff(osc1On, osc2On);

            // pitch, detune, gain for each oscillator
            v->updateFromParameters(
                gain1, (float)pitch1, detune1,
                gain2, (float)pitch2, detune2,
                blend,
                osc1FM, osc2FM
            );

            // envelopes + filters
            v->updateEnvelope(attack, decay, sustain, release);
            v->updateFilter(cutoff, resonance, filterType);
        }
    }

    // ===================== RENDER SYNTH ===================== //

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // ===================== PAN ===================== //

    if (buffer.getNumChannels() >= 2)
    {
        const float angle = (pan + 1.0f) * juce::MathConstants<float>::halfPi * 0.5f;
        float leftGain  = std::cos(angle);
        float rightGain = std::sin(angle);

        auto* left  = buffer.getWritePointer(0);
        auto* right = buffer.getWritePointer(1);

        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            left[s]  *= leftGain;
            right[s] *= rightGain;
        }
    }

    // ===================== MASTER GAIN ===================== //
    float gainDb = *state.getRawParameterValue("masterGain");
    float gain   = juce::Decibels::decibelsToGain(gainDb);
    buffer.applyGain(gain);

    // ===================== PLAY PARAM (MUTE) ===================== //

    if (playParam && ! (bool)playParam->load())
        buffer.clear();

    // ===================== VISUALIZER ======================= //
    auto* read = buffer.getReadPointer(0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        pushNextSampleIntoScope(read[i]);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            // Use channel 0 or the average — here we use CH0:
            if (ch == 0)
                pushNextSampleIntoScope(data[i]);
        }
    }
}


//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto valueTree = state.copyState();

    std::unique_ptr<juce::XmlElement> xml (valueTree.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary(data, sizeInBytes));

    if (xml && xml->hasTagName(state.state.getType()))
    {
        auto vt = juce::ValueTree::fromXml(*xml);
        state.replaceState(vt);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameters()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // ======= GLOBAL SLIDERS & BUTTONS ====== //
    params.push_back (std::make_unique<AudioParameterBool>(
        "play", "Play", true));

    params.push_back (std::make_unique<AudioParameterFloat>(
    "masterGain", "Master Gain",
    NormalisableRange<float> (-60.0f, 0.0f, 0.01f),
    -6.0f));

    params.push_back (std::make_unique<AudioParameterFloat>(
        "pan", "Pan",
        -1.0f, 1.0f, 0.0f)); // -1 = left, 0 = center, +1 = right

    // ============== ADSR =================== //
    params.push_back (std::make_unique<AudioParameterFloat>(
        "attack", "Attack",
        NormalisableRange<float> (0.001f, 5.0f, 0.0f, 0.5f),
        0.01f));

    params.push_back (std::make_unique<AudioParameterFloat>(
        "decay", "Decay",
        NormalisableRange<float> (0.001f, 5.0f, 0.0f, 0.5f),
        0.1f));

    params.push_back (std::make_unique<AudioParameterFloat>(
        "sustain", "Sustain",
        NormalisableRange<float> (0.0f, 1.0f, 0.0f, 1.0f),
        0.8f));

    params.push_back (std::make_unique<AudioParameterFloat>(
        "release", "Release",
        NormalisableRange<float> (0.001f, 5.0f, 0.0f, 0.5f),
        0.2f));

    // ========== FILTER CONTROLS ========== //
    params.push_back(std::make_unique<AudioParameterFloat>(
        "filterCutoff", "Cutoff",
        NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.33f), 1000.0f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        "filterResonance", "Resonance",
        NormalisableRange<float>(0.1f, 1.5f, 0.001f), 0.2f));

    params.push_back(std::make_unique<AudioParameterChoice>(
        "filterType", "Filter Type",
        StringArray { "Lowpass", "Highpass", "Bandpass" },
        0     // 0 = LP, 1 = HP, 2 = BP
    ));

    // ========== OSC1 ========== //
    params.push_back(std::make_unique<AudioParameterBool>(
        "osc1On", "OSC1 On", true));

    params.push_back(std::make_unique<AudioParameterChoice>(
        "osc1Wave", "OSC1 Waveform",
        StringArray{ "Sine","Square","Saw","Triangle","Noise","Add1","Add2" }, 0));

    // 5-point pitch for OSC1 (matches editor)
    params.push_back(std::make_unique<AudioParameterChoice>(
    "osc1Pitch", "OSC1 Pitch",
    StringArray{ "+12","+7","0","-7","-12" }, 2));

    params.push_back(std::make_unique<AudioParameterFloat>(
        "osc1Detune", "OSC1 Detune", -100.f, 100.f, 0.f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        "osc1Gain", "OSC1 Gain", 0.f, 1.f, 0.8f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
    "osc1FM", "OSC1 FM", 0.0f, 100.0f, 0.0f));

    // ========== OSC2 ========== //
    params.push_back(std::make_unique<AudioParameterBool>(
        "osc2On", "OSC2 On", true));

    params.push_back(std::make_unique<AudioParameterChoice>(
        "osc2Wave", "OSC2 Waveform",
        StringArray{ "Sine","Square","Saw","Triangle","Noise","Add1","Add2" }, 0));

    params.push_back(std::make_unique<AudioParameterChoice>(
    "osc2Pitch", "OSC2 Pitch",
    StringArray{ "+12","+7","0","-7","-12" }, 2));

    params.push_back(std::make_unique<AudioParameterFloat>(
        "osc2Detune", "OSC2 Detune", -100.f, 100.f, 0.f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        "osc2Gain", "OSC2 Gain", 0.f, 1.f, 0.8f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
    "osc2FM", "OSC2 FM", 0.0f, 100.0f, 0.0f));

    // ========== BLEND ========== //
    params.push_back(std::make_unique<AudioParameterFloat>(
        "oscBlend", "OSC Blend", 0.f, 1.f, 0.5f));

    return { params.begin(), params.end() };
}
