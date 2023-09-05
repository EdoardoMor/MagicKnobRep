#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <iostream>
#include <torch/torch.h>

#include "DCSlider.h"
#include "TabPage.h"
#include "PluginProcessor.h"
#include "NeuralNetwork.h"

/*
    KnobPage component for the main tab in PluginEditor
    Features:
    *   play manually using distortion and lpf knobs
    *   double click on distortion and lpf knobs to cycle between models
    *   add training points and train superknob to play using a single knob
*/
class KnobPage : public TabPage, public juce::Slider::Listener
{

public:
    KnobPage(MagicKnobProcessor &proc) : TabPage(proc)
    {
        int sliderLabelWidth = 80, sliderLabelHeight = 25;

        // superKnob
        addAndMakeVisible(superKnobLabel);
        superKnobLabel.setText("SuperKnob", juce::dontSendNotification);
        superKnobLabel.setFont(18.0f);
        superKnobLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(superKnob);
        superKnob.addListener(this);
        superKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        superKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, sliderLabelWidth, sliderLabelHeight);
        superKnob.setNumDecimalPlacesToDisplay(2);
        superKnob.setRange(0, 1);

        // distKnob
        addAndMakeVisible(distKnob);
        distKnob.addListener(this);
        distKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        distKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, sliderLabelWidth, sliderLabelHeight);
        distKnob.setNumDecimalPlacesToDisplay(2);
        distKnob.setRange(0, 1);

        addAndMakeVisible(lpfKnob);
        lpfKnob.addListener(this);
        lpfKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lpfKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, sliderLabelWidth, sliderLabelHeight);
        lpfKnob.setNumDecimalPlacesToDisplay(2);
        lpfKnob.setRange(0, 1);

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
        int paddingSmall = 10, paddingButton = 4;

        // LOCAL BOUNDS
        juce::Rectangle localBoundsWithPad = getLocalBounds().withSizeKeepingCentre(getWidth() - paddingSmall * 2, getHeight() - paddingSmall * 2);

        int knobWidth = localBoundsWithPad.getWidth() / 2;
        int knobHeight = localBoundsWithPad.getHeight() / 2;
        int labelHeight = 20;

        //  KNOBS
        juce::Rectangle superKnobRect = localBoundsWithPad.withTrimmedRight(knobWidth).withTrimmedBottom(knobHeight);
        superKnobRect = superKnobRect.withSizeKeepingCentre(knobWidth - paddingSmall / 2, knobHeight - paddingSmall / 2);

        juce::Point pos = superKnobRect.getPosition();
        juce::Rectangle<int> superKnobLabelRect = juce::Rectangle<int>(0, 0, knobWidth - paddingSmall / 2, labelHeight);
        superKnobLabelRect.setPosition(pos);
        superKnobLabelRect.translate(0, superKnobRect.getHeight() / 2 - labelHeight);

        juce::Rectangle distKnobRect = localBoundsWithPad.withTrimmedLeft(knobWidth).withTrimmedBottom(knobHeight);
        distKnobRect = distKnobRect.withSizeKeepingCentre(knobWidth - paddingSmall / 2, knobHeight - paddingSmall / 2);
        juce::Rectangle lpfKnobRect = localBoundsWithPad.withTrimmedLeft(knobWidth).withTrimmedTop(knobHeight);
        lpfKnobRect = lpfKnobRect.withSizeKeepingCentre(knobWidth - paddingSmall / 2, knobHeight - paddingSmall / 2);

        superKnob.setBounds(superKnobRect);
        superKnobLabel.setBounds(superKnobLabelRect);
        distKnob.setBounds(distKnobRect);
        lpfKnob.setBounds(lpfKnobRect);

        // BUTTONS
        juce::Rectangle buttonsRect = localBoundsWithPad.withTrimmedRight(knobWidth).withTrimmedTop(knobHeight);
        buttonsRect = buttonsRect.withSizeKeepingCentre(knobWidth - paddingSmall, knobHeight - paddingSmall / 2);
        buttonsRect.translate(0, paddingButton);

        int singleButtonHeight = buttonsRect.getHeight() / 3;
        int buttonWidth = buttonsRect.getWidth();
        juce::Rectangle<int> singleButtonRect1 = buttonsRect.withTrimmedBottom(singleButtonHeight * 2);
        singleButtonRect1 = singleButtonRect1.withSizeKeepingCentre(buttonWidth, singleButtonHeight - paddingButton);
        juce::Rectangle<int> singleButtonRect2 = buttonsRect.withTrimmedBottom(singleButtonHeight).withTrimmedTop(singleButtonHeight);
        singleButtonRect2 = singleButtonRect2.withSizeKeepingCentre(buttonWidth, singleButtonHeight - paddingButton);
        juce::Rectangle<int> singleButtonRect3 = buttonsRect.withTrimmedTop(singleButtonHeight * 2);
        singleButtonRect3 = singleButtonRect3.withSizeKeepingCentre(buttonWidth, singleButtonHeight - paddingButton);

        powerToggle.setBounds(singleButtonRect1);
        addSampleButton.setBounds(singleButtonRect2);
        trainButton.setBounds(singleButtonRect3);
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
        TabPage::buttonClicked(btn); // for powerState button

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

    void updateDisplayedModels() override
    {
        distKnob.updateDisplayedModel();
        lpfKnob.updateDisplayedModel();
    }

    void updateDisplayedValue() override
    {
        distKnob.setValue(audioProc.getDistKnobValue(), juce::dontSendNotification);
        lpfKnob.setValue(audioProc.getLPFKnobValue(), juce::dontSendNotification);
    }

private:
    NeuralNetwork nn{1, 2};

    juce::Label superKnobLabel;
    juce::Slider superKnob;
    DCSlider distKnob{audioProc, "dist", "Distortion"}, lpfKnob{audioProc, "lpf", "LPF"};
    juce::TextButton trainButton, addSampleButton;
};