/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

// not this:
// #include <JuceHeader.h>
// but this:
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <iostream>

#include "KnobPage.h"
#include "RectPage.h"

/*
    Custom TabComponent with KnobPage and RectPage
*/
class OurTabbedComponent : public juce::TabbedComponent
{

public:
    OurTabbedComponent(MagicKnobProcessor &proc) : TabbedComponent(juce::TabbedButtonBar::TabsAtRight), audioP(proc)
    {
        // custom look and feel
        auto &lnf = getLookAndFeel();
        lnf.setColour(juce::Slider::thumbColourId, juce::Colours::brown);
        lnf.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::khaki);
        lnf.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
        lnf.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        lnf.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::brown);
        lnf.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::khaki);
        lnf.setColour(juce::TextButton::buttonColourId, juce::Colours::brown);
        lnf.setColour(juce::ComboBox::outlineColourId, juce::Colours::khaki); // outline for buttons, it's a bug

        sendLookAndFeelChange();

        knobPage = new KnobPage(audioP);
        rectPage = new RectPage(audioP);

        addTab("KnobPage", juce::Colours::indianred, knobPage, true);
        addTab("RectPage", juce::Colours::indianred, rectPage, true);

        setTabBarDepth(31); // to have an even size in the pages
    }

    void currentTabChanged(int newCurrentTabIndex, const juce::String &newCurrentTabName) override
    {
        // std::cout << newCurrentTabName << std::endl;
        bool powerState = audioP.getCurrPowerState();

        if (newCurrentTabName == "KnobPage")
        {
            knobPage->updatePowerState(powerState);
            knobPage->updateDisplayedModels();
        }
        else if (newCurrentTabName == "RectPage")
        {
            rectPage->updatePowerState(powerState);
            rectPage->updateDisplayedModels();
        }
    }

private:
    MagicKnobProcessor &audioP;

    KnobPage *knobPage;
    RectPage *rectPage;
};

/*
    PluginEditor
*/
class MagicKnobEditor : public juce::AudioProcessorEditor
{

public:
    MagicKnobEditor(MagicKnobProcessor &);
    ~MagicKnobEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    // juce::AudioBuffer<float> getAudioBufferFromFile(juce::File file); //POTREBBE TORNARE UTILE

private:
    MagicKnobProcessor &audioProcessor;

    OurTabbedComponent tabs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MagicKnobEditor)
};