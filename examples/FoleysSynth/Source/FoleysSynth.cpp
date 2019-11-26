/*
  ==============================================================================

    FoleysSynth.cpp
    Created: 23 Nov 2019 8:08:35pm
    Author:  Daniel Walz

  ==============================================================================
*/

#include "FoleysSynth.h"


void FoleysSynth::FoleysVoice::startNote (int midiNoteNumber,
                                          float velocity,
                                          SynthesiserSound* sound,
                                          int currentPitchWheelPosition)
{

}

void FoleysSynth::FoleysVoice::stopNote (float velocity, bool allowTailOff)
{

}

void FoleysSynth::FoleysVoice::pitchWheelMoved (int newPitchWheelValue)
{

}

void FoleysSynth::FoleysVoice::controllerMoved (int controllerNumber, int newControllerValue)
{

}

void FoleysSynth::FoleysVoice::renderNextBlock (AudioBuffer<float>& outputBuffer,
                                                int startSample,
                                                int numSamples)
{

}
