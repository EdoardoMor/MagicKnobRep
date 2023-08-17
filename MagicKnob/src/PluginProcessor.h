/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

// not this:
// #include <JuceHeader.h>
// but this:

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_analytics/juce_analytics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_box2d/juce_box2d.h>
#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_opengl/juce_opengl.h>
#include <juce_osc/juce_osc.h>
#include <juce_product_unlocking/juce_product_unlocking.h>
#include <juce_video/juce_video.h>

#include <iostream>
#include <filesystem>
#include <RTNeural/RTNeural.h>

struct prediction
{
	int channel;
	float modelOutput;
};

// per model_dist (1 o 2) .json
using ModelType = RTNeural::ModelT<float, 2, 1, RTNeural::LSTMLayerT<float, 2, 16>, RTNeural::DenseT<float, 16, 1>>;

// per model.json
// using ModelType = RTNeural::ModelT<float, 1, 1, RTNeural::LSTMLayerT<float, 1, 32>, RTNeural::DenseT<float, 32, 1>>;

/*per neural_net_weights.json
using ModelType = RTNeural::ModelT<float, 1, 1,
		RTNeural::DenseT<float, 1, 8>,
		RTNeural::TanhActivationT<float, 8>,
		RTNeural::Conv1DT<float, 8, 4, 3, 2>,
		RTNeural::TanhActivationT<float, 4>,
		RTNeural::GRULayerT<float, 4, 8>,
		RTNeural::DenseT<float, 8, 1>
	>;
	*/

//==============================================================================
/**
 */
class MagicKnobProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	,
						   public juce::AudioProcessorARAExtension
#endif
{
public:
	//==============================================================================
	MagicKnobProcessor();
	~MagicKnobProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

	//==============================================================================
	juce::AudioProcessorEditor *createEditor() override;
	bool hasEditor() const override;

	//==============================================================================
	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String &newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock &destData) override;
	void setStateInformation(const void *data, int sizeInBytes) override;

	/** add some midi to be played at the sent sample offset*/
	void addMidi(juce::MidiMessage msg, int sampleOffset);

	void loadModel(std::ifstream &jsonStream, ModelType &model);

	void setDistKnobValue(float val);
	void setLPFKnobValue(float val);
	void togglePowerState();

private:
	bool powerState;

	float distKnobValue, lpfKnobValue;

	std::string modelFilePathDist, modelFilePathLPF; // models
	ModelType modelsDist[2], modelsLPF[2];

	// prediction predict(const float *input, int channel);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MagicKnobProcessor)
};
