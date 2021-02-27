#!/bin/bash

PROJUCER=JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer

${PROJUCER} --resave examples/EqualizerExample/EqualizerExample.jucer
${PROJUCER} --resave examples/ExtendingExample/ExtendingExample.jucer
${PROJUCER} --resave examples/APVTS_Tutorial/APVTS_Tutorial.jucer
${PROJUCER} --resave examples/FoleysSynth/FoleysSynth.jucer
${PROJUCER} --resave examples/SignalGenerator/SignalGenerator.jucer
${PROJUCER} --resave examples/PlayerExample/PlayerExample.jucer
