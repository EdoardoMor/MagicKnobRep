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
// #include <chrono>
// #include <thread>

//==============================================================================
MagicKnobEditor::MagicKnobEditor(MagicKnobProcessor &p)
	: AudioProcessorEditor(&p), audioProcessor(p), tabs{p}
{
	setOpaque(true);
	addAndMakeVisible(tabs);

	setSize(500, 500);
}

MagicKnobEditor::~MagicKnobEditor()
{
}

//==============================================================================
void MagicKnobEditor::paint(juce::Graphics &g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	// g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
	g.fillAll(juce::Colours::brown);

	// g.setColour (juce::Colours::white);
	// g.setFont (15.0f);
	// g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void MagicKnobEditor::resized()
{
	int padding = 4;

	tabs.setBounds(getLocalBounds().reduced(padding));
}
