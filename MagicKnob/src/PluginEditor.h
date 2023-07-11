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
#include "NeuralNetwork.h"

//==============================================================================
/**
*/



class MagicKnobEditor  :   public juce::AudioProcessorEditor,
                          // listen to buttons
                          public juce::Button::Listener, 
                          // listen to sliders
                          public juce::Slider::Listener 
                          

{
public:
    MagicKnobEditor (MagicKnobProcessor&);
    ~MagicKnobEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged (juce::Slider *slider) override;
    void buttonClicked(juce::Button* btn) override;


    //juce::AudioBuffer<float> getAudioBufferFromFile(juce::File file); //POTREBBE TORNARE UTILE


    


private:

    juce::ToggleButton envToggle; 
    juce::Slider superKnob;

    // torch::nn::Linear linear{1, 2}; 
    //NeuralNetwork nn{1, 2};

    
    //------------------------------------------------------------------------------------
    MagicKnobProcessor& audioProcessor;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MagicKnobEditor)
};


