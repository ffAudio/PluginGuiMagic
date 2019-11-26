/*
  ==============================================================================

    FoleysSynth.h
    Created: 23 Nov 2019 8:08:35pm
    Author:  Daniel Walz

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class FoleysSynth : public Synthesiser
{
public:
    FoleysSynth()=default;

    class FoleysSound : public SynthesiserSound
    {
    public:
        bool appliesToNote (int) override { return true; }
        bool appliesToChannel (int) override { return true; }
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FoleysSound)
    };

    class FoleysVoice : public SynthesiserVoice
    {
    public:
        bool canPlaySound (SynthesiserSound *) override { return true; }

        void startNote (int midiNoteNumber,
                        float velocity,
                        SynthesiserSound* sound,
                        int currentPitchWheelPosition) override;

        void stopNote (float velocity, bool allowTailOff) override;

        void pitchWheelMoved (int newPitchWheelValue) override;

        void controllerMoved (int controllerNumber, int newControllerValue) override;

        void renderNextBlock (AudioBuffer<float>& outputBuffer,
                              int startSample,
                              int numSamples) override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FoleysVoice)
    };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FoleysSynth)
};
