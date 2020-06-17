/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    manager.registerBasicFormats();

    magicBuilder.registerJUCELookAndFeels();
    magicBuilder.registerJUCEFactories();


    magicState.addTrigger ("start", [&] { transport.start(); });
    magicState.addTrigger ("stop", [&] { transport.stop(); });
    magicState.addTrigger ("rewind", [&] { transport.setNextReadPosition (0); });

    magicState.addTrigger ("open", [&]
    {
        auto dialog = std::make_unique<foleys::FileBrowserDialog>(NEEDS_TRANS ("Cancel"), NEEDS_TRANS ("Load"),
                                                                  juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                                                  lastFolder,
                                                                  std::make_unique<WildcardFileFilter>(manager.getWildcardForAllFormats(), "*", NEEDS_TRANS ("Audio Files")));
        dialog->setAcceptFunction ([&, dlg=dialog.get()]
        {
            loadFile (dlg->getFile());
            magicBuilder.closeOverlayDialog();
        });
        dialog->setCancelFunction ([&]
        {
            magicBuilder.closeOverlayDialog();
        });

        magicBuilder.showOverlayDialog (std::move (dialog));

    });

    outputLevel    = magicState.createAndAddObject<foleys::MagicLevelSource>("level");
    outputAnalyser = magicState.createAndAddObject<foleys::MagicAnalyser>("analyser");
    magicState.addBackgroundProcessing (outputAnalyser);

    gainValue.referTo (magicState.getPropertyAsValue ("gain"));
    gainValue.addListener (this);
    gainValue.setValue (1.0);

    magicBuilder.setConfigTree (BinaryData::magic_xml, BinaryData::magic_xmlSize);
    magicBuilder.createGUI (*this);
    updatePositionSlider();

    setSize (800, 600);

    transport.addChangeListener (this);

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

void MainComponent::loadFile (const File& file)
{
    lastFolder = file.getParentDirectory();

    auto reader = manager.createReaderFor (file);
    if (reader)
    {
        transport.setSource (nullptr);
        source.reset (new AudioFormatReaderSource (reader, true));
        transport.setSource (source.get());
    }
}

void MainComponent::valueChanged (Value& value)
{
    if (value == gainValue)
        transport.setGain (gainValue.getValue());
}

void MainComponent::sliderValueChanged (Slider* slider)
{
    transport.setPosition (slider->getValue());
}

void MainComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updatePositionSlider();
}

void MainComponent::updatePositionSlider()
{
    if (auto* item = magicBuilder.findGuiItemWithId ("position"))
    {
        if (auto* position = dynamic_cast<Slider*>(item->getWrappedComponent()))
        {
            if (transport.getLengthInSeconds() > 0)
                position->setRange (0, transport.getLengthInSeconds());

            position->textFromValueFunction = [](float value){
                return String (int (value / 3600)) + ":" +
                       String (int (value / 60) % 60).paddedLeft ('0', 2) + ":" +
                       String (int(value) % 60).paddedLeft ('0', 2); };
            position->addListener (this);

            if (transport.isPlaying())
                startTimerHz (4);
            else
                stopTimer();
        }
    }
}

void MainComponent::timerCallback()
{
    if (auto* item = magicBuilder.findGuiItemWithId ("position"))
    {
        if (auto* position = dynamic_cast<Slider*>(item->getWrappedComponent()))
            position->setValue (transport.getCurrentPosition(), dontSendNotification);
    }
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    outputLevel->setNumChannels (2);
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
    outputLevel->pushSamples (proxy);
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
