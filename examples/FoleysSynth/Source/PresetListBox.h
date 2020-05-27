
#pragma once

class PresetListBox : public ListBoxModel
{
public:
    PresetListBox (ValueTree presetsNode) : presets (presetsNode)
    {
    }

    void setPresetsNode (ValueTree node)
    {
        presets = node;
    }

    int getNumRows() override
    {
        return presets.getNumChildren();
    }

    void listBoxItemClicked (int rowNumber, const MouseEvent&) override
    {
        if (onSelectionChanged)
            onSelectionChanged (rowNumber);
    }

    void paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override
    {
        auto bounds = Rectangle<int> (0, 0, width, height);
        if (rowIsSelected)
        {
            g.setColour (Colours::grey);
            g.fillRect (bounds);
        }

        g.setColour (Colours::silver);
        g.drawFittedText (presets.getChild (rowNumber).getProperty ("name", "foo").toString(), bounds, Justification::centredLeft, 1);
    }

    std::function<void(int rowNumber)> onSelectionChanged;

private:
    ValueTree presets;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetListBox)
};
