/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

//==============================================================================

namespace IDs
{
    static String mainType  { "mainType" };
    static String mainFreq  { "mainfreq" };
    static String mainLevel { "mainlevel" };
    static String lfoType   { "lfoType" };
    static String lfoFreq   { "lfofreq" };
    static String lfoLevel  { "lfolevel" };
    static String vfoType   { "vfoType" };
    static String vfoFreq   { "vfofreq" };
    static String vfoLevel  { "vfolevel" };

    static Identifier oscilloscope { "oscilloscope" };
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    auto freqRange = juce::NormalisableRange<float> {20.0f, 20000.0f,
        [](float start, float end, float normalised)
        {
            return start + (std::pow (2.0f, normalised * 10.0f) - 1.0f) * (end - start) / 1023.0f;
        },
        [](float start, float end, float value)
        {
            return (std::log (((value - start) * 1023.0f / (end - start)) + 1.0f) / std::log ( 2.0f)) / 10.0f;
        },
        [](float start, float end, float value)
        {
            if (value > 3000.0f)
                return jlimit (start, end, 100.0f * roundToInt (value / 100.0f));

            if (value > 1000.0f)
                return jlimit (start, end, 10.0f * roundToInt (value / 10.0f));

            return jlimit (start, end, float (roundToInt (value)));
        }};

    AudioProcessorValueTreeState::ParameterLayout layout;
    auto generator = std::make_unique<AudioProcessorParameterGroup>("Generator", TRANS ("Generator"), "|");
    generator->addChild (std::make_unique<AudioParameterChoice>(IDs::mainType, "Type", StringArray ("None", "Sine", "Triangle", "Square"), 1),
                         std::make_unique<AudioParameterFloat>(IDs::mainFreq, "Frequency", freqRange, 440),
                         std::make_unique<AudioParameterFloat>(IDs::mainLevel, "Level", NormalisableRange<float>(-100.0f, 0.0, 1.0), -6.0f));

    auto lfo = std::make_unique<AudioProcessorParameterGroup>("lfo", TRANS ("LFO"), "|");
    lfo->addChild (std::make_unique<AudioParameterChoice>(IDs::lfoType, "LFO-Type", StringArray ("None", "Sine", "Triangle", "Square"), 0),
                   std::make_unique<AudioParameterFloat>(IDs::lfoFreq, "Frequency", NormalisableRange<float>(0.25f, 10.0f), 2.0f),
                         std::make_unique<AudioParameterFloat>(IDs::lfoLevel, "Level", NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    auto vfo = std::make_unique<AudioProcessorParameterGroup>("vfo", TRANS ("VFO"), "|");
    vfo->addChild (std::make_unique<AudioParameterChoice>(IDs::vfoType, "VFO-Type", StringArray ("None", "Sine", "Triangle", "Square"), 0),
                   std::make_unique<AudioParameterFloat>(IDs::vfoFreq, "Frequency", NormalisableRange<float>(0.5f, 10.0f), 2.0f),
                   std::make_unique<AudioParameterFloat>(IDs::vfoLevel, "Level", NormalisableRange<float>(0.0f, 1.0), 0.0f));

    layout.add (std::move (generator),
                std::move (lfo),
                std::move (vfo));

    return layout;
}

//==============================================================================
SignalGeneratorAudioProcessor::SignalGeneratorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
    :
#endif
    treeState (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    frequency = treeState.getRawParameterValue (IDs::mainFreq);
    jassert (frequency != nullptr);
    level = treeState.getRawParameterValue (IDs::mainLevel);
    jassert (level != nullptr);

    lfoFrequency = treeState.getRawParameterValue (IDs::lfoFreq);
    jassert (lfoFrequency != nullptr);
    lfoLevel = treeState.getRawParameterValue (IDs::lfoLevel);
    jassert (lfoLevel != nullptr);

    vfoFrequency = treeState.getRawParameterValue (IDs::vfoFreq);
    jassert (vfoFrequency != nullptr);
    vfoLevel = treeState.getRawParameterValue (IDs::vfoLevel);
    jassert (vfoLevel != nullptr);

    treeState.addParameterListener (IDs::mainType, this);
    treeState.addParameterListener (IDs::lfoType, this);
    treeState.addParameterListener (IDs::vfoType, this);

    // MAGIC GUI: register an oscilloscope to display in the GUI.
    //            We keep a pointer to push samples into in processBlock().
    //            And we are only interested in channel 0
    oscilloscope = magicState.addPlotSource (IDs::oscilloscope, std::make_unique<foleys::MagicOscilloscope>(0));
}

SignalGeneratorAudioProcessor::~SignalGeneratorAudioProcessor()
{
}

//==============================================================================

void SignalGeneratorAudioProcessor::setOscillator (dsp::Oscillator<float>& osc, WaveType type)
{
    if (type == WaveType::Sine)
        osc.initialise ([](auto in) { return std::sin (in); });
    else if (type == WaveType::Triangle)
        osc.initialise ([](auto in) { return in / MathConstants<float>::pi; });
    else if (type == WaveType::Square)
        osc.initialise ([](auto in) { return in < 0 ? 1.0f : -1.0f; });
    else
        osc.initialise ([](auto) { return 0.0f; });
}

void SignalGeneratorAudioProcessor::parameterChanged (const String& param, float value)
{
    if (param == IDs::mainType)
        setOscillator (mainOSC, WaveType (roundToInt (value)));
    else if (param == IDs::lfoType)
        setOscillator (lfoOSC, WaveType (roundToInt (value)));
    else if (param == IDs::vfoType)
        setOscillator (vfoOSC, WaveType (roundToInt (value)));
}


void SignalGeneratorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    const auto numChannels = getTotalNumOutputChannels();

    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = uint32 (samplesPerBlock);
    spec.numChannels = uint32 (numChannels);

    mainOSC.prepare (spec);
    lfoOSC.prepare (spec);
    vfoOSC.prepare (spec);

    setOscillator (mainOSC, WaveType (roundToInt (treeState.getRawParameterValue (IDs::mainType)->load())));
    setOscillator (lfoOSC, WaveType (roundToInt (treeState.getRawParameterValue (IDs::lfoType)->load())));
    setOscillator (vfoOSC, WaveType (roundToInt (treeState.getRawParameterValue (IDs::vfoType)->load())));

    // MAGIC GUI: this will setup all internals like MagicPlotSources etc.
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
}

void SignalGeneratorAudioProcessor::releaseResources()
{
}

void SignalGeneratorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ignoreUnused (midiMessages);

    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    auto gain = Decibels::decibelsToGain (level->load());

    lfoOSC.setFrequency (*lfoFrequency);
    vfoOSC.setFrequency (*vfoFrequency);

    auto* channelData = buffer.getWritePointer (0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        mainOSC.setFrequency (frequency->load() * (1.0 + vfoOSC.processSample (0.0f) * vfoLevel->load()));
        channelData [i] = jlimit (-1.0f, 1.0f,
                                  mainOSC.processSample (0.0f) * gain * ( 1.0f - (lfoLevel->load() * lfoOSC.processSample (0.0f))));
    }

    for (int i=1; i < getTotalNumOutputChannels(); ++i)
        buffer.copyFrom (i, 0, buffer.getReadPointer (0), buffer.getNumSamples());

    // MAGIC GUI: push the samples to be displayed
    oscilloscope->pushSamples (buffer);
}

//==============================================================================
bool SignalGeneratorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SignalGeneratorAudioProcessor::createEditor()
{
    // MAGIC GUI: return a default Plugin GUI
    //            If you saved a GUI before, it is loaded here from the BinaryResources in the optional arguments
    // return new foleys::MagicPluginEditor (magicState);
    return new foleys::MagicPluginEditor (magicState, BinaryData::magic_xml, BinaryData::magic_xmlSize);
}

//==============================================================================
void SignalGeneratorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // MAGIC GUI: let the magicState conveniently handle save and restore the state.
    //            You don't need to use that, but it also takes care of restoring the last editor size
    magicState.getStateInformation (destData);
}

void SignalGeneratorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // MAGIC GUI: let the magicState conveniently handle save and restore the state.
    //            You don't need to use that, but it also takes care of restoring the last editor size
    magicState.setStateInformation (data, sizeInBytes, getActiveEditor());
}

//==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
bool SignalGeneratorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

//==============================================================================
const String SignalGeneratorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SignalGeneratorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SignalGeneratorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SignalGeneratorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SignalGeneratorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SignalGeneratorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SignalGeneratorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SignalGeneratorAudioProcessor::setCurrentProgram (int index)
{
    ignoreUnused (index);
}

const String SignalGeneratorAudioProcessor::getProgramName (int index)
{
    ignoreUnused (index);
    return {};
}

void SignalGeneratorAudioProcessor::changeProgramName (int index, const String& newName)
{
    ignoreUnused (index);
    ignoreUnused (newName);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SignalGeneratorAudioProcessor();
}
