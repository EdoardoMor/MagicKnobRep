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
#include <torch/torch.h>
#include <torch/script.h>
#include <iostream>
#include <format>
#include <cmath>

#include "PluginProcessor.h"
#include "NeuralNetwork.h"

//==============================================================================
/**
*/

class Dot  : public juce::Component
{
public:
    Dot () :

          colour (juce::Colours::white)
    {
        setSize (10, 10);
        step(0 , 0);
        setAlwaysOnTop(false);
    }

    void step(int x, int y)
    {

        setCentrePosition (x, y);
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (colour);
        g.fillEllipse (0, 0, (float) getWidth() , (float) getHeight() );

        g.setColour (juce::Colours::darkgrey);
        g.drawEllipse (0, 0, (float) getWidth() , (float) getHeight() , 0.5f);
    }


private:
    juce::Point<float> position, speed;
    juce::Colour colour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Dot)
};

class Area : public juce::TextButton
{
public:

    Area(MagicKnobProcessor& proc) : TextButton(), audioProc(proc), dot{} 
    {
        addAndMakeVisible(dot);
    }
    
    void mouseMove(const juce::MouseEvent& e) override

    {
        float eX = (float) e.getPosition().getX();
        float eY = (float) e.getPosition().getY();
        float x = eX / 440.0;
        float y = eY / 195.0;
        DBG(x);
        DBG(y);

        audioProc.setLPFKnobValue(x);
        audioProc.setDistKnobValue(y);

        dot.step(eX, eY);

    }


private:
    MagicKnobProcessor& audioProc;
    Dot dot;
};

// KNOBPAGE ------|------|------|------|------|------|------|------|------|------|------|------|
class KnobPage : public juce::Component, public juce::Button::Listener, public juce::Slider::Listener {

public:
    KnobPage(MagicKnobProcessor &proc) : audioProc(proc)
    {
        setSize(500, 500);

        // superKnob
        addAndMakeVisible(superKnobLabel);
        superKnobLabel.setText("SuperKnob", juce::dontSendNotification);
        superKnobLabel.setFont(18.0f);
        superKnobLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(superKnob);
        superKnob.addListener(this);
        superKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        superKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 80, 20);
        superKnob.setNumDecimalPlacesToDisplay(2);
        superKnob.setRange(0, 1);

        // distKnob
        addAndMakeVisible(distKnobLabel);
        distKnobLabel.setText("Distortion", juce::dontSendNotification);
        distKnobLabel.setFont(18.0f);
        distKnobLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(distKnob);
        distKnob.addListener(this);
        distKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        distKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 80, 20);
        distKnob.setNumDecimalPlacesToDisplay(2);
        distKnob.setRange(0, 1);

        addAndMakeVisible(lpfKnobLabel);
        lpfKnobLabel.setText("LPF", juce::dontSendNotification);
        lpfKnobLabel.setFont(18.0f);
        lpfKnobLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(lpfKnob);
        lpfKnob.addListener(this);
        lpfKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lpfKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 80, 20);
        lpfKnob.setNumDecimalPlacesToDisplay(2);
        lpfKnob.setRange(0, 1);

        addAndMakeVisible(powerToggle);
        powerToggle.setButtonText("ON/OFF");
        powerToggle.addListener(this);

        addAndMakeVisible(trainButton);
        trainButton.addListener(this);
        trainButton.setButtonText("Train SuperKnob");
        trainButton.setEnabled(false);

        addAndMakeVisible(addSampleButton);
        addSampleButton.addListener(this);
        addSampleButton.setButtonText("Add training datapoint");
    }

    void resized() override
    {
        // This is generally where you'll want to lay out the positions of any
        // subcomponents in your editor..
        int rowHeight = (int)getHeight() / 2;
        int columnWidth = (int)getWidth() / 2;
        int labelHeight = 20;
        int buttonHeight = (int)rowHeight / 3;

        // first row
        superKnob.setBounds(0, 0, columnWidth, rowHeight);
        superKnobLabel.setBounds(0, rowHeight / 2 - labelHeight, columnWidth, labelHeight);
        distKnob.setBounds(columnWidth, 0, columnWidth, rowHeight);
        distKnobLabel.setBounds(columnWidth, rowHeight / 2 - labelHeight, columnWidth, labelHeight);

        // second row
        powerToggle.setBounds(0, rowHeight, columnWidth, buttonHeight);
        addSampleButton.setBounds(0, rowHeight + buttonHeight, columnWidth, buttonHeight);
        trainButton.setBounds(0, rowHeight + 2 * buttonHeight, columnWidth, buttonHeight);
        lpfKnob.setBounds(columnWidth, rowHeight, columnWidth, rowHeight);
        lpfKnobLabel.setBounds(columnWidth, rowHeight * 3 / 2 - labelHeight, columnWidth, labelHeight);
    }

    void sliderValueChanged(juce::Slider *slider) override
    {
        float read = slider->getValue();

        if (slider == &superKnob)
        {
            if (nn.isTrained())
            {
                // set values of other sliders using the NN
                std::vector<float> nn_outs = nn.forward(std::vector<float>{(float)superKnob.getValue()});
                distKnob.setValue(nn_outs[0]);
                lpfKnob.setValue(nn_outs[1]);
            }
            // distKnob.setValue(read);
            // lpfKnob.setValue(read);
        }
        if (slider == &distKnob)
            audioProc.setDistKnobValue(read);
        if (slider == &lpfKnob)
            // set values of other sliders using the NN
            // std::vector<float> nn_outs = nn.forward(std::vector<float>{(float)superKnob.getValue()});
            // modDepthSlider.setValue(nn_outs[0] * modDepthSlider.getMaximum());
            // modIndexSlider.setValue(nn_outs[1] * modIndexSlider.getMaximum());
            audioProc.setLPFKnobValue(read);
    }

    void buttonClicked(juce::Button *btn) override
    {
        if (btn == &powerToggle)
            audioProc.togglePowerState();
        if (btn == &addSampleButton)
        {
            float in = (float)superKnob.getValue();
            float outDist = (float)distKnob.getValue();
            float outLPF = (float)lpfKnob.getValue();
            std::cout << "Superknob: " << in << "\nDist: " << outDist << "\nLPF: " << outLPF << std::endl
                      << std::endl;
            nn.addTrainingData({in}, {outDist, outLPF});
            trainButton.setEnabled(true);
        }
        if (btn == &trainButton)
        {
            if (!nn.isTrained())
            {
                nn.runTraining(1000000);
                trainButton.setButtonText("Reset SuperKnob");
            }
            else
            {
                std::cout << nn.isTrained() << std::endl;
                this->nn = NeuralNetwork(1, 2);
                std::cout << nn.isTrained() << std::endl;
                trainButton.setButtonText("Train SuperKnob");
                trainButton.setEnabled(false);
            }
        }
    }

private:
    juce::Label superKnobLabel, distKnobLabel, lpfKnobLabel;
    juce::Slider superKnob, distKnob, lpfKnob;
    juce::ToggleButton powerToggle;
    juce::TextButton trainButton, addSampleButton;

    NeuralNetwork nn{1, 2};
    MagicKnobProcessor &audioProc;
};

class RectPage : public juce::Component, public juce::Button::Listener {

public:
    RectPage(MagicKnobProcessor& proc) : audioProc(proc), area(proc) {

        setSize(500, 500);

        addAndMakeVisible(area);
        //area.addListener(this);
        area.setName("area");

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
        area.setBounds(10, 150 , getWidth() - 20, rowHeight * 2);

    }

    void buttonClicked(juce::Button* btn) override
    {
        if (btn == &powerToggle) {
            audioProc.togglePowerState();
        }

    }




private:
    Area area;
    juce::ToggleButton powerToggle;

    MagicKnobProcessor& audioProc;

};


// TAB COMPONTENT   ------|------|------|------|------|------|------|------|------|------|------|
class OurTabbedComponent : public juce::TabbedComponent {

public:
    OurTabbedComponent(MagicKnobProcessor &proc) : TabbedComponent(juce::TabbedButtonBar::TabsAtRight), audioP(proc)
    {
        addTab("KnobPage", juce::Colours::indianred, new KnobPage(proc), true);
        addTab("RectPage", juce::Colours::indianred, new RectPage(proc), true);
    }

private:
    MagicKnobProcessor &audioP;
};

// EDITOR   ------|------|------|------|------|------|------|------|------|------|------|------|
class MagicKnobEditor : public juce::AudioProcessorEditor
{

public:
    MagicKnobEditor(MagicKnobProcessor &);
    ~MagicKnobEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    // void sliderValueChanged (juce::Slider *slider) override;
    // void buttonClicked(juce::Button* btn) override;

    // juce::AudioBuffer<float> getAudioBufferFromFile(juce::File file); //POTREBBE TORNARE UTILE

private:
    // juce::ToggleButton powerToggle;
    // juce::Slider superKnob;

    MagicKnobProcessor &audioProcessor;

    OurTabbedComponent tabs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MagicKnobEditor)
};