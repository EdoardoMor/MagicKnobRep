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
        knobPage = new KnobPage(audioP);
        rectPage = new RectPage(audioP);

        addTab("KnobPage", juce::Colours::indianred, knobPage, true);
        addTab("RectPage", juce::Colours::indianred, rectPage, true);
    }

    void currentTabChanged(int newCurrentTabIndex, const juce::String &newCurrentTabName) override	
    {
        // std::cout << newCurrentTabName << std::endl;
        bool powerState = audioP.getCurrPowerState();

        if (newCurrentTabName == "KnobPage") 
            knobPage->updatePowerState(powerState);
        else 
            rectPage->updatePowerState(powerState);

    }

private:
    MagicKnobProcessor &audioP;

    KnobPage* knobPage;
    RectPage* rectPage;
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