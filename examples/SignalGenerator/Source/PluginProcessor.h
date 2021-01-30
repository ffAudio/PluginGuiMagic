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
class SignalGeneratorAudioProcessor  : public  juce::AudioProcessor,
                                       private juce::AudioProcessorValueTreeState::Listener
{
public:
    enum WaveType
    {
        None = 0,
        Sine,
        Triangle,
        Square
    };

    //==============================================================================
    SignalGeneratorAudioProcessor();
    ~SignalGeneratorAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;
   #endif

    void parameterChanged (const juce::String& param, float value) override;

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

private:
    //==============================================================================

    void setOscillator (juce::dsp::Oscillator<float>& osc, WaveType type);

    std::atomic<float>* frequency  = nullptr;
    std::atomic<float>* level      = nullptr;

    std::atomic<float>* lfoFrequency  = nullptr;
    std::atomic<float>* lfoLevel      = nullptr;

    std::atomic<float>* vfoFrequency  = nullptr;
    std::atomic<float>* vfoLevel      = nullptr;

    juce::dsp::Oscillator<float> mainOSC;
    juce::dsp::Oscillator<float> lfoOSC;
    juce::dsp::Oscillator<float> vfoOSC;

    juce::AudioProcessorValueTreeState treeState;

    // MAGIC GUI: add this docking station for the GUI
    foleys::MagicProcessorState magicState { *this, treeState.state };

    // MAGIC GUI: this is a shorthand where the samples to display are fed to
    foleys::MagicPlotSource*    oscilloscope = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalGeneratorAudioProcessor)
};
