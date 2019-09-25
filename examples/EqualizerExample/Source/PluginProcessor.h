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

    using FilterBand = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>>;
    using Gain       = dsp::Gain<float>;

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

    template<typename ValueType>
    class AttachedValue : private AudioProcessorValueTreeState::Listener
    {
    public:
        AttachedValue (AudioProcessorValueTreeState& state, ValueType& value, const String& paramID, std::function<void()> changedLambda=nullptr);
        virtual ~AttachedValue();
        void parameterChanged (const String& parameterID, float newValue) override;
    private:
        AudioProcessorValueTreeState& state;
        ValueType& value;
        String paramID;
        std::function<void()> onParameterChanged;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttachedValue)
    };

    //==============================================================================

    class FilterAttachment : private AudioProcessorValueTreeState::Listener
    {
    public:
        FilterAttachment (AudioProcessorValueTreeState& state, FilterBand& filter, const String& prefix, const CriticalSection& lock);
        virtual ~FilterAttachment();
        void parameterChanged (const String& parameterID, float newValue) override;
        void setSampleRate (double sampleRate);
        bool isActive() const { return active; }

    private:
        void updateFilter();

        AudioProcessorValueTreeState& state;
        FilterBand&                   filter;
        String                        prefix;
        const CriticalSection&        callbackLock;

        FilterType type   = NoFilter;
        float  frequency  = 1000.0f;
        float  gain       = 0.0f;
        float  quality    = 1.0f;
        bool   active     = true;
        double sampleRate = 0.0;

        friend AttachedValue<float>;
        AttachedValue<float> frequencyAttachment;
        AttachedValue<float> gainAttachment;
        AttachedValue<float> qualityAttachment;
        AttachedValue<bool>  activeAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterAttachment)
    };

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

    dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, Gain> filter;

    float gain = 1.0f;
    AttachedValue<float> gainAttachment;

    FilterAttachment attachment1 { treeState, filter.get<0>(), "Q1", getCallbackLock() };
    FilterAttachment attachment2 { treeState, filter.get<1>(), "Q2", getCallbackLock() };
    FilterAttachment attachment3 { treeState, filter.get<2>(), "Q3", getCallbackLock() };
    FilterAttachment attachment4 { treeState, filter.get<3>(), "Q4", getCallbackLock() };
    FilterAttachment attachment5 { treeState, filter.get<4>(), "Q5", getCallbackLock() };
    FilterAttachment attachment6 { treeState, filter.get<5>(), "Q6", getCallbackLock() };

    // define that as last member of your AudioProcessor
    foleys::MagicProcessorState magicState { *this, treeState };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerExampleAudioProcessor)
};
