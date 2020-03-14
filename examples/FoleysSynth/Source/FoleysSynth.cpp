/*
  ==============================================================================

    FoleysSynth.cpp
    Created: 23 Nov 2019 8:08:35pm
    Author:  Daniel Walz

  ==============================================================================
*/

#include "FoleysSynth.h"

FoleysSynth::FoleysVoice::FoleysVoice()
{
    osc.initialise ([](auto arg){return std::sin (arg);}, 512);

    buffer.setSize (1, 128);
}

bool FoleysSynth::FoleysVoice::canPlaySound (SynthesiserSound* sound)
{
    return dynamic_cast<FoleysSound*>(sound) != nullptr;
}

void FoleysSynth::FoleysVoice::startNote (int midiNoteNumber,
                                          float velocity,
                                          SynthesiserSound* sound,
                                          int currentPitchWheelPosition)
{
    if (auto* foleysSound = dynamic_cast<FoleysSound*>(sound))
    {
        adsr.setParameters (foleysSound->getADSR());
    }

    osc.setFrequency (getFrequencyForNote (midiNoteNumber, getDetuneFromPitchWheel (currentPitchWheelPosition, maxPitchWheelSemitones)));
    midiNumber = midiNoteNumber;

    adsr.noteOn();
}

void FoleysSynth::FoleysVoice::stopNote (float velocity, bool allowTailOff)
{
    adsr.noteOff();

    if (! allowTailOff)
        adsr.reset();
}

void FoleysSynth::FoleysVoice::pitchWheelMoved (int newPitchWheelValue)
{
    osc.setFrequency (getFrequencyForNote (midiNumber, getDetuneFromPitchWheel (newPitchWheelValue, maxPitchWheelSemitones)));
}

void FoleysSynth::FoleysVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    DBG ("Controller moved: " << controllerNumber << ": " << newControllerValue);
}

void FoleysSynth::FoleysVoice::renderNextBlock (AudioBuffer<float>& outputBuffer,
                                                int startSample,
                                                int numSamples)
{
    if (! adsr.isActive())
        return;

    while (numSamples > 0)
    {
        auto left = std::min (numSamples, buffer.getNumSamples());
        auto block = dsp::AudioBlock<float> (buffer).getSingleChannelBlock (0).getSubBlock (0, left);
        buffer.clear();

        dsp::ProcessContextReplacing<float> context (block);
        osc.process (context);
        adsr.applyEnvelopeToBuffer (buffer, 0, left);
        outputBuffer.addFrom (0, startSample, buffer.getReadPointer (0), left);

        startSample += left;
        numSamples  -= left;
    }
}

void FoleysSynth::FoleysVoice::setCurrentPlaybackSampleRate (double newRate)
{
    SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);

    dsp::ProcessSpec spec;
    spec.sampleRate = newRate;
    spec.numChannels = 1;
    osc.prepare (spec);
}

double FoleysSynth::FoleysVoice::getFrequencyForNote (int noteNumber, double detune, double concertPitch) const
{
    return concertPitch * std::pow (2.0, (noteNumber + detune - 69.0) / 12.0);
}

double FoleysSynth::FoleysVoice::getDetuneFromPitchWheel (int wheelValue, double semitonesToDetune) const
{
    return semitonesToDetune * ((wheelValue / 8192.0) - 1.0);
}


ADSR::Parameters FoleysSynth::FoleysSound::getADSR()
{
    ADSR::Parameters parameters;

    return parameters;
}
