/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public AudioAppComponent,
                        public Slider::Listener,
                        private Value::Listener,
                        private ChangeListener,
                        private Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    void loadFile (const File& file);

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:

    void valueChanged (Value&) override;
    void sliderValueChanged (Slider*) override;
    void changeListenerCallback (ChangeBroadcaster*) override;
    void timerCallback() override;

    void updatePositionSlider();

    //==============================================================================
    // Your private member variables go here...

    AudioFormatManager manager;
    File               lastFolder = File::getSpecialLocation (File::userMusicDirectory);

    std::unique_ptr<AudioFormatReaderSource> source;
    AudioTransportSource     transport;
    Value                    gainValue { 1.0f };

    foleys::MagicGUIState    magicState;
    foleys::MagicGUIBuilder  magicBuilder { magicState };

    foleys::MagicPlotSource*  outputAnalyser { nullptr };
    foleys::MagicLevelSource* outputLevel    { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
