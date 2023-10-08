#include <regex>
#include <algorithm>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "juce_core/system/juce_PlatformDefs.h"

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
	  ,
	  powerState(false), currModelDist(-1), currModelLPF(-1)
{
	// modelFolder = "C:/PROGETTI/STMAE/MagicKnobRep/final_models/"

	modelFolder = "/Users/macdonald/Desktop/MagicKnobRep/final_models/";
	std::cout << "Looking for models at: " + modelFolder << std::endl;

	searchJsonModelsInDir(modelFolder);
}

MagicKnobProcessor::~MagicKnobProcessor()
{
}

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

void MagicKnobProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	loadNextModel("dist");
	loadNextModel("lpf");
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

	// use compile-time model
	for (int ch = 0; ch < numInChannels; ++ch)
	{
		auto *x = buffer.getWritePointer(ch);
		for (int n = 0; n < buffer.getNumSamples(); ++n)
		{
			if (powerState)
			{
				// std::cout << "DIST: " << distKnobValue << " LPF: " << lpfKnobValue << std::endl;

				// NET A 2 INPUT
				float tempDist[2] = {x[n], distKnobValue};
				const float *inputArr = tempDist;
				x[n] = modelsDist[ch].forward(inputArr);

				float tempLpf[2] = {x[n], lpfKnobValue};
				const float *inputArrLpf = tempLpf;
				x[n] = modelsLPF[ch].forward(inputArrLpf);

				// NET A 1 INPUT
				// float input[] = { x[n] };
				// x[n] = modelsDist[ch].forward (input);
			}
			else
			{
				x[n] = x[n];
			}
		}
	}
}

bool MagicKnobProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *MagicKnobProcessor::createEditor()
{
	return new MagicKnobEditor(*this);
}

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

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
	return new MagicKnobProcessor();
}

void MagicKnobProcessor::searchJsonModelsInDir(std::string modelFolder)
{
	for (const auto &entry : std::filesystem::directory_iterator(modelFolder))
	{
		std::string name, str_temp;
		std::stringstream ss(entry.path());
		std::vector<std::string> splitted;

		while (std::getline(ss, name, '/'))
		{
		} // need the last one

		std::stringstream ssName(name);
		std::vector<std::string> splittedName;

		while (std::getline(ssName, str_temp, '_'))
			splittedName.push_back(str_temp);

		if (splittedName.size() == 5)
		{
			if (splittedName[3] == "dist")
				distModelFiles.push_back(name);
			else if (splittedName[3] == "lpf2")
				lpfModelFiles.push_back(name);
		}
	}

	sort(distModelFiles.begin(), distModelFiles.end());
	sort(lpfModelFiles.begin(), lpfModelFiles.end());

	for (std::string model : distModelFiles)
		std::cout << "Found Distortion model: " + model << std::endl;

	for (std::string model : lpfModelFiles)
		std::cout << "Found LPF model: " + model << std::endl;

	std::cout << std::endl;
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
	auto hidden_count = modelJson[prefix + "weight_ih_l0"].size() / 4;
	// assert that the number of hidden units is the same as this count
	// to ensure the json file we are importing matches the model we defined.
	RTNeural::torch_helpers::loadLSTM<float>(modelJson, prefix, lstm);

	auto &dense = model.get<1>();
	// as per the lstm prefix, here the json needs a key prefixed with dense.
	RTNeural::torch_helpers::loadDense<float>(modelJson, "dense.", dense);
}

void MagicKnobProcessor::setDistKnobValue(float val)
{
	distKnobValue = val;
}

void MagicKnobProcessor::setLPFKnobValue(float val)
{
	lpfKnobValue = val;
}

float MagicKnobProcessor::getDistKnobValue()
{
	return distKnobValue;
}

float MagicKnobProcessor::getLPFKnobValue()
{
	return lpfKnobValue;
}

void MagicKnobProcessor::togglePowerState()
{
	powerState = !powerState;
	// std::cout << powerState << std::endl;
}

bool MagicKnobProcessor::getCurrPowerState()
{
	return powerState;
}

void MagicKnobProcessor::setCurrPowerState(bool newState)
{
	powerState = newState;
}

void MagicKnobProcessor::loadModelFromJson(ModelType *models, std::string path)
{
	std::cout << "Loading model at path: " << path << std::endl;
	std::ifstream jsonStream(path, std::ifstream::binary);
	loadModel(jsonStream, models[0]);

	jsonStream.clear();
	jsonStream.seekg(0, std::ios::beg);
	loadModel(jsonStream, models[1]);
}

void MagicKnobProcessor::loadNextModel(std::string knobId)
{
	if (knobId == "dist")
	{
		++currModelDist;

		modelsDist[0].reset();
		modelsDist[1].reset();

		loadModelFromJson(modelsDist, modelFolder + distModelFiles[currModelDist % distModelFiles.size()]);
	}

	if (knobId == "lpf")
	{
		++currModelLPF;

		modelsLPF[0].reset();
		modelsLPF[1].reset();

		loadModelFromJson(modelsLPF, modelFolder + lpfModelFiles[currModelLPF % lpfModelFiles.size()]);
	}
}

std::string MagicKnobProcessor::getCurrentModel(std::string knobId)
{
	std::string name = "", str_temp;
	if (knobId == "dist")
		name = distModelFiles[currModelDist % distModelFiles.size()];

	if (knobId == "lpf")
		name = lpfModelFiles[currModelLPF % lpfModelFiles.size()];

	std::stringstream ss(name);
	std::vector<std::string> splitted;

	while (std::getline(ss, str_temp, '_'))
		splitted.push_back(str_temp);

	assert(splitted.size() == 5);

	return splitted[4];
}