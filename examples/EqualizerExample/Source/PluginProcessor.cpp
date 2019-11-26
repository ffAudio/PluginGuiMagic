/*
  ==============================================================================

    A simple equalizer using the JUCE dsp module

  ==============================================================================
*/

#include "PluginProcessor.h"

//==============================================================================

namespace IDs
{
    static String paramOutput  { "output" };
    static String paramType    { "type" };
    static String paramFreq    { "freq" };
    static String paramGain    { "gain" };
    static String paramQuality { "quality" };
    static String paramActive  { "active" };
}

StringArray filterNames =
{
    NEEDS_TRANS ("No filter"),
    NEEDS_TRANS ("High pass"),
    NEEDS_TRANS ("1st order high pass"),
    NEEDS_TRANS ("Low shelf"),
    NEEDS_TRANS ("Band pass"),
    NEEDS_TRANS ("Notch"),
    NEEDS_TRANS ("Peak"),
    NEEDS_TRANS ("High shelf"),
    NEEDS_TRANS ("1st order low pass"),
    NEEDS_TRANS ("Low pass")
};

static float maxLevel = 24.0f;

std::unique_ptr<AudioProcessorParameterGroup> createParametersForFilter (const String& prefix,
                                                                         const String& name,
                                                                         EqualizerExampleAudioProcessor::FilterType type,
                                                                         float frequency,
                                                                         float gain    = 0.0f,
                                                                         float quality = 1.0f,
                                                                         bool  active  = true)
{
    auto typeParameter = std::make_unique<AudioParameterChoice> (prefix + IDs::paramType,
                                                                 name + ": " + TRANS ("Filter Type"),
                                                                 filterNames,
                                                                 type);

    auto actvParameter = std::make_unique<AudioParameterBool> (prefix + IDs::paramActive,
                                                               name + ": " + TRANS ("Active"),
                                                               active,
                                                               String(),
                                                               [](float value, int) {return value > 0.5f ? TRANS ("active") : TRANS ("bypassed");},
                                                               [](String text) {return text == TRANS ("active");});

    auto freqParameter = std::make_unique<AudioParameterFloat> (prefix + IDs::paramFreq,
                                                                name + ": " + TRANS ("Frequency"),
                                                                foleys::Conversions::makeLogarithmicRange<float>(20.0f, 20000.0f),
                                                                frequency,
                                                                String(),
                                                                AudioProcessorParameter::genericParameter,
                                                                [](float value, int) { return (value < 1000) ?
                                                                    String (value, 0) + " Hz" :
                                                                    String (value / 1000.0) + " kHz"; },
                                                                [](String text) { return text.endsWith(" kHz") ?
                                                                    text.getFloatValue() * 1000.0 :
                                                                    text.getFloatValue(); });

    auto qltyParameter = std::make_unique<AudioParameterFloat> (prefix + IDs::paramQuality,
                                                                name + ": " + TRANS ("Quality"),
                                                                NormalisableRange<float> {0.1f, 10.0f, 0.1f, std::log (0.5f) / std::log (0.9f / 9.9f)},
                                                                quality,
                                                                String(),
                                                                AudioProcessorParameter::genericParameter,
                                                                [](float value, int) { return String (value, 1); },
                                                                [](const String& text) { return text.getFloatValue(); });

    auto gainParameter = std::make_unique<AudioParameterFloat> (prefix + IDs::paramGain,
                                                                name + ": " + TRANS ("Gain"),
                                                                NormalisableRange<float> {-maxLevel, maxLevel, 0.1f},
                                                                gain,
                                                                String(),
                                                                AudioProcessorParameter::genericParameter,
                                                                [](float value, int) {return String (value, 1) + " dB";},
                                                                [](String text) {return text.getFloatValue();});

    auto group = std::make_unique<AudioProcessorParameterGroup> ("band" + prefix, name, "|",
                                                                 std::move (typeParameter),
                                                                 std::move (actvParameter),
                                                                 std::move (freqParameter),
                                                                 std::move (qltyParameter),
                                                                 std::move (gainParameter));

    return group;
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<AudioProcessorParameterGroup>> params;

    params.push_back (createParametersForFilter ("Q1", NEEDS_TRANS ("Q1"), EqualizerExampleAudioProcessor::HighPass,     40.0f));
    params.push_back (createParametersForFilter ("Q2", NEEDS_TRANS ("Q2"), EqualizerExampleAudioProcessor::LowShelf,    250.0f));
    params.push_back (createParametersForFilter ("Q3", NEEDS_TRANS ("Q3"), EqualizerExampleAudioProcessor::Peak,        500.0f));
    params.push_back (createParametersForFilter ("Q4", NEEDS_TRANS ("Q4"), EqualizerExampleAudioProcessor::Peak,       1000.0f));
    params.push_back (createParametersForFilter ("Q5", NEEDS_TRANS ("Q5"), EqualizerExampleAudioProcessor::HighShelf,  5000.0f));
    params.push_back (createParametersForFilter ("Q6", NEEDS_TRANS ("Q6"), EqualizerExampleAudioProcessor::LowPass,   12000.0f));

    auto param = std::make_unique<AudioParameterFloat> (IDs::paramOutput, TRANS ("Output"),
                                                        NormalisableRange<float> (0.0f, 2.0f, 0.01f), 1.0f,
                                                        String(),
                                                        AudioProcessorParameter::genericParameter,
                                                        [](float value, int) {return String (Decibels::gainToDecibels(value), 1) + " dB";},
                                                        [](String text) {return Decibels::decibelsToGain (text.getFloatValue());});

    auto group = std::make_unique<AudioProcessorParameterGroup> ("global", TRANS ("Globals"), "|", std::move (param));
    params.push_back (std::move (group));

    return { params.begin(), params.end() };
}

auto createPostUpdateLambda (foleys::MagicProcessorState& magicState, const String& plotID)
{
    return [plot = dynamic_cast<foleys::MagicFilterPlot*>(magicState.getPlotSource (plotID))] (const EqualizerExampleAudioProcessor::FilterAttachment& a)
    {
        if (plot != nullptr)
        {
            plot->setIIRCoefficients (a.coefficients, maxLevel);
            plot->setActive (a.isActive());
        }
    };
}

//==============================================================================
EqualizerExampleAudioProcessor::EqualizerExampleAudioProcessor()
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
    treeState (*this, nullptr, JucePlugin_Name, createParameterLayout()),
    gainAttachment (treeState, gain, IDs::paramOutput)
{
    // GUI MAGIC: add plots to be displayed in the GUI
    for (size_t i = 0; i < attachments.size(); ++i)
    {
        auto name = "plot" + String (i + 1);
        magicState.addPlotSource (name, std::make_unique<foleys::MagicFilterPlot>());
        attachments.at (i)->postFilterUpdate = createPostUpdateLambda (magicState, name);
    }

    plotSum = dynamic_cast<foleys::MagicFilterPlot*>(magicState.addPlotSource ("plotSum", std::make_unique<foleys::MagicFilterPlot>()));

    // GUI MAGIC: add analyser plots
    inputAnalyser  = magicState.addPlotSource ("input", std::make_unique<foleys::MagicAnalyser>());
    outputAnalyser = magicState.addPlotSource ("output", std::make_unique<foleys::MagicAnalyser>());

    // MAGIC GUI: add a meter at the output
    outputMeter = magicState.addLevelSource ("output", std::make_unique<foleys::MagicLevelSource>());

    for (auto* parameter : getParameters())
        if (auto* p = dynamic_cast<AudioProcessorParameterWithID*>(parameter))
            treeState.addParameterListener (p->paramID, this);
}

EqualizerExampleAudioProcessor::~EqualizerExampleAudioProcessor()
{
    for (auto* parameter : getParameters())
        if (auto* p = dynamic_cast<AudioProcessorParameterWithID*>(parameter))
            treeState.removeParameterListener (p->paramID, this);
}

//==============================================================================
void EqualizerExampleAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto numChannels = getTotalNumOutputChannels();

    // GUI MAGIC: call this to set up the visualisers
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
    outputMeter->setupSource (getTotalNumOutputChannels(), sampleRate, 500, 200);

    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = uint32 (samplesPerBlock);
    spec.numChannels = uint32 (numChannels);

    filter.get<6>().setGainLinear (*treeState.getRawParameterValue (IDs::paramOutput));

    for (auto* a : attachments)
        a->setSampleRate (sampleRate);

    filter.prepare (spec);
}

void EqualizerExampleAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EqualizerExampleAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void EqualizerExampleAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    ignoreUnused (midiMessages);

    filter.setBypassed<0>(attachment1.isActive() == false);
    filter.setBypassed<1>(attachment2.isActive() == false);
    filter.setBypassed<2>(attachment3.isActive() == false);
    filter.setBypassed<3>(attachment4.isActive() == false);
    filter.setBypassed<4>(attachment5.isActive() == false);
    filter.setBypassed<5>(attachment6.isActive() == false);

    filter.get<6>().setGainLinear (gain);

    // GUI MAGIC: measure before processing
    inputAnalyser->pushSamples (buffer);

    dsp::AudioBlock<float>              ioBuffer (buffer);
    dsp::ProcessContextReplacing<float> context  (ioBuffer);
    filter.process (context);

    // GUI MAGIC: measure after processing
    outputAnalyser->pushSamples (buffer);
    outputMeter->pushSamples (buffer);
}

//==============================================================================

EqualizerExampleAudioProcessor::FilterAttachment::FilterAttachment (AudioProcessorValueTreeState& stateToUse, FilterBand& filterToControl, const String& prefixToUse, const CriticalSection& lock)
  : state               (stateToUse),
    filter              (filterToControl),
    prefix              (prefixToUse),
    callbackLock        (lock),
    typeAttachment      (state, type,      prefix + IDs::paramType,     [&]{ updateFilter(); }),
    frequencyAttachment (state, frequency, prefix + IDs::paramFreq,     [&]{ updateFilter(); }),
    gainAttachment      (state, gain,      prefix + IDs::paramGain,     [&]{ updateFilter(); }),
    qualityAttachment   (state, quality,   prefix + IDs::paramQuality,  [&]{ updateFilter(); }),
    activeAttachment    (state, active,    prefix + IDs::paramActive,   [&]
    { if (postFilterUpdate)
        postFilterUpdate (*this);
    })
{
    updateFilter();
}

void EqualizerExampleAudioProcessor::FilterAttachment::updateFilter()
{
    if (sampleRate < 20.0)
        return;

    switch (type)
    {
        case NoFilter:    coefficients = new dsp::IIR::Coefficients<float> (1, 0, 1, 0); break;
        case LowPass:     coefficients = dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, frequency, quality); break;
        case LowPass1st:  coefficients = dsp::IIR::Coefficients<float>::makeFirstOrderLowPass (sampleRate, frequency); break;
        case LowShelf:    coefficients = dsp::IIR::Coefficients<float>::makeLowShelf (sampleRate, frequency, quality, Decibels::decibelsToGain (gain)); break;
        case BandPass:    coefficients = dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, frequency, quality); break;
        case Notch:       coefficients = dsp::IIR::Coefficients<float>::makeNotch (sampleRate, frequency, quality); break;
        case Peak:        coefficients = dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, frequency, quality, Decibels::decibelsToGain (gain)); break;
        case HighShelf:   coefficients = dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, frequency, quality, Decibels::decibelsToGain (gain)); break;
        case HighPass1st: coefficients = dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (sampleRate, frequency); break;
        case HighPass:    coefficients = dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, frequency, quality); break;
        default:          return;
    }

    {
        ScopedLock processLock (callbackLock);
        *filter.state = *coefficients;
    }

    if (postFilterUpdate)
        postFilterUpdate (*this);
}

void EqualizerExampleAudioProcessor::FilterAttachment::setSampleRate (double sampleRateToUse)
{
    sampleRate = sampleRateToUse;
    updateFilter();
}

//==============================================================================

template<typename ValueType>
EqualizerExampleAudioProcessor::AttachedValue<ValueType>::AttachedValue (AudioProcessorValueTreeState& stateToUse, ValueType& valueToUse, const String& paramToUse, std::function<void()> changedLambda)
  : state (stateToUse),
    value (valueToUse),
    paramID (paramToUse),
    onParameterChanged (changedLambda)
{
    // Oh uh, tried to attach to a non existing parameter
    jassert (state.getParameter (paramID) != nullptr);

    initialUpdate();
    state.addParameterListener (paramID, this);
}

template<typename ValueType>
EqualizerExampleAudioProcessor::AttachedValue<ValueType>::~AttachedValue()
{
    state.removeParameterListener (paramID, this);
}

template<typename ValueType>
void EqualizerExampleAudioProcessor::AttachedValue<ValueType>::initialUpdate()
{
    value = ValueType (*state.getRawParameterValue (paramID));
}

template<>
void EqualizerExampleAudioProcessor::AttachedValue<EqualizerExampleAudioProcessor::FilterType>::initialUpdate()
{
    value = EqualizerExampleAudioProcessor::FilterType (juce::roundToInt (*state.getRawParameterValue (paramID)));
}

template<typename ValueType>
void EqualizerExampleAudioProcessor::AttachedValue<ValueType>::parameterChanged (const String&, float newValue)
{
    value = newValue;
    if (onParameterChanged)
        onParameterChanged();
}

template<>
void EqualizerExampleAudioProcessor::AttachedValue<bool>::parameterChanged (const String&, float newValue)
{
    value = (newValue > 0.5f);
    if (onParameterChanged)
        onParameterChanged();
}

template<>
void EqualizerExampleAudioProcessor::AttachedValue<EqualizerExampleAudioProcessor::FilterType>::parameterChanged (const String&, float newValue)
{
    value = EqualizerExampleAudioProcessor::FilterType (roundToInt (newValue));
    if (onParameterChanged)
        onParameterChanged();
}

//==============================================================================
void EqualizerExampleAudioProcessor::parameterChanged (const String&, float)
{
    triggerAsyncUpdate();
}

void EqualizerExampleAudioProcessor::handleAsyncUpdate()
{
    std::vector<juce::dsp::IIR::Coefficients<float>::Ptr> coefficients;
    for (auto* a : attachments)
        if (a->isActive())
            coefficients.push_back (a->coefficients);

    plotSum->setIIRCoefficients (gain, coefficients, maxLevel);
}

//==============================================================================
bool EqualizerExampleAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* EqualizerExampleAudioProcessor::createEditor()
{
    // MAGIC GUI: create the generated editor
    return new foleys::MagicPluginEditor (magicState, BinaryData::magic_xml, BinaryData::magic_xmlSize);
}

//==============================================================================
void EqualizerExampleAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // MAGIC GUI: let the magicState conveniently handle save and restore the state.
    //            You don't need to use that, but it also takes care of restoring the last editor size
    magicState.getStateInformation (destData);
}

void EqualizerExampleAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // MAGIC GUI: let the magicState conveniently handle save and restore the state.
    //            You don't need to use that, but it also takes care of restoring the last editor size
    magicState.setStateInformation (data, sizeInBytes, getActiveEditor());
}

//==============================================================================
const String EqualizerExampleAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EqualizerExampleAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EqualizerExampleAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EqualizerExampleAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EqualizerExampleAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EqualizerExampleAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EqualizerExampleAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EqualizerExampleAudioProcessor::setCurrentProgram (int)
{
}

const String EqualizerExampleAudioProcessor::getProgramName (int)
{
    return {};
}

void EqualizerExampleAudioProcessor::changeProgramName (int, const String&)
{
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EqualizerExampleAudioProcessor();
}
