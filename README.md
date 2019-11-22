PluginGuiMagic - by Foleys Finest Audio
=======================================


Plugin GUI Magic is a live editor for JUCE GUIs, focussed on Audio Plugins.
The main features are:

- Offer a default GUI from the audio processor parameters
- Uses audio parameter groups for the layout
- completely styleable via CSS-like structure
- Arranging of layout via drag and drop
- Out-of-the-box visualisers, including an FFT-Analyser and Oscilloscope
- Extensible with your own Components

![EqualizerExample Screenshot](https://raw.githubusercontent.com/ffAudio/PluginGuiMagic/master/screenshots/EqualizerExample-screenshot.png)

Pre-requisites
-------------

This editor works for JUCE GUIs. Best use the JUCE project templates
generated with Projucer, although it is no different from integrating
any other JUCE module.

To automatically connect parameters to the GUI, use the AudioProcessorValueTreeState
in your AudioProcessor. 

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


### ExtendingExample

[examples/ExtendingExample](examples/ExtendingExample)

This example shows, how you can add your custom components into the generated
GUI. There is a Component where you can edit the colours, as well as a
Component, that accesses a typesafe back reference to the AudioProcessor.


### EqualizerExample

[examples/EqualizerExample](examples/EqualizerExample)

An example of a 6 band IIR equalizer with complete plot curves, analysers and
more features following (this is the reference implementation for new features).


Deploying
---------

In the module there is a define called `FOLEYS_SHOW_GUI_EDITOR_PALLETTE`.
By setting this to 0, the editor is completely excluded from the build.

You can save your GUI to an XML and add it via BinaryData to the project.
Alternatively you can supply only a style definition and let the GUI
components be automatically generated each time.


Licensing
---------

The code is supplied under the BSD-3 clause license, free to use. 

Once you are selling products using foleys_gui_magic, we kindly ask 
you to support the ongoing development by buying an annual license 
for 35 GBP, or a perpetual license for 99 GBP. I will take your 
contribution into account, when I pick the next feature requests
to implement (no promises though).

A page to buy the licenses will appear shortly on https://foleysfinest.com


Contributing
------------

Any contribution in form of feedback, suggestions, criticism, test
results and code pull requests are very welcome and highly appreciated.


Enjoy using the GUI editor and let me know, how it goes.


Daniel - developer@foleysfinest.com

