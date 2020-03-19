/*
  ==============================================================================

    A simple equalizer using the JUCE dsp module

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class EqualizerExampleAudioProcessor  : public AudioProcessor,
                                        private AudioProcessorValueTreeState::Listener,
                                        private AsyncUpdater
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
        AttachedValue (AudioProcessorValueTreeState& state, std::atomic<ValueType>& value, const String& paramID, std::function<void()> changedLambda=nullptr);
        virtual ~AttachedValue();
        void parameterChanged (const String& parameterID, float newValue) override;
    private:
        void initialUpdate();

        AudioProcessorValueTreeState& state;
        std::atomic<ValueType>& value;
        String paramID;
        std::function<void()> onParameterChanged;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttachedValue)
    };

    //==============================================================================

    class FilterAttachment
    {
    public:
        FilterAttachment (AudioProcessorValueTreeState& state, FilterBand& filter, const String& prefix, const CriticalSection& lock);

        void setSampleRate (double sampleRate);
        bool isActive() const { return active; }

        std::function<void(const FilterAttachment&)> postFilterUpdate;

        juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
        double                                   sampleRate = 0.0;

    private:
        void updateFilter();

        AudioProcessorValueTreeState& state;
        FilterBand&                   filter;
        String                        prefix;
        const CriticalSection&        callbackLock;

        std::atomic<FilterType> type   { NoFilter };
        std::atomic<float>  frequency  { 1000.0f };
        std::atomic<float>  gain       { 0.0f };
        std::atomic<float>  quality    { 1.0f };
        std::atomic<bool>   active     { true };

        friend AttachedValue<float>;
        AttachedValue<FilterType> typeAttachment;
        AttachedValue<float> frequencyAttachment;
        AttachedValue<float> gainAttachment;
        AttachedValue<float> qualityAttachment;
        AttachedValue<bool>  activeAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterAttachment)
    };

    //==============================================================================
    void parameterChanged (const String& paramID, float newValue) override;
    void handleAsyncUpdate() override;

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

    std::atomic<float> gain { 1.0f };
    AttachedValue<float> gainAttachment;

    FilterAttachment attachment1 { treeState, filter.get<0>(), "Q1", getCallbackLock() };
    FilterAttachment attachment2 { treeState, filter.get<1>(), "Q2", getCallbackLock() };
    FilterAttachment attachment3 { treeState, filter.get<2>(), "Q3", getCallbackLock() };
    FilterAttachment attachment4 { treeState, filter.get<3>(), "Q4", getCallbackLock() };
    FilterAttachment attachment5 { treeState, filter.get<4>(), "Q5", getCallbackLock() };
    FilterAttachment attachment6 { treeState, filter.get<5>(), "Q6", getCallbackLock() };

    std::array<FilterAttachment*, 6> attachments
    { &attachment1, &attachment2, &attachment3, &attachment4, &attachment5, &attachment6 };

    foleys::MagicPlotSource*  inputAnalyser  = nullptr;
    foleys::MagicPlotSource*  outputAnalyser = nullptr;

    foleys::MagicFilterPlot*  plotSum = nullptr;
    foleys::MagicLevelSource* outputMeter = nullptr;

    // GUI MAGIC: define that as last member of your AudioProcessor
    foleys::MagicProcessorState magicState { *this, treeState };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerExampleAudioProcessor)
};
