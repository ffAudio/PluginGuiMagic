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
    static int  numOscillators;

    static void addADSRParameters (AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addOvertoneParameters (AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addGainParameters (AudioProcessorValueTreeState::ParameterLayout& layout);

    FoleysSynth() = default;

    class FoleysSound : public SynthesiserSound
    {
    public:
        FoleysSound (AudioProcessorValueTreeState& state);
        bool appliesToNote (int) override { return true; }
        bool appliesToChannel (int) override { return true; }

        ADSR::Parameters getADSR();

    private:
        AudioProcessorValueTreeState& state;
        AudioParameterFloat* attack  = nullptr;
        AudioParameterFloat* decay   = nullptr;
        AudioParameterFloat* sustain = nullptr;
        AudioParameterFloat* release = nullptr;
        AudioParameterFloat* gain    = nullptr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FoleysSound)
    };

    class FoleysVoice : public SynthesiserVoice
    {
    public:
        FoleysVoice (AudioProcessorValueTreeState& state);

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

        class BaseOscillator
        {
        public:
            BaseOscillator() = default;

            dsp::ProcessorChain<dsp::Oscillator<float>, dsp::Gain<float>> osc;
            AudioParameterFloat* gain   = nullptr;
            AudioParameterFloat* detune = nullptr;
            double multiplier = 1.0;

        private:
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BaseOscillator)
        };

        double getDetuneFromPitchWheel (int wheelValue) const;
        double getFrequencyForNote (int noteNumber, double detune, double concertPitch = 440.0) const;

        void updateFrequency (BaseOscillator& oscillator);

        std::vector<std::unique_ptr<BaseOscillator>> oscillators;

        int                    midiNumber = -1;
        double                 pitchWheelValue = 0.0;
        int                    maxPitchWheelSemitones = 12;
        const int              internalBufferSize = 64;
        AudioBuffer<float>     oscillatorBuffer;
        AudioBuffer<float>     voiceBuffer;
        ADSR                   adsr;
        AudioParameterFloat*   gainParameter = nullptr;
        double                 lastGain = 0.0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FoleysVoice)
    };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FoleysSynth)
};
