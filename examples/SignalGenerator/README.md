SignalGenerator example
=======================

This example demonstrates a simple SignalGenerator and how to add
a generated GUI, that can be completely styled without programming.

It also demonstrates how to add an Oscilloscope to visualise the
outgoing Signal.

![Signal Generator Screenshot](https://raw.githubusercontent.com/ffAudio/PluginGuiMagic/master/examples/SignalGenerator/Resources/SignalGenerator-screenshot.png)

Setup
-----

The setup is a standard JUCE Audio Plugin project. It is assumed, 
that you use the AudioProcessorValueTreeState to manage your parameters,
since the GUI MAGIC uses the Attachment classes to synchronise the
GUI Components.

Add the module foleys_gui_magic to the project. It has a dependency
to the juce::dsp module to create FFT and FrequencyResponce curves.

The Generate AudioProcessorEditor class is removed (don't forget the 
include ;-) ).

As a member add an instance of MagicProcessorState like this:

```
foleys::MagicProcessorState magicState { *this, treeState };
```

And in your createEditor() return an instance of MagicPluginEditor:

```
return new foleys::MagicPluginEditor (magicState);
```

This will already show a default editor populated from the generic
AudioProcessor::getParameterTree() method (yes, it supports
AudioParameterGroups!).


Add the oscilloscope
--------------------

There are different kiinds of visualisers, one is driven by
MagicPlotSource. You can inherit that class to provide your
own individual plot types.

To add the oscilloscope you can use the existing MagicOscilloscope.
To use it, add a member, so you can push the samples for displaying:

```
private:
    foleys::MagicPlotSource* oscilloscope = nullptr;
```

And register it as a new MagicPlotSource in your AudioProcessor's
constructor giving it an ID to recognise it in the GUI later:

```
oscilloscope = magicState.addPlotSource ("oscilloscope", std::make_unique<foleys::MagicOscilloscope>(0));
```

For the visualiser to work, you need to call the prepareToPlay
on your magicState from your processor's prepareToPlay():

```
void SignalGeneratorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // ...
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
}
```

And finally we push the samples to display. Note by choosing the place
in your code you can decide, what to visualise. For instance you could
create one plot before and one plot after your DSP code:

```
oscilloscope->pushSamples (buffer);
```

This is all that's necessary in the code.


Editing the GUI
---------------

If you enabled in the foleys_gui_magic module the setting 
FOLEYS_SHOW_GUI_EDITOR_PALLETTE (the default), the you see the
palette attached to your Plugin Editor. Here you can move the
Nodes around and specify the properties.

To add the oscilloscope, drag a Plot node into a View add the `source`
property and select the `oscilloscope` from the dropdown. You should
see the generated Oscilloscope immediately.


Baking a GUI into the code
--------------------------

Once you designed your GUI, you can save the resulting XML in your
Project (Source or create a Resources folder) and use the Projucer
to add it as binary resource. Re-save the Project and change the line
to create the editor to this (assuming you named the GUI magic.xml):

```
return new foleys::MagicPluginEditor (magicState, BinaryData::magic_xml, BinaryData::magic_xmlSize);
```

And adding `FOLEYS_SHOW_GUI_EDITOR_PALLETTE=0` to the preprocessor
defines in release avoids shipping your plugin with the Editor 
attached :-)

That should be all.


Enjoy playing with it and let me know, if it works or what you wish
to improve it.

