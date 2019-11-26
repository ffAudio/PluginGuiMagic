/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

//==============================================================================
FoleysSynthAudioProcessor::FoleysSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // MAGIC GUI: add a meter at the output
    outputMeter = magicState.addLevelSource ("output", std::make_unique<foleys::MagicLevelSource>());
}

FoleysSynthAudioProcessor::~FoleysSynthAudioProcessor()
{
}

//==============================================================================
void FoleysSynthAudioProcessor::prepareToPlay (double sampleRate, int)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synthesiser.setCurrentPlaybackSampleRate (sampleRate);

    // MAGIC GUI: setup the output meter
    outputMeter->setupSource (getTotalNumOutputChannels(), sampleRate, 500, 200);
}

void FoleysSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FoleysSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FoleysSynthAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // MAGIC GUI: send midi messages to the keyboard state
    magicState.processMidiBuffer (midiMessages, buffer.getNumSamples());

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    synthesiser.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // MAGIC GUI: send the finished buffer to the level meter
    outputMeter->pushSamples (buffer);
}

//==============================================================================
bool FoleysSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* FoleysSynthAudioProcessor::createEditor()
{
    // MAGIC GUI: create the generated editor
    return new foleys::MagicPluginEditor (magicState, BinaryData::magic_xml, BinaryData::magic_xmlSize);
}

//==============================================================================
void FoleysSynthAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // MAGIC GUI: let the magicState conveniently handle save and restore the state.
    //            You don't need to use that, but it also takes care of restoring the last editor size
    magicState.getStateInformation (destData);
}

void FoleysSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // MAGIC GUI: let the magicState conveniently handle save and restore the state.
    //            You don't need to use that, but it also takes care of restoring the last editor size
    magicState.setStateInformation (data, sizeInBytes, getActiveEditor());
}

//==============================================================================
const String FoleysSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FoleysSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FoleysSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FoleysSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FoleysSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FoleysSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FoleysSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FoleysSynthAudioProcessor::setCurrentProgram (int)
{
}

const String FoleysSynthAudioProcessor::getProgramName (int)
{
    return {};
}

void FoleysSynthAudioProcessor::changeProgramName (int, const String&)
{
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FoleysSynthAudioProcessor();
}
