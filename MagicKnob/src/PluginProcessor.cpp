/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <thread>
#include <future>
#include "juce_core/system/juce_PlatformDefs.h"

//==============================================================================
MagicKnobProcessor::MagicKnobProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
						 .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
						 .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
						 )
#endif

	  , powerState(false)
{


	// juce::MemoryInputStream jsonStream (BinaryData::tensorflow_model_json, BinaryData::tensorflow_model.jsonSize, false);
	// auto jsonInput = nlohmann::json::parse (jsonStream.readEntireStreamAsString().toStdString());
    // modelsT[0] = RTNeural::json_parser::parseJson<float> (jsonInput);
    // modelsT[1] = RTNeural::json_parser::parseJson<float> (jsonInput);
}

MagicKnobProcessor::~MagicKnobProcessor()
{
}

//==============================================================================
const juce::String MagicKnobProcessor::getName() const
{
	return JucePlugin_Name;
}

bool MagicKnobProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool MagicKnobProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool MagicKnobProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double MagicKnobProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int MagicKnobProcessor::getNumPrograms()
{
	return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
			  // so this should be at least 1, even if you're not really implementing programs.
}

int MagicKnobProcessor::getCurrentProgram()
{
	return 0;
}

void MagicKnobProcessor::setCurrentProgram(int index)
{
}

const juce::String MagicKnobProcessor::getProgramName(int index)
{
	return {};
}

void MagicKnobProcessor::changeProgramName(int index, const juce::String &newName)
{
}

//==============================================================================
void MagicKnobProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{

    auto modelFilePath = "C:/PROGETTI/STMAE/MagicKnobRep/model_dist_2.json";
    //assert(std::filesystem::exists(modelFilePath));

    DBG("Loading model from path: "); 
    DBG(modelFilePath);
    
    std::ifstream jsonStream(modelFilePath, std::ifstream::binary);
    loadModel(jsonStream, models[0]);

    auto modelFilePath2 = "C:/PROGETTI/STMAE/MagicKnobRep/model_dist_2.json";
    std::ifstream jsonStream2(modelFilePath2, std::ifstream::binary);
    loadModel(jsonStream2, models[1]);
    modelsT[0].reset();
    modelsT[1].reset();


}

void MagicKnobProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MagicKnobProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

		// This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void MagicKnobProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{

	// In case we have more outputs than inputs, this code clears any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	// I've added this to avoid people getting screaming feedback
	// when they first compile the plugin, but obviously you don't need to
	// this code if your algorithm already fills all the output channels.

	auto numInChannels = getNumInputChannels();
	
	for (int i = numInChannels; i < getNumOutputChannels(); ++i)
			buffer.clear(i, 0, buffer.getNumSamples());

	if (powerState)
	{
		// std::vector<float *> data;
		// for (int i = 0; i < numInChannels; ++i)
		// 	data.push_back(buffer.getWritePointer(i));

		// for (int i = 0; i < buffer.getNumSamples(); ++i)
		// {

		// 	/* DECOMMENTARE PER NET A 2 INPUT
		// 	float temp[2] = {channelData[i], magicKnobValue};
		// 	const float *inputArr = temp;
		// 	channelData[i] = model.forward(inputArr);
		// 	*/

		// 	int channel = 0;
		// 	for (auto channelData : data)
		// 	{
		// 		auto res1 = std::async(this->predict, &channelData[i], channel);
		// 	}
		// }

		// use compile-time model
        for (int ch = 0; ch < numInChannels; ++ch)
        {
            auto* x = buffer.getWritePointer (ch);
            for (int n = 0; n < buffer.getNumSamples(); ++n)
            {
                // DECOMMENTARE PER NET A 2 INPUT
                float temp[2] = {channelData[i], magicKnobValue};
                const float *inputArr = temp; 
                channelData[i] = modelsT[channel].forward(inputArr);
                

                /*
                float input[] = { channelData[i] };
                channelData[i] = models[channel].forward (input);
                */
            } else {
                channelData[i] = channelData[i];
            }
        }
	}

	/*
		  // now take the model for a spin :)
	  std::vector<float> inputs {1.0, 2.0, 3.0, 4.0};
	  std::vector<float> outputs {};
	  outputs.resize(inputs.size(), {});

	  for(size_t i = 0; i < inputs.size(); ++i)
	  {
		  outputs[i] = model.forward(&inputs[i]);
		  std::cout << "in " << inputs[i] << " out: " << outputs[i] << std::endl;
	  }
	*/
}

prediction MagicKnobProcessor::predict(const float *input, int channel)
{
	prediction result;
	result.modelOutput = modelsT[channel].forward(input);
	result.channel = channel;
	return result;
}

//==============================================================================
bool MagicKnobProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *MagicKnobProcessor::createEditor()
{
	return new MagicKnobEditor(*this);
}

//==============================================================================
void MagicKnobProcessor::getStateInformation(juce::MemoryBlock &destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void MagicKnobProcessor::setStateInformation(const void *data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
	return new MagicKnobProcessor();
}

void MagicKnobProcessor::loadModel(std::ifstream &jsonStream, ModelType &model)
{
	nlohmann::json modelJson;
	jsonStream >> modelJson;

	auto &lstm = model.get<0>();
	// note that the "lstm." is a prefix used to find the
	// lstm data in the json file so your python
	// needs to name the lstm layer 'lstm' if you use lstm. as your prefix
	std::string prefix = "lstm.";
	// for LSTM layers, number of hidden  = number of params in a hidden weight set
	// divided by 4
	auto hidden_count = modelJson[prefix + ".weight_ih_l0"].size() / 4;
	// assert that the number of hidden units is the same as this count
	// to ensure the json file we are importing matches the model we defined.
	RTNeural::torch_helpers::loadLSTM<float>(modelJson, prefix, lstm);

	auto &dense = model.get<1>();
	// as per the lstm prefix, here the json needs a key prefixed with dense.
	RTNeural::torch_helpers::loadDense<float>(modelJson, "dense.", dense);
}

void MagicKnobProcessor::setMagicKnobValue(float val)
{
	magicKnobValue = val;
}

void MagicKnobProcessor::togglePowerState()
{
	powerState = !powerState;
}