/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

// not this:
//#include <JuceHeader.h>
// but this:
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <torch/torch.h>
#include <torch/script.h>
#include <iostream>
#include <cmath>


#include "PluginProcessor.h"

//==============================================================================
/**
*/


class KnobPage : public juce::Component, public juce::Button::Listener, public juce::Slider::Listener {

public:
    KnobPage(MagicKnobProcessor& proc) : audioProc(proc) {

        setSize(500, 500);
        addAndMakeVisible(superKnob);
        superKnob.addListener(this);
        superKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
        superKnob.setRange(0, 1);

        addAndMakeVisible(powerToggle);
        powerToggle.setButtonText("ON/OFF");
        powerToggle.addListener(this);
    }

    void resized() override
    {
        // This is generally where you'll want to lay out the positions of any
        // subcomponents in your editor..
        float rowHeight = getHeight() / 5;
        powerToggle.setBounds(0, 0, getWidth() / 2, rowHeight);
        superKnob.setBounds(getWidth() / 2, 0, getWidth() / 2, rowHeight);

    }

    void sliderValueChanged(juce::Slider* slider) override
    {

        if (slider == &superKnob) {

            audioProc.setMagicKnobValue(slider->getValue());
        }
    }

    void buttonClicked(juce::Button* btn) override
    {
        if (btn == &powerToggle) {
            audioProc.togglePowerState();
        }

    }


private:
    juce::Slider superKnob;
    juce::ToggleButton powerToggle;

    MagicKnobProcessor& audioProc;

};

class OurTabbedComponent : public juce::TabbedComponent {
public:
    OurTabbedComponent(MagicKnobProcessor& proc) : TabbedComponent(juce::TabbedButtonBar::TabsAtRight), audioP(proc) {

        //juce::Colour* colour = new juce::Colour(50, 50, 50);
        addTab("KnobPage", juce::Colours::indianred, new KnobPage(proc), true);
    }
private:
    MagicKnobProcessor& audioP;

};


class MagicKnobEditor  :   public juce::AudioProcessorEditor
                          // listen to buttons
                          
                          // listen to sliders

                          

{
public:
    MagicKnobEditor (MagicKnobProcessor&);
    ~MagicKnobEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //void sliderValueChanged (juce::Slider *slider) override;
    //void buttonClicked(juce::Button* btn) override;


    //juce::AudioBuffer<float> getAudioBufferFromFile(juce::File file); //POTREBBE TORNARE UTILE


    


private:

    juce::ToggleButton powerToggle; 
    juce::Slider superKnob;

    // torch::nn::Linear linear{1, 2}; 
    //NeuralNetwork nn{1, 2};

    
    //------------------------------------------------------------------------------------
    MagicKnobProcessor& audioProcessor;

    OurTabbedComponent tabs;
    //KnobPage* knobpage;
    //juce::Component RectPage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MagicKnobEditor)
};