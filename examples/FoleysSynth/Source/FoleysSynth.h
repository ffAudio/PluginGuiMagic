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
        FoleysSound() = default;
        bool appliesToNote (int) override { return true; }
        bool appliesToChannel (int) override { return true; }

        ADSR::Parameters getADSR();

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FoleysSound)
    };

    class FoleysVoice : public SynthesiserVoice
    {
    public:
        FoleysVoice();

        bool canPlaySound (SynthesiserSound *) override;

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

        void setCurrentPlaybackSampleRate (double newRate) override;

    private:
        double getDetuneFromPitchWheel (int wheelValue, double semitonesToDetune) const;
        double getFrequencyForNote (int noteNumber, double detune, double concertPitch = 440.0) const;

        dsp::Oscillator<float> osc;
        int                    midiNumber = -1;
        int                    maxPitchWheelSemitones = 12;
        AudioBuffer<float>     buffer;
        ADSR                   adsr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FoleysVoice)
    };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FoleysSynth)
};
