/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PresetListBox.h"

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    FoleysSynth::addADSRParameters (layout);
    FoleysSynth::addOvertoneParameters (layout);
    FoleysSynth::addGainParameters (layout);
    return layout;
}

//==============================================================================

FoleysSynthAudioProcessor::FoleysSynthAudioProcessor()
  : foleys::MagicProcessor (juce::AudioProcessor::BusesProperties()
                            .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
    treeState (*this, nullptr, ProjectInfo::projectName, createParameterLayout())
{
//    auto defaultGUI = magicState.createDefaultGUITree();
//    magicState.setGuiValueTree (defaultGUI);

    auto file = juce::File::getSpecialLocation (juce::File::currentApplicationFile)
        .getChildFile ("Contents")
        .getChildFile ("Resources")
        .getChildFile ("magic.xml");

    if (file.existsAsFile())
        magicState.setGuiValueTree (file);
    else
        magicState.setGuiValueTree (BinaryData::magic_xml, BinaryData::magic_xmlSize);

    // MAGIC GUI: add a meter at the output
    outputMeter  = magicState.createAndAddObject<foleys::MagicLevelSource>("output");
    oscilloscope = magicState.createAndAddObject<foleys::MagicOscilloscope>("waveform");

    analyser     = magicState.createAndAddObject<foleys::MagicAnalyser>("analyser");
    magicState.addBackgroundProcessing (analyser);

    presetList = magicState.createAndAddObject<PresetListBox>("presets");
    presetList->onSelectionChanged = [&](int number)
    {
        loadPresetInternal (number);
    };
    magicState.addTrigger ("save-preset", [this]
    {
        savePresetInternal();
    });

    magicState.setApplicationSettingsFile (juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                                           .getChildFile (ProjectInfo::companyName)
                                           .getChildFile (ProjectInfo::projectName + juce::String (".settings")));

    magicState.setPlayheadUpdateFrequency (30);

    FoleysSynth::FoleysSound::Ptr sound (new FoleysSynth::FoleysSound (treeState));
    synthesiser.addSound (sound);

    for (int i=0; i < 16; ++i)
        synthesiser.addVoice (new FoleysSynth::FoleysVoice (treeState));
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

bool FoleysSynthAudioProcessor::isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const
{
    // This synth only supports mono or stereo.
    return (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
            || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}

void FoleysSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // MAGIC GUI: send midi messages to the keyboard state and MidiLearn
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
void FoleysSynthAudioProcessor::savePresetInternal()
{
    presetNode = magicState.getSettings().getOrCreateChildWithName ("presets", nullptr);

    juce::ValueTree preset { "Preset" };
    preset.setProperty ("name", "Preset " + juce::String (presetNode.getNumChildren() + 1), nullptr);

    foleys::ParameterManager manager (*this);
    manager.saveParameterValues (preset);

    presetNode.appendChild (preset, nullptr);
}

void FoleysSynthAudioProcessor::loadPresetInternal(int index)
{
    presetNode = magicState.getSettings().getOrCreateChildWithName ("presets", nullptr);
    auto preset = presetNode.getChild (index);

    foleys::ParameterManager manager (*this);
    manager.loadParameterValues (preset);
}

//==============================================================================

double FoleysSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FoleysSynthAudioProcessor();
}
