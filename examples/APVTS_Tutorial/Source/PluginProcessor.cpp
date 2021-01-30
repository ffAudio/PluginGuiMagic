/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/


#include "../JuceLibraryCode/JuceHeader.h"

class TutorialProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TutorialProcessor()
    : parameters (*this, nullptr, juce::Identifier ("APVTSTutorial"),
    {
        std::make_unique<juce::AudioParameterFloat> ("gain",            // parameterID
                                                     "Gain",            // parameter name
                                                     0.0f,              // minimum value
                                                     1.0f,              // maximum value
                                                     0.5f),             // default value
        std::make_unique<juce::AudioParameterBool> ("invertPhase",      // parameterID
                                                    "Invert Phase",     // parameter name
                                                    false)              // default value
    })
    {
        phaseParameter = parameters.getRawParameterValue ("invertPhase");
        gainParameter  = parameters.getRawParameterValue ("gain");
    }

    //==============================================================================
    void prepareToPlay (double, int) override
    {
        auto phase = *phaseParameter < 0.5f ? 1.0f : -1.0f;
        previousGain = *gainParameter * phase;
    }

    void releaseResources() override {}

    void processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override
    {
        auto phase = *phaseParameter < 0.5f ? 1.0f : -1.0f;
        auto currentGain = *gainParameter * phase;

        if (currentGain == previousGain)
        {
            buffer.applyGain (currentGain);
        }
        else
        {
            buffer.applyGainRamp (0, buffer.getNumSamples(), previousGain, currentGain);
            previousGain = currentGain;
        }
    }

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override
    {
        // return a MagicPluginEditor. It will be pre-filled with a tree, that you can edit
        return new foleys::MagicPluginEditor (magicState);
    }

    bool hasEditor() const override                        { return true; }

    //==============================================================================
    const juce::String getName() const override            { return "APVTS Tutorial"; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0; }

    //==============================================================================
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const juce::String getProgramName (int) override       { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override
    {
        auto state = parameters.copyState();
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

        if (xmlState.get() != nullptr)
            if (xmlState->hasTagName (parameters.state.getType()))
                parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
    }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
    float previousGain; // [1]

    std::atomic<float>* phaseParameter = nullptr;
    std::atomic<float>* gainParameter  = nullptr;

    // Add an instance of MagicProcessorState to connect a MagicGuiEditor to it
    foleys::MagicProcessorState magicState { *this, parameters.state };
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TutorialProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TutorialProcessor();
}
