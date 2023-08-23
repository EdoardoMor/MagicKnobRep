#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <iostream>
#include "DCSlider.h"
#include "PluginProcessor.h"
#include "NeuralNetwork.h"

#include <torch/torch.h>

/* 
    KnobPage component for the main tab in PluginEditor
    Features:
    *   play manually using distortion and lpf knobs
    *   double click on distortion and lpf knobs to cycle between models
    *   add training points and train superknob to play using a single knob
*/
class KnobPage : public juce::Component, public juce::Button::Listener, public juce::Slider::Listener
{

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
        addAndMakeVisible(distKnob);
        distKnob.addListener(this);
        distKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        distKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 80, 20);
        distKnob.setNumDecimalPlacesToDisplay(2);
        distKnob.setRange(0, 1);

        addAndMakeVisible(lpfKnob);
        lpfKnob.addListener(this);
        lpfKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lpfKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 80, 20);
        lpfKnob.setNumDecimalPlacesToDisplay(2);
        lpfKnob.setRange(0, 1);

        addAndMakeVisible(powerToggle);
        powerToggle.setButtonText("OFF");
        powerToggle.setClickingTogglesState(true);
        powerToggle.addListener(this);

        addAndMakeVisible(changeDistModelButton);
        changeDistModelButton.addListener(this);
        changeDistModelButton.setButtonText("Change dist");

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

        if (slider == &superKnob && nn.isTrained())
        {
            // set values of other sliders using the NN
            std::vector<float> nn_outs = nn.forward(std::vector<float>{(float)superKnob.getValue()});
            distKnob.setValue(nn_outs[0]);
            lpfKnob.setValue(nn_outs[1]);
        }
        if (slider == &distKnob)
            audioProc.setDistKnobValue(read);
        if (slider == &lpfKnob)
            audioProc.setLPFKnobValue(read);
    }

    void buttonClicked(juce::Button *btn) override
    {
        if (btn == &powerToggle)
            audioProc.togglePowerState();
        if (powerToggle.getToggleState())
            powerToggle.setButtonText("ON");
        else
            powerToggle.setButtonText("OFF");
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
    NeuralNetwork nn{1, 2};
    MagicKnobProcessor &audioProc;

    juce::Label superKnobLabel, distKnobLabel, lpfKnobLabel;
    juce::Slider superKnob;
    DCSlider distKnob{audioProc, "dist", "Distortion"}, lpfKnob{audioProc, "lpf", "LPF"};
    juce::TextButton powerToggle, trainButton, addSampleButton, changeDistModelButton;
};