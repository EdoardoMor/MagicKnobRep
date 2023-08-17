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
#include <cmath>

#include "PluginProcessor.h"

// KNOBPAGE ------|------|------|------|------|------|------|------|------|------|------|------|
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
    }

    void resized() override
    {
        // This is generally where you'll want to lay out the positions of any
        // subcomponents in your editor..
        float rowHeight = getHeight() / 2;
        float columnWidth = getWidth() / 2;
        float labelHeight = 20;

        // first row
        superKnob.setBounds(0, 0, columnWidth, rowHeight);
        superKnobLabel.setBounds(0, rowHeight / 2 - labelHeight, columnWidth, labelHeight);
        distKnob.setBounds(columnWidth, 0, columnWidth, rowHeight);
        distKnobLabel.setBounds(columnWidth, rowHeight / 2 - labelHeight, columnWidth, labelHeight);

        // second row
        powerToggle.setBounds(0, rowHeight, columnWidth, rowHeight);
        lpfKnob.setBounds(columnWidth, rowHeight, columnWidth, rowHeight);
        lpfKnobLabel.setBounds(columnWidth, rowHeight * 3 / 2 - labelHeight, columnWidth, labelHeight);
    }

    void sliderValueChanged(juce::Slider *slider) override
    {
        float read = slider->getValue();

        if (slider == &superKnob)
        {
            audioProc.setDistKnobValue(read);
            distKnob.setValue(read);
            audioProc.setLPFKnobValue(read);
            lpfKnob.setValue(read);
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
        {
            audioProc.togglePowerState();
        }
    }

private:
    juce::Label superKnobLabel, distKnobLabel, lpfKnobLabel;
    juce::Slider superKnob, distKnob, lpfKnob;
    juce::ToggleButton powerToggle;

    MagicKnobProcessor &audioProc;
};

// TAB COMPONTENT   ------|------|------|------|------|------|------|------|------|------|------|
class OurTabbedComponent : public juce::TabbedComponent
{

public:
    OurTabbedComponent(MagicKnobProcessor &proc) : TabbedComponent(juce::TabbedButtonBar::TabsAtRight), audioP(proc)
    {
        addTab("KnobPage", juce::Colours::indianred, new KnobPage(proc), true);
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
    // KnobPage* knobpage;
    // juce::Component RectPage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MagicKnobEditor)
};