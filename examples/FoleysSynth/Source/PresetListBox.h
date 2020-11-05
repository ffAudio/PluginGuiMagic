
#pragma once

class PresetListBox   : public juce::ListBoxModel,
                        public juce::ChangeBroadcaster
{
public:
    PresetListBox (juce::ValueTree presetsNode) : presets (presetsNode)
    {
    }

    void setPresetsNode (juce::ValueTree node)
    {
        presets = node;
    }

    int getNumRows() override
    {
        return presets.getNumChildren();
    }

    void listBoxItemClicked (int rowNumber, const juce::MouseEvent&) override
    {
        if (onSelectionChanged)
            onSelectionChanged (rowNumber);
    }

    void paintListBoxItem (int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override
    {
        auto bounds = juce::Rectangle<int> (0, 0, width, height);
        if (rowIsSelected)
        {
            g.setColour (juce::Colours::grey);
            g.fillRect (bounds);
        }

        g.setColour (juce::Colours::silver);
        g.drawFittedText (presets.getChild (rowNumber).getProperty ("name", "foo").toString(), bounds, juce::Justification::centredLeft, 1);
    }

    std::function<void(int rowNumber)> onSelectionChanged;

private:
    juce::ValueTree presets;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetListBox)
};
