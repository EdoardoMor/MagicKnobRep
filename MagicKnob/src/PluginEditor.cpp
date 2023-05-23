/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <iostream>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utils.cpp"


#include <torch/torch.h>
#include <torch/script.h>
#include <cmath>
#include <chrono>
#include <thread>

//==============================================================================
MagicKnobEditor::MagicKnobEditor (MagicKnobProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor(p), 
    miniPianoKbd{kbdState, juce::MidiKeyboardComponent::horizontalKeyboard}

{    
    // listen to the mini piano
    kbdState.addListener(this);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1000, 500);

    addAndMakeVisible(envToggle);
    envToggle.addListener(this);
    
    addAndMakeVisible(modIndexSlider);
    modIndexSlider.setRange(0.1, 10, 0.1);
    modIndexSlider.addListener(this);
    modIndexSlider.setSliderStyle(juce::Slider::ThreeValueHorizontal);

    addAndMakeVisible(modDepthSlider);
    modDepthSlider.setRange(0, 5);
    modDepthSlider.addListener(this);
    modDepthSlider.setSliderStyle(juce::Slider::ThreeValueHorizontal);

    addAndMakeVisible(superKnob);
    superKnob.addListener(this);
    superKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
    superKnob.setRange(0,1);

    addAndMakeVisible(miniPianoKbd);

   


}

MagicKnobEditor::~MagicKnobEditor()
{
}

//==============================================================================
void MagicKnobEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    
   // g.setColour (juce::Colours::white);
   // g.setFont (15.0f);
   // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    
}

void MagicKnobEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    float rowHeight = getHeight()/5; 
    envToggle.setBounds(0, 0, getWidth()/2, rowHeight);
    modIndexSlider.setBounds(0, rowHeight , getWidth(), rowHeight);
    modDepthSlider.setBounds(0, rowHeight *2, getWidth(), rowHeight);
    superKnob.setBounds(getWidth()/2, 0, getWidth()/2, rowHeight);
    miniPianoKbd.setBounds(0, rowHeight * 3, getWidth(), rowHeight);

    
}

 void MagicKnobEditor::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &modDepthSlider || 
        slider == &modIndexSlider){
        audioProcessor.updateFMParams(modIndexSlider.getValue(), 
                                      modDepthSlider.getValue());
    }

    if (slider == &superKnob){

        double high = modIndexSlider.getMaxValue();
        double low = modIndexSlider.getMinValue();
        double weight = high - low; // m
        double bias = low; // b

        double superV = superKnob.getValue(); // x
        double newV = (superV * weight) + bias; // y
        modIndexSlider.setValue(newV);

        high = modDepthSlider.getMaxValue();
        low = modDepthSlider.getMinValue();
        weight = high - low; // m
        bias = low; // b

        superV = superKnob.getValue(); // x
        newV = (superV * weight) + bias; // y
        modDepthSlider.setValue(newV);
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
    if (btn == &envToggle){
        double envLen = 0;
        if (envToggle.getToggleState()) { // one
            envLen = 1;
        }
        audioProcessor.setEnvLength(envLen);
    }

}


void MagicKnobEditor::handleNoteOn(juce::MidiKeyboardState *source, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::MidiMessage msg1 = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    audioProcessor.addMidi(msg1, 0);
    
}

void MagicKnobEditor::handleNoteOff(juce::MidiKeyboardState *source, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::MidiMessage msg2 = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber, velocity);
    audioProcessor.addMidi(msg2, 0);  
}



void MagicKnobEditor::InferModels(std::vector<torch::jit::IValue> my_input, torch::Tensor phase, int size) 
{
 

}

