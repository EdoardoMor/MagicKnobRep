#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "PluginProcessor.h"

/**
    TabPage
    Parent component for KnobPage and RectPage
    Features:
    *   handles the powerToggle button
    *   virtual methods for updating models and values when a tab is changed
*/
class TabPage : public juce::Component, public juce::Button::Listener
{

public:
    TabPage(MagicKnobProcessor &proc) : audioProc(proc)
    {
        addAndMakeVisible(powerToggle);
        powerToggle.setButtonText("OFF");
        powerToggle.setClickingTogglesState(true);
        powerToggle.addListener(this);
        powerToggle.setColour(juce::TextButton::buttonColourId, juce::Colours::brown);
        powerToggle.setColour(juce::TextButton::buttonOnColourId, juce::Colours::brown);
    }

    void buttonClicked(juce::Button *btn) override
    {
        if (btn == &powerToggle)
        {
            audioProc.togglePowerState();
            updateTextPowerButton();
        }
    }

    void updatePowerState(bool powerState)
    {
        if (powerToggle.getToggleState() != powerState)
        {
            powerToggle.setToggleState(powerState, juce::dontSendNotification);
            updateTextPowerButton();
        }
    }

    virtual void updateDisplayedModels() = 0;
    virtual void updateDisplayedValues() = 0;

protected:
    juce::TextButton powerToggle;
    MagicKnobProcessor &audioProc;

private:
    void updateTextPowerButton()
    {
        if (powerToggle.getToggleState())
            powerToggle.setButtonText("ON");
        else
            powerToggle.setButtonText("OFF");
    }
};