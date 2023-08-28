#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "PluginProcessor.h"

/*
    Custom slider with double click and integrated name and current model labels
*/
class DCSlider : public juce::Slider
{
public:
    DCSlider(MagicKnobProcessor &proc, std::string id, std::string knobName) : audioProc(proc), sliderId{id}, knobName{knobName}
    {
        addAndMakeVisible(knobNameLabel);
        knobNameLabel.setText(knobName, juce::dontSendNotification);
        knobNameLabel.setFont(18.0f);
        knobNameLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(currModelLabel);
        currModelLabel.setText(audioProc.getCurrentModel(sliderId), juce::dontSendNotification);
        currModelLabel.setFont(14.0f);
        currModelLabel.setJustificationType(juce::Justification::centred);

        knobNameLabel.addMouseListener(this, false);
        currModelLabel.addMouseListener(this, false);
    }

    void mouseDoubleClick(const juce::MouseEvent &event) override
    {
        audioProc.loadNextModel(sliderId);
        updateDisplayedModel();
    }

    void resized() override
    {
        Slider::resized();

        int rowHeight = (int)getHeight();
        int columnWidth = (int)getWidth();
        int labelHeight = 20;

        knobNameLabel.setBounds(0, rowHeight / 2 - labelHeight*3/2, columnWidth, labelHeight);
        currModelLabel.setBounds(0, rowHeight / 2 - labelHeight/2, columnWidth, labelHeight);
    }

    void updateDisplayedModel()
    {
        currModelLabel.setText(audioProc.getCurrentModel(sliderId), juce::dontSendNotification);
    }

private:
    MagicKnobProcessor &audioProc;
    const std::string sliderId, knobName;

    juce::Label knobNameLabel, currModelLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DCSlider)
};