/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <iostream>
#include "PluginProcessor.h"
#include "PluginEditor.h"


#include <torch/torch.h>
#include <torch/script.h>
#include <cmath>
#include <chrono>
#include <thread>


//==============================================================================
MagicKnobEditor::MagicKnobEditor (MagicKnobProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor(p)

{    

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 500);

    addAndMakeVisible(powerToggle);
    powerToggle.setButtonText("ON/OFF");
    powerToggle.addListener(this);
    

    addAndMakeVisible(superKnob);
    superKnob.addListener(this);
    superKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
    superKnob.setRange(0,1);

}

MagicKnobEditor::~MagicKnobEditor()
{
}

//==============================================================================
void MagicKnobEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    // g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colours::blue);
    
   // g.setColour (juce::Colours::white);
   // g.setFont (15.0f);
   // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    
}

void MagicKnobEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    float rowHeight = getHeight()/5; 
    powerToggle.setBounds(0, 0, getWidth()/2, rowHeight);
    superKnob.setBounds(getWidth()/2, 0, getWidth()/2, rowHeight);

    
}

 void MagicKnobEditor::sliderValueChanged (juce::Slider *slider)
{

    if (slider == &superKnob){

        audioProcessor.setMagicKnobValue(slider->getValue());
    }
}

/*
juce::AudioBuffer<float> MagicKnobEditor::getAudioBufferFromFile(juce::File file)
{
    
    //juce::AudioFormatManager formatManager - declared in header...`;
    auto* reader = formatManager.createReaderFor(file);
    juce::AudioBuffer<float> audioBuffer;
    audioBuffer.setSize(reader->numChannels, reader->lengthInSamples);
    reader->read(&audioBuffer, 0, reader->lengthInSamples, 0, true, true);
    delete reader;
    return audioBuffer;
    
}
*/
void MagicKnobEditor::buttonClicked(juce::Button* btn)
{
    if (btn == &powerToggle){
        audioProcessor.togglePowerState();
    }

}


