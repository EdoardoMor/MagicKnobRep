#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <iostream>
#include <math.h>

#include "TabPage.h"
#include "PluginProcessor.h"

// DOT  ------|------|------|------|------|------|------|------|------|------|------|------|
class Dot : public juce::Component
{
public:
    Dot() : colourFill{juce::Colours::white}, colourOutline{juce::Colours::khaki}
    {
        setSize(diameter, diameter);
        setAlwaysOnTop(false);
        setInterceptsMouseClicks(false, false);

        step(0, 0);
    }

    void step(int x, int y)
    {
        setCentrePosition(x, y);
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(colourFill);
        g.fillEllipse(0, 0, diameter, diameter);

        g.setColour(colourOutline);
        g.drawEllipse(0, 0, diameter, diameter, 2.0f);
    }

private:
    float diameter = 10;
    juce::Point<float> position, speed;
    juce::Colour colourFill, colourOutline;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Dot)
};

// AREA     ------|------|------|------|------|------|------|------|------|------|------|------|
class Area : public juce::TextButton
{
public:
    Area(MagicKnobProcessor &proc) : TextButton(), audioProc(proc), dot{}
    {
        addAndMakeVisible(dot);

        dot.addMouseListener(this, false);
    }

    void mouseDrag(const juce::MouseEvent &e) override
    {
        float eX = (float)e.getPosition().getX();
        float eY = (float)e.getPosition().getY();

        if (eX >= 0 && eX <= widthArea && eY >= 0 && eY <= heightArea)
        {
            float x = eX / widthArea;
            float y = 1 - eY / heightArea;
            // DBG("X: " + x + " Y: " + y);
            std::cout << "X: " << x << " Y: " << y << std::endl;

            audioProc.setLPFKnobValue(x);
            audioProc.setDistKnobValue(y);

            dot.step(eX, eY);
        }
    }

    void updateMeasures()
    {
        widthArea = getWidth();
        heightArea = getHeight();
    }

private:
    int widthArea, heightArea;

    MagicKnobProcessor &audioProc;
    Dot dot;
};

// RECTPAGE     ------|------|------|------|------|------|------|------|------|------|------|------|
class RectPage : public TabPage
{

public:
    RectPage(MagicKnobProcessor &proc) : TabPage(proc), area(proc)
    {
        addAndMakeVisible(area);
        area.setName("area");
        area.setEnabled(false);

        addAndMakeVisible(lpfModelLabel);
        lpfModelLabel.setText("LPF", juce::dontSendNotification);
        lpfModelLabel.setFont(18.0f);
        lpfModelLabel.setJustificationType(juce::Justification::centredRight);

        addAndMakeVisible(currLPFModelLabel);
        currLPFModelLabel.setText(audioProc.getCurrentModel("lpf"), juce::dontSendNotification);
        currLPFModelLabel.setFont(14.0f);
        currLPFModelLabel.setJustificationType(juce::Justification::centredLeft);

        addAndMakeVisible(distModelLabel);
        distModelLabel.setText("Distortion", juce::dontSendNotification);
        distModelLabel.setFont(18.0f);
        distModelLabel.setJustificationType(juce::Justification::centredRight);

        addAndMakeVisible(currDistModelLabel);
        currDistModelLabel.setText(audioProc.getCurrentModel("dist"), juce::dontSendNotification);
        currDistModelLabel.setFont(14.0f);
        currDistModelLabel.setJustificationType(juce::Justification::centredLeft);

        addAndMakeVisible(minLPFLabel);
        minLPFLabel.setText("0", juce::dontSendNotification);
        minLPFLabel.setFont(16.0f);
        minLPFLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(maxLPFLabel);
        maxLPFLabel.setText("1", juce::dontSendNotification);
        maxLPFLabel.setFont(16.0f);
        maxLPFLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(minDistLabel);
        minDistLabel.setText("0", juce::dontSendNotification);
        minDistLabel.setFont(16.0f);
        minDistLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(maxDistLabel);
        maxDistLabel.setText("1", juce::dontSendNotification);
        maxDistLabel.setFont(16.0f);
        maxDistLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(nextDistModelButton);
        nextDistModelButton.setButtonText("Change Distortion");
        nextDistModelButton.addListener(this);

        addAndMakeVisible(nextLPFModelButton);
        nextLPFModelButton.setButtonText("Change LPF");
        nextLPFModelButton.addListener(this);
    }

    void resized() override
    {
        int paddingSmall = 10, paddingLarge = 30, paddingButton = 4;
        int horizontalWidthLabel = 90; // == verticalHeightLabel
        int buttonHeight = 50;
        int labelDisplacement = 18;

        juce::Rectangle localBoundsWithPad = getLocalBounds().withSizeKeepingCentre(getWidth() - paddingSmall * 2, getHeight() - paddingSmall * 2);

        //  AREA
        juce::Rectangle<int> areaAndLabelRect = localBoundsWithPad.withTrimmedBottom(buttonHeight + paddingSmall);
        areaAndLabelRect = areaAndLabelRect.withTrimmedLeft(paddingSmall).withTrimmedRight(paddingSmall * 2);

        juce::Rectangle<int> areaRect = areaAndLabelRect.withTrimmedLeft(paddingLarge).withTrimmedBottom(paddingLarge);

        area.setBounds(areaRect);
        area.updateMeasures();

        // VERTICAL LABELS
        juce::Rectangle<int> verticalLabelRect = areaAndLabelRect.withTrimmedRight(areaRect.getWidth()).withTrimmedBottom(paddingLarge);
        juce::Rectangle<int> verticalLabel0Rect = verticalLabelRect.withTrimmedTop(verticalLabelRect.getHeight() - paddingLarge);
        verticalLabel0Rect.translate(0, paddingLarge / 2 - 1);
        juce::Rectangle<int> verticalLabel1Rect = verticalLabelRect.withTrimmedBottom(verticalLabelRect.getHeight() - paddingLarge);
        verticalLabel1Rect.translate(0, -paddingLarge / 2 + 1);

        juce::Point<int> pos = verticalLabelRect.getPosition();

        juce::Rectangle<int> verticalModelLabelRect = verticalLabelRect.withTrimmedBottom(verticalLabelRect.getHeight() - horizontalWidthLabel);
        verticalModelLabelRect = verticalModelLabelRect.withPosition(pos.getX(), pos.getY() + verticalLabelRect.getHeight() / 2 - horizontalWidthLabel);
        juce::Rectangle<int> verticalCurrModelLabelRect = verticalLabelRect.withTrimmedBottom(verticalLabelRect.getHeight() - horizontalWidthLabel);
        verticalCurrModelLabelRect = verticalCurrModelLabelRect.withPosition(pos.getX(), pos.getY() + verticalLabelRect.getHeight() / 2);

        setVerticalRotatedWithBounds(distModelLabel, true, verticalModelLabelRect);
        setVerticalRotatedWithBounds(currDistModelLabel, true, verticalCurrModelLabelRect);
        setVerticalRotatedWithBounds(minDistLabel, true, verticalLabel0Rect);
        setVerticalRotatedWithBounds(maxDistLabel, true, verticalLabel1Rect);

        // HORIZONTAL LABELS
        juce::Rectangle<int> horizontalLabelRect = areaAndLabelRect.withTrimmedTop(areaRect.getWidth()).withTrimmedLeft(paddingLarge);
        juce::Rectangle<int> horizontalLabel0Rect = horizontalLabelRect.withTrimmedRight(horizontalLabelRect.getWidth() - paddingLarge);
        horizontalLabel0Rect.translate(-paddingLarge / 2 + 1, 0);
        juce::Rectangle<int> horizontalLabel1Rect = horizontalLabelRect.withTrimmedLeft(horizontalLabelRect.getWidth() - paddingLarge);
        horizontalLabel1Rect.translate(paddingLarge / 2 - 1, 0);

        pos = horizontalLabelRect.getPosition();

        juce::Rectangle<int> horizontalModelLabelRect = horizontalLabelRect.withTrimmedRight(horizontalLabelRect.getWidth() - horizontalWidthLabel);
        horizontalModelLabelRect = horizontalModelLabelRect.withPosition(getWidth() / 2 - horizontalWidthLabel - labelDisplacement, pos.getY());
        // horizontalModelLabelRect = horizontalModelLabelRect.withPosition(pos.getX() + horizontalLabelRect.getWidth() / 2 - horizontalWidthLabel, pos.getY());
        juce::Rectangle<int> horizontalCurrModelLabelRect = horizontalLabelRect.withTrimmedLeft(horizontalLabelRect.getWidth() - horizontalWidthLabel);
        horizontalCurrModelLabelRect = horizontalCurrModelLabelRect.withPosition(getWidth() / 2 - labelDisplacement, pos.getY());
        // horizontalCurrModelLabelRect = horizontalCurrModelLabelRect.withPosition(pos.getX() + horizontalLabelRect.getWidth() / 2, pos.getY());

        lpfModelLabel.setBounds(horizontalModelLabelRect);
        currLPFModelLabel.setBounds(horizontalCurrModelLabelRect);
        minLPFLabel.setBounds(horizontalLabel0Rect);
        maxLPFLabel.setBounds(horizontalLabel1Rect);

        // BUTTONS
        juce::Rectangle<int> buttonRect = localBoundsWithPad.withTrimmedTop(localBoundsWithPad.getHeight() - buttonHeight);
        int singleButtonWidth = buttonRect.getWidth() / 3;
        juce::Rectangle<int> singleButtonRect1 = buttonRect.withTrimmedRight(singleButtonWidth * 2);
        singleButtonRect1 = singleButtonRect1.withSizeKeepingCentre(singleButtonWidth - paddingButton, buttonHeight);
        juce::Rectangle<int> singleButtonRect2 = buttonRect.withTrimmedRight(singleButtonWidth).withTrimmedLeft(singleButtonWidth);
        singleButtonRect2 = singleButtonRect2.withSizeKeepingCentre(singleButtonWidth - paddingButton, buttonHeight);
        juce::Rectangle<int> singleButtonRect3 = buttonRect.withTrimmedLeft(singleButtonWidth * 2);
        singleButtonRect3 = singleButtonRect3.withSizeKeepingCentre(singleButtonWidth - paddingButton, buttonHeight);

        powerToggle.setBounds(singleButtonRect1);
        nextDistModelButton.setBounds(singleButtonRect2);
        nextLPFModelButton.setBounds(singleButtonRect3);
    }

    void buttonClicked(juce::Button *btn) override
    {
        TabPage::buttonClicked(btn); // for powerState button

        if (btn == &nextDistModelButton)
        {
            audioProc.loadNextModel("dist");
            currDistModelLabel.setText(audioProc.getCurrentModel("dist"), juce::dontSendNotification);
        }
        if (btn == &nextLPFModelButton)
        {
            audioProc.loadNextModel("lpf");
            currLPFModelLabel.setText(audioProc.getCurrentModel("lpf"), juce::dontSendNotification);
        }
    }

    void updateDisplayedModels() override
    {
        currDistModelLabel.setText(audioProc.getCurrentModel("dist"), juce::dontSendNotification);
        currLPFModelLabel.setText(audioProc.getCurrentModel("lpf"), juce::dontSendNotification);
    }

private:
    static void setVerticalRotatedWithBounds(juce::Component &component, bool clockWiseRotation, juce::Rectangle<int> verticalBounds)
    {
        auto angle = M_PI / 2.0f;

        if (!clockWiseRotation)
            angle *= -1.0f;

        component.setTransform({});
        component.setSize(verticalBounds.getHeight(), verticalBounds.getWidth());
        component.setCentrePosition(0, 0);
        component.setTransform(juce::AffineTransform::rotation(angle).translated(verticalBounds.getCentreX(), verticalBounds.getCentreY()));
    }

    Area area;

    juce::Label distModelLabel, lpfModelLabel;
    juce::Label currDistModelLabel, currLPFModelLabel;
    juce::Label minDistLabel, maxDistLabel, minLPFLabel, maxLPFLabel;

    juce::TextButton nextDistModelButton, nextLPFModelButton;
};