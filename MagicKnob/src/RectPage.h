#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <iostream>
#include "PluginProcessor.h"

// DOT  ------|------|------|------|------|------|------|------|------|------|------|------|
class Dot : public juce::Component
{
public:
    Dot() : colour(juce::Colours::white)
    {
        setSize(10, 10);
        step(0, 0);
        setAlwaysOnTop(false);
    }

    void step(int x, int y)
    {
        setCentrePosition(x, y);
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(colour);
        g.fillEllipse(0, 0, (float)getWidth(), (float)getHeight());

        g.setColour(juce::Colours::darkgrey);
        g.drawEllipse(0, 0, (float)getWidth(), (float)getHeight(), 0.5f);
    }

private:
    juce::Point<float> position, speed;
    juce::Colour colour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Dot)
};

// AREA     ------|------|------|------|------|------|------|------|------|------|------|------|
class Area : public juce::TextButton
{
public:
    Area(MagicKnobProcessor &proc) : TextButton(), audioProc(proc), dot{}
    {
        addAndMakeVisible(dot);
    }

    void mouseMove(const juce::MouseEvent &e) override
    {
        float eX = (float)e.getPosition().getX();
        float eY = (float)e.getPosition().getY();
        float x = eX / getWidth();
        float y = eY / getHeight();
        // DBG("X: " + x + " Y: " + y);
        std::cout << "X: " << x << " Y: " << y << std::endl;

        audioProc.setLPFKnobValue(x);
        audioProc.setDistKnobValue(y);

        dot.step(eX, eY);
    }

private:
    MagicKnobProcessor &audioProc;
    Dot dot;
};

// RECTPAGE     ------|------|------|------|------|------|------|------|------|------|------|------|
class RectPage : public juce::Component, public juce::Button::Listener
{

public:
    RectPage(MagicKnobProcessor &proc) : audioProc(proc), area(proc)
    {
        addAndMakeVisible(area);
        area.setName("area");
        area.setEnabled(false);
        area.setColour(juce::TextButton::buttonColourId, juce::Colours::brown);

        addAndMakeVisible(powerToggle);
        powerToggle.setButtonText("OFF");
        powerToggle.setClickingTogglesState(true);
        powerToggle.addListener(this);
    }

    void resized() override
    {
        // This is generally where you'll want to lay out the positions of any
        // subcomponents in your editor..
        int paddingTop = 10, paddingRightLeft = 30;

        juce::Rectangle rect = getLocalBounds();
        int rectWidth = rect.getWidth() - paddingRightLeft * 2;
        rect = rect.withSizeKeepingCentre(rectWidth, rectWidth).withY(paddingTop);

        area.setBounds(rect);

        int buttonHeight = getHeight() - getWidth();
        powerToggle.setBounds(0, getWidth(), getWidth() / 3, buttonHeight);
    }

    void buttonClicked(juce::Button *btn) override
    {
        if (btn == &powerToggle)
        {
            audioProc.togglePowerState();
            
            if (powerToggle.getToggleState())
                powerToggle.setButtonText("ON");
            else
                powerToggle.setButtonText("OFF");
        }
    }

    void updatePowerState(bool powerState)
    {
        if (powerToggle.getToggleState() != powerState)
        {
            powerToggle.setToggleState(powerState, juce::dontSendNotification);

            if (powerToggle.getToggleState())
                powerToggle.setButtonText("ON");
            else
                powerToggle.setButtonText("OFF");
        }
    }

private:
    Area area;
    juce::TextButton powerToggle;

    MagicKnobProcessor &audioProc;
};