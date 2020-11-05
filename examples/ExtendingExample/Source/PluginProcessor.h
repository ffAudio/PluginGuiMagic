/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
*/
class ExtendingExampleAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ExtendingExampleAudioProcessor();
    ~ExtendingExampleAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    std::atomic<double> statisticsSampleRate;
    std::atomic<int>    statisticsSamplesPerBlock;

private:
    //==============================================================================

    juce::AudioProcessorValueTreeState treeState { *this, nullptr, "PARAMETERS", juce::AudioProcessorValueTreeState::ParameterLayout() };

    // MAGIC GUI: add a MagicPluginState as connection
    foleys::MagicProcessorState magicState { *this, treeState };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExtendingExampleAudioProcessor)
};
