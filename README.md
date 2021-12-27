PluginGuiMagic - by Foleys Finest Audio
=======================================

+--------+---------+
| main   | develop |
+--------+---------+
| [![CMake](https://github.com/ffAudio/PluginGuiMagic/actions/workflows/workflow.yml/badge.svg?branch=main)](https://github.com/ffAudio/PluginGuiMagic/actions/workflows/workflow.yml)
| [![CMake](https://github.com/ffAudio/PluginGuiMagic/actions/workflows/workflow.yml/badge.svg?branch=develop)](https://github.com/ffAudio/PluginGuiMagic/actions/workflows/workflow.yml) |
+--------+---------+

Plugin GUI Magic is a live editor for JUCE GUIs, focussed on Audio Plugins.
The main features are:

- Offer a default GUI from the audio processor parameters
- Uses audio parameter groups for the layout
- completely styleable via CSS-like structures
- Arranging of layout via drag and drop
- Out-of-the-box visualisers, including an FFT-Analyser and Oscilloscope
- Extensible with your own Components

![EqualizerExample Screenshot](https://raw.githubusercontent.com/ffAudio/PluginGuiMagic/master/screenshots/EqualizerExample-screenshot.png)

[![CMake](https://github.com/ffAudio/PluginGuiMagic/actions/workflows/workflow.yml/badge.svg)](https://github.com/ffAudio/PluginGuiMagic/actions/workflows/workflow.yml)

Pre-requisites
-------------

This editor works for JUCE GUIs. Best use the JUCE project templates
generated with Projucer, although it is no different from integrating
any other JUCE module.

The PluginGuiMagic can connect to any juce::AudioProcessorParameter, the
AudioProcessorValueTreeState is no longer a requirement.

To add the GUI, add the foleys_gui_magic as submodule in the Projucer
to the project. To learn how to integrate the MagicPluginEditor, have
a look in the examples folder, the SignalGenerator example is a good starting
point.

Examples
--------

### SignalGenerator

[examples/SignalGenerator](examples/SignalGenerator)

This example demonstrates how to add a editable GUI for an AudioProcessor
without writing code for the GUI, only implementing the processor.


### ExtendingExample

[examples/ExtendingExample](examples/ExtendingExample)

This example shows, how you can add your custom components into the generated
GUI. There is a Component where you can edit the colours, as well as a
Component, that accesses a typesafe back reference to the AudioProcessor.


### EqualizerExample

[examples/EqualizerExample](examples/EqualizerExample)

An example of a 6 band IIR equalizer with complete plot curves, analysers and
more features following (this is the reference implementation for new features).

### FoleysSynth

[examples/FoleysSynth](examples/FoleysSynth)

An example additive synthesiser to show the MidiKeyboardComponent and how to 
create a nice GUI with PluginGuiMagic.

![FoleysSynth Screenshot](https://raw.githubusercontent.com/ffAudio/PluginGuiMagic/master/screenshots/FoleysSynth-screenshot.png)

Deploying
---------

In the module there is a define called `FOLEYS_SHOW_GUI_EDITOR_PALLETTE`.
By setting this to 0, the editor is completely excluded from the build.

You can save your GUI to an XML and add it via BinaryData to the project.
Alternatively you can supply only a style definition and let the GUI
components be automatically generated each time.


Licensing
---------

The code is supplied under the BSD-3 clause license, free to use for non-commercial purposes. 

To use PluginGuiMagic in commercial products, you should buy a license on our web-page.

To check the pricing and buy licenses please look at https://foleysfinest.com/developer/pluginguimagic

License holders will get priority on bug reports and feature requests.

Contributing
------------

Any contribution in form of feedback, suggestions, criticism, test
results and code pull requests are very welcome and highly appreciated.


Enjoy using the GUI editor and let me know, how it goes.


Daniel - developer@foleysfinest.com

