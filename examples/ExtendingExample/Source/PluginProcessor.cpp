/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

// Some nice example drawing
class Lissajour   : public Component,
                    private Timer
{
public:
    Lissajour()
    {
        startTimerHz (30);
    }

    void paint (Graphics& g) override
    {
        const float radius = std::min (getWidth(), getHeight()) * 0.4f;
        const auto  centre = getLocalBounds().getCentre().toFloat();
        const float factor = 3.0f;

        g.fillAll (Colours::black);
        g.setColour (Colours::green);
        Path p;
        p.startNewSubPath (centre + Point<float>(0, std::sin (phase)) * radius);
        for (float i = 0.1; i <= MathConstants<float>::twoPi; i += 0.01)
        {
            auto x = i;
            auto y = i * factor + phase;
            p.lineTo (centre + Point<float>(std::sin (x),
                                            std::sin (std::fmod (y, MathConstants<float>::twoPi))) * radius);
        }

        g.strokePath (p, PathStrokeType (2.0f));
    }

private:
    void timerCallback() override
    {
        phase += 0.1;
        if (phase >= MathConstants<float>::twoPi)
            phase -= MathConstants<float>::twoPi;

        repaint();
    }

    float phase = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Lissajour)
};

// Silly example class that displays some members from your specific processor
class StatisticsComponent : public Component,
                            private Timer
{
public:
    StatisticsComponent (ExtendingExampleAudioProcessor& processorToUse)
      : processor (processorToUse)
    {
        startTimerHz (30);
    }

    void paint (Graphics& g) override
    {
        // silly example: display current actual buffer size
        auto sampleRate = processor.statisticsSampleRate.load();
        auto samples    = processor.statisticsSamplesPerBlock.load();

        g.fillAll (Colours::darkgrey);
        g.setColour (Colours::white);
        g.drawFittedText ("Samplerate: " + String (sampleRate) + "\n" +
                          "Buffersize: " + String (samples), getLocalBounds(), Justification::centredLeft, 3);
    }

private:
    void timerCallback() override
    {
        repaint();
    }

    ExtendingExampleAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatisticsComponent)
};

//==============================================================================
ExtendingExampleAudioProcessor::ExtendingExampleAudioProcessor()
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
}

ExtendingExampleAudioProcessor::~ExtendingExampleAudioProcessor()
{
}


//==============================================================================
void ExtendingExampleAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    statisticsSampleRate.store (sampleRate);
}

void ExtendingExampleAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ExtendingExampleAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ExtendingExampleAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // silly example: display current actual buffer size
    statisticsSamplesPerBlock.store (buffer.getNumSamples());
}

//==============================================================================
bool ExtendingExampleAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ExtendingExampleAudioProcessor::createEditor()
{
    // MAGIC GUI: we create our custom builder instance here, that will be available for all factories we add
    auto builder = std::make_unique<foleys::MagicGUIBuilder<ExtendingExampleAudioProcessor>>(*this, &magicState);
    builder->registerJUCEFactories();

    builder->registerFactory ("Lissajour", [](const ValueTree&, ExtendingExampleAudioProcessor&)
                              {
                                  return std::make_unique<Lissajour>();
                              });

    builder->registerFactory ("Statistics", [](const ValueTree&, ExtendingExampleAudioProcessor& p)
                              {
                                  return std::make_unique<StatisticsComponent>(p);
                              });

    return new foleys::MagicPluginEditor (magicState, BinaryData::magic_xml, BinaryData::magic_xmlSize, std::move (builder));
}

//==============================================================================
void ExtendingExampleAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    magicState.getStateInformation (destData);
}

void ExtendingExampleAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    magicState.setStateInformation (data, sizeInBytes, getActiveEditor());
}

//==============================================================================
const String ExtendingExampleAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ExtendingExampleAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ExtendingExampleAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ExtendingExampleAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ExtendingExampleAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ExtendingExampleAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int ExtendingExampleAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ExtendingExampleAudioProcessor::setCurrentProgram (int index)
{
}

const String ExtendingExampleAudioProcessor::getProgramName (int index)
{
    return {};
}

void ExtendingExampleAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ExtendingExampleAudioProcessor();
}
