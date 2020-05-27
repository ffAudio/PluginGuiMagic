/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PresetListBox.h"

//==============================================================================

AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;
    FoleysSynth::addADSRParameters (layout);
    FoleysSynth::addOvertoneParameters (layout);
    FoleysSynth::addGainParameters (layout);
    return layout;
}

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
                       ),
#else
    :
#endif
    treeState (*this, nullptr, ProjectInfo::projectName, createParameterLayout())
{
    // MAGIC GUI: add a meter at the output
    outputMeter  = magicState.createAndAddObject<foleys::MagicLevelSource>("output");
    oscilloscope = magicState.createAndAddObject<foleys::MagicOscilloscope>("waveform");

    analyser     = magicState.createAndAddObject<foleys::MagicAnalyser>("analyser");
    magicState.addBackgroundProcessing (analyser);

    presetNode = magicState.getValueTreeState().state.getOrCreateChildWithName ("presets", nullptr);

    presetList = magicState.createAndAddObject<PresetListBox>("presets", presetNode);
    presetList->onSelectionChanged = [&](int number)
    {
        loadPresetInternal (number);
    };
    magicState.addTrigger ("save-preset", [this]
    {
        savePresetInternal();
    });

    magicState.setPlayheadUpdateFrequency (30);

    FoleysSynth::FoleysSound::Ptr sound (new FoleysSynth::FoleysSound (treeState));
    synthesiser.addSound (sound);

    for (int i=0; i < 16; ++i)
        synthesiser.addVoice (new FoleysSynth::FoleysVoice (treeState));
}

FoleysSynthAudioProcessor::~FoleysSynthAudioProcessor()
{
}

//==============================================================================
void FoleysSynthAudioProcessor::prepareToPlay (double sampleRate, int blockSize)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synthesiser.setCurrentPlaybackSampleRate (sampleRate);

    // MAGIC GUI: setup the output meter
    outputMeter->setupSource (getTotalNumOutputChannels(), sampleRate, 500, 200);
    oscilloscope->prepareToPlay (sampleRate, blockSize);
    analyser->prepareToPlay (sampleRate, blockSize);
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
    magicState.processMidiBuffer (midiMessages, buffer.getNumSamples(), true);

    // MAGIC GUI: send playhead information to the GUI
    magicState.updatePlayheadInformation (getPlayHead());

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    synthesiser.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    for (int i = 1; i < buffer.getNumChannels(); ++i)
        buffer.copyFrom (i, 0, buffer.getReadPointer (0), buffer.getNumSamples());

    // MAGIC GUI: send the finished buffer to the level meter
    outputMeter->pushSamples (buffer);
    oscilloscope->pushSamples (buffer);
    analyser->pushSamples (buffer);
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
void FoleysSynthAudioProcessor::savePresetInternal()
{
    ValueTree preset { "Preset" };
    preset.setProperty ("name", "Preset " + String (presetNode.getNumChildren() + 1), nullptr);
    for (const auto& p : magicState.getValueTreeState().state)
        if (p.getType().toString() == "PARAM")
            preset.appendChild (p.createCopy(), nullptr);

    DBG (preset.toXmlString());
    presetNode.appendChild (preset, nullptr);
}

void FoleysSynthAudioProcessor::loadPresetInternal(int index)
{
    auto preset = presetNode.getChild (index);
    for (const auto& p : preset)
    {
        if (p.hasType ("PARAM"))
        {
            auto id = p.getProperty ("id", "unknownID").toString();
            if (auto* parameter = dynamic_cast<RangedAudioParameter*>(magicState.getValueTreeState().getParameter (id)))
                parameter->setValueNotifyingHost (parameter->convertTo0to1 (p.getProperty ("value")));
        }
    }
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

    presetNode = magicState.getValueTreeState().state.getOrCreateChildWithName ("presets", nullptr);
    presetList->setPresetsNode (presetNode);
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

void FoleysSynthAudioProcessor::setCurrentProgram (int index)
{
    loadPresetInternal (index);
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
