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
class SignalGeneratorAudioProcessor  : public AudioProcessor,
                                       private AudioProcessorValueTreeState::Listener
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
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void parameterChanged (const String& param, float value) override;

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================

    void setOscillator (dsp::Oscillator<float>& osc, WaveType type);

    float* frequency  = nullptr;
    float* level      = nullptr;

    float* lfoFrequency  = nullptr;
    float* lfoLevel      = nullptr;

    float* vfoFrequency  = nullptr;
    float* vfoLevel      = nullptr;

    dsp::Oscillator<float> mainOSC;
    dsp::Oscillator<float> lfoOSC;
    dsp::Oscillator<float> vfoOSC;

    AudioProcessorValueTreeState treeState;

    // MAGIC GUI: add this docking station for the GUI
    foleys::MagicProcessorState magicState { *this, treeState };

    // MAGIC GUI: this is a shorthand where the samples to display are fed to
    foleys::MagicPlotSource*    oscilloscope = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalGeneratorAudioProcessor)
};
