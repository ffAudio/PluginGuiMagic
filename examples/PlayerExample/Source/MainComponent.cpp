/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    magicBuilder.registerJUCELookAndFeels();
    magicBuilder.registerJUCEFactories();

    magicBuilder.createGUI (*this);

    setSize (800, 600);

    magicState.addTrigger ("start", [&] { transport.start(); });
    magicState.addTrigger ("stop", [&] { transport.stop(); });
    magicState.addTrigger ("rewind", [&] { transport.setNextReadPosition (0); });

    outputAnalyser = magicState.createAndAddObject<foleys::MagicAnalyser>("analyser");
    magicState.addBackgroundProcessing (outputAnalyser);

    AudioFormatManager manager;
    manager.registerBasicFormats();
    auto reader = manager.createReaderFor (File::getSpecialLocation (File::userDesktopDirectory).getChildFile ("02 Heroes.mp3"));
    if (reader)
    {
        source.reset (new AudioFormatReaderSource (reader, true));
        transport.setSource (source.get());
    }

//    magicBuilder.setConfigTree (BinaryData::magic_xml, BinaryData::magic_xmlSize);

#if FOLEYS_SHOW_GUI_EDITOR_PALLETTE
    magicBuilder.attachToolboxToWindow (*this);
#endif

    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    outputAnalyser->prepareToPlay (sampleRate, samplesPerBlockExpected);
    transport.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    transport.getNextAudioBlock (bufferToFill);

    AudioBuffer<float> proxy (bufferToFill.buffer->getArrayOfWritePointers(),
                              bufferToFill.buffer->getNumChannels(),
                              bufferToFill.startSample,
                              bufferToFill.numSamples);
    outputAnalyser->pushSamples (proxy);
}

void MainComponent::releaseResources()
{
    transport.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void MainComponent::resized()
{
    magicBuilder.updateLayout();
}
