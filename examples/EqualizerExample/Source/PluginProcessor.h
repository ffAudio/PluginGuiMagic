/*
  ==============================================================================

    A simple equalizer using the JUCE dsp module

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class EqualizerExampleAudioProcessor  : public AudioProcessor
{
public:

    enum FilterType
    {
        NoFilter = 0,
        HighPass,
        HighPass1st,
        LowShelf,
        BandPass,
        Notch,
        Peak,
        HighShelf,
        LowPass1st,
        LowPass,
        LastFilterID
    };

    //==============================================================================
    EqualizerExampleAudioProcessor();
    ~EqualizerExampleAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

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

    static StringArray filterNames;

private:
    //==============================================================================

    AudioProcessorValueTreeState treeState { *this, nullptr };

    using FilterBand = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>>;
    using Gain       = dsp::Gain<float>;
    dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, Gain> filter;

    // define that as last member of your AudioProcessor
    foleys::MagicProcessorState magicState { *this, treeState };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerExampleAudioProcessor)
};
