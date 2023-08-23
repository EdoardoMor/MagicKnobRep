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
        addTab("KnobPage", juce::Colours::indianred, new KnobPage(audioP), true);
        addTab("RectPage", juce::Colours::indianred, new RectPage(audioP), true);
    }

private:
    MagicKnobProcessor &audioP;
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