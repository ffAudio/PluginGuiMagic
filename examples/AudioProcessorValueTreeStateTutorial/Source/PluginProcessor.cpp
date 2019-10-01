/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TutorialProcessor();
}
