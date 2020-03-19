/*
  ==============================================================================

    FoleysSynth.cpp
    Created: 23 Nov 2019 8:08:35pm
    Author:  Daniel Walz

  ==============================================================================
*/

#include "FoleysSynth.h"

namespace IDs
{
    static String paramAttack  { "attack" };
    static String paramDecay   { "decay" };
    static String paramSustain { "sustain" };
    static String paramRelease { "release" };
    static String paramGain    { "gain" };
}

//==============================================================================

int FoleysSynth::numOscillators = 8;

void FoleysSynth::addADSRParameters (AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto attack  = std::make_unique<AudioParameterFloat>(IDs::paramAttack,  "Attack",  NormalisableRange<float> (0.001f, 0.5f, 0.01f), 0.10f);
    auto decay   = std::make_unique<AudioParameterFloat>(IDs::paramDecay,   "Decay",   NormalisableRange<float> (0.001f, 0.5f, 0.01f), 0.10f);
    auto sustain = std::make_unique<AudioParameterFloat>(IDs::paramSustain, "Sustain", NormalisableRange<float> (0.0f,   1.0f, 0.01f), 1.0f);
    auto release = std::make_unique<AudioParameterFloat>(IDs::paramRelease, "Release", NormalisableRange<float> (0.001f, 0.5f, 0.01f), 0.10f);

    auto group = std::make_unique<AudioProcessorParameterGroup>("adsr", "ADRS", "|",
                                                                std::move (attack),
                                                                std::move (decay),
                                                                std::move (sustain),
                                                                std::move (release));
    layout.add (std::move (group));
}

void FoleysSynth::addOvertoneParameters (AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto group = std::make_unique<AudioProcessorParameterGroup>("oscillators", "Oscillators", "|");
    for (int i = 0; i < FoleysSynth::numOscillators; ++i)
    {
        group->addChild (std::make_unique<AudioParameterFloat>("osc" + String (i), "Oscillator " + String (i), NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
        group->addChild (std::make_unique<AudioParameterFloat>("detune" + String (i), "Detune " + String (i), NormalisableRange<float>(-0.5f, 0.5f, 0.01f), 0.0f));
    }

    layout.add (std::move (group));
}

void FoleysSynth::addGainParameters (AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto gain  = std::make_unique<AudioParameterFloat>(IDs::paramGain,  "Gain",  NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.70f);

    layout.add (std::make_unique<AudioProcessorParameterGroup>("output", "Output", "|", std::move (gain)));
}

//==============================================================================

FoleysSynth::FoleysSound::FoleysSound (AudioProcessorValueTreeState& stateToUse)
  : state (stateToUse)
{
    attack = dynamic_cast<AudioParameterFloat*>(state.getParameter (IDs::paramAttack));
    jassert (attack);
    decay = dynamic_cast<AudioParameterFloat*>(state.getParameter (IDs::paramDecay));
    jassert (decay);
    sustain = dynamic_cast<AudioParameterFloat*>(state.getParameter (IDs::paramSustain));
    jassert (sustain);
    release = dynamic_cast<AudioParameterFloat*>(state.getParameter (IDs::paramRelease));
    jassert (release);
    gain = dynamic_cast<AudioParameterFloat*>(state.getParameter (IDs::paramGain));
    jassert (gain);
}

ADSR::Parameters FoleysSynth::FoleysSound::getADSR()
{
    ADSR::Parameters parameters;
    parameters.attack  = attack->get();
    parameters.decay   = decay->get();
    parameters.sustain = sustain->get();
    parameters.release = release->get();
    return parameters;
}

//==============================================================================

FoleysSynth::FoleysVoice::FoleysVoice (AudioProcessorValueTreeState& state)
{
    for (int i=0; i < FoleysSynth::numOscillators; ++i)
    {
        oscillators.push_back (std::make_unique<BaseOscillator>());
        auto& osc = oscillators.back();
        osc->gain = dynamic_cast<AudioParameterFloat*>(state.getParameter ("osc" + String (i)));
        osc->detune = dynamic_cast<AudioParameterFloat*>(state.getParameter ("detune" + String (i)));
        osc->osc.get<0>().initialise ([](auto arg){return std::sin (arg);}, 512);
        osc->multiplier = i + 1;
    }

    gainParameter = dynamic_cast<AudioParameterFloat*>(state.getParameter (IDs::paramGain));
    jassert (gainParameter);

    oscillatorBuffer.setSize (1, internalBufferSize);
    voiceBuffer.setSize (1, internalBufferSize);
}

bool FoleysSynth::FoleysVoice::canPlaySound (SynthesiserSound* sound)
{
    return dynamic_cast<FoleysSound*>(sound) != nullptr;
}

void FoleysSynth::FoleysVoice::startNote (int midiNoteNumber,
                                          [[maybe_unused]]float velocity,
                                          SynthesiserSound* sound,
                                          int currentPitchWheelPosition)
{
    if (auto* foleysSound = dynamic_cast<FoleysSound*>(sound))
        adsr.setParameters (foleysSound->getADSR());

    pitchWheelValue = getDetuneFromPitchWheel (currentPitchWheelPosition);
    midiNumber = midiNoteNumber;

    adsr.noteOn();
}

void FoleysSynth::FoleysVoice::stopNote ([[maybe_unused]]float velocity,
                                         bool allowTailOff)
{
    adsr.noteOff();

    if (! allowTailOff)
        adsr.reset();
}

void FoleysSynth::FoleysVoice::pitchWheelMoved (int newPitchWheelValue)
{
    pitchWheelValue = getDetuneFromPitchWheel (newPitchWheelValue);
}

void FoleysSynth::FoleysVoice::controllerMoved ([[maybe_unused]]int controllerNumber, [[maybe_unused]]int newControllerValue)
{
}

void FoleysSynth::FoleysVoice::renderNextBlock (AudioBuffer<float>& outputBuffer,
                                                int startSample,
                                                int numSamples)
{
    if (! adsr.isActive())
        return;

    while (numSamples > 0)
    {
        auto left = std::min (numSamples, oscillatorBuffer.getNumSamples());
        auto block = dsp::AudioBlock<float> (oscillatorBuffer).getSingleChannelBlock (0).getSubBlock (0, left);

        dsp::ProcessContextReplacing<float> context (block);
        voiceBuffer.clear();
        for (auto& osc : oscillators)
        {
            auto oscGain = osc->gain->get();
            if (oscGain < 0.01)
                continue;

            updateFrequency (*osc);
            osc->osc.get<1>().setGainLinear (oscGain);
            oscillatorBuffer.clear();
            osc->osc.process (context);
            voiceBuffer.addFrom (0, 0, oscillatorBuffer.getReadPointer (0), left);
        }

        adsr.applyEnvelopeToBuffer (voiceBuffer, 0, left);

        const auto gain = gainParameter->get();
        outputBuffer.addFromWithRamp (0, startSample, voiceBuffer.getReadPointer (0), left, lastGain, gain);
        lastGain = gain;

        startSample += left;
        numSamples  -= left;
    }
}

void FoleysSynth::FoleysVoice::setCurrentPlaybackSampleRate (double newRate)
{
    SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);

    dsp::ProcessSpec spec;
    spec.sampleRate = newRate;
    spec.maximumBlockSize = internalBufferSize;
    spec.numChannels = 1;
    for (auto& osc : oscillators)
        osc->osc.prepare (spec);
}

double FoleysSynth::FoleysVoice::getFrequencyForNote (int noteNumber, double detune, double concertPitch) const
{
    return concertPitch * std::pow (2.0, (noteNumber + detune - 69.0) / 12.0);
}

double FoleysSynth::FoleysVoice::getDetuneFromPitchWheel (int wheelValue) const
{
    return (wheelValue / 8192.0) - 1.0;
}

void FoleysSynth::FoleysVoice::updateFrequency (BaseOscillator& oscillator)
{
    const auto freq = getFrequencyForNote (midiNumber,
                                           pitchWheelValue * maxPitchWheelSemitones
                                           + oscillator.detune->get());
    oscillator.osc.get<0>().setFrequency (freq * oscillator.multiplier);
}
