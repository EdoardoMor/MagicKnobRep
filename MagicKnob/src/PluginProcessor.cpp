/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MagicKnobProcessor::MagicKnobProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
,  baseFrequency{440}, 
     carrPhase{0}, 
     carrDPhase{0}, 

     //modFreq{0}, 
     modPhase{0}, 
     modDPhase{0}, 
    
     modIndex{1}, 
     modDepth{0}, 
    
   amp{0}, ampTarget{0}, dAmp{0.00001}, ampMax{0.25}
{
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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MagicKnobProcessor::getCurrentProgram()
{
    return 0;
}

void MagicKnobProcessor::setCurrentProgram (int index)
{
}

const juce::String MagicKnobProcessor::getProgramName (int index)
{
    return {};
}

void MagicKnobProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MagicKnobProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{



    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    carrDPhase = getDPhase(baseFrequency, getSampleRate());
    modDPhase = getDPhase(baseFrequency*modIndex, getSampleRate());

}

void MagicKnobProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MagicKnobProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MagicKnobProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
 
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // I've added this to avoid people getting screaming feedback
    // when they first compile the plugin, but obviously you don't need to
    // this code if your algorithm already fills all the output channels.
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
//    std::cout << processor->controls.mode << std::endl;
//    std::cout << processor->controls.drive << std::endl;
//    std::cout << processor->controls.mix << std::endl;
//    std::cout << std::endl;
    
    for (int channel = 0; channel < getNumInputChannels(); ++channel) {
        float* channelData = buffer.getWritePointer (channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            channelData[i] = channelData[i];
        }
    }
  

  
 
}

//==============================================================================
bool MagicKnobProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MagicKnobProcessor::createEditor()
{
    return new MagicKnobEditor (*this);
}

//==============================================================================
void MagicKnobProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MagicKnobProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MagicKnobProcessor();
}

double MagicKnobProcessor::getDPhase(double freq, double sampleRate)
{
    double two_pi = 3.1415927 * 2;
    return (two_pi / sampleRate) * freq;   
}
void MagicKnobProcessor::updateFrequency(double newFreq)
{
    baseFrequency = newFreq; 
}

void MagicKnobProcessor::updateFMParams(double _modIndex, double _modDepth)
{
    modIndex = _modIndex;
    modDepth = _modDepth;
}


void MagicKnobProcessor::addMidi(juce::MidiMessage msg, int sampleOffset)
{
  midiToProcess.addEvent(msg, sampleOffset);
}


void MagicKnobProcessor::setEnvLength(double envLenSecs)
{
    // calculate damp based on length
    if (envLenSecs == 0){// 
        // always on mode
        amp = ampMax;
        dAmp = 0;
    }
    else{
        double envLenSamples = envLenSecs * getSampleRate();
        dAmp = 1 / envLenSamples / 2; 
    }
}

void loadModel(std::ifstream& jsonStream, ModelType& model)
{
    nlohmann::json modelJson;
    jsonStream >> modelJson;

    auto& lstm = model.get<0>();
    // note that the "lstm." is a prefix used to find the 
    // lstm data in the json file so your python
    // needs to name the lstm layer 'lstm' if you use lstm. as your prefix
    std::string prefix = "lstm.";
    // for LSTM layers, number of hidden  = number of params in a hidden weight set
    // divided by 4
    auto hidden_count = modelJson[prefix + ".weight_ih_l0"].size() / 4;
    // assert that the number of hidden units is the same as this count
    // to ensure the json file we are importing matches the model we defined.
    RTNeural::torch_helpers::loadLSTM<float> (modelJson, prefix, lstm);
  
    auto& dense = model.get<1>();
    // as per the lstm prefix, here the json needs a key prefixed with dense. 
    RTNeural::torch_helpers::loadDense<float> (modelJson, "dense.", dense);
}