/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParametricEQAudioProcessor::ParametricEQAudioProcessor()
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
{
  addParameter(lowFrequencyParam = new juce::AudioParameterFloat("lowFrequency", "Low Corner frequency", 10.0f, 600.0f, 150.0f));
  addParameter(lowGainParam = new juce::AudioParameterFloat("lowGain", "Low Gain", -24.0f, 12.0f, 2.0f));
  addParameter(lowQParam = new juce::AudioParameterFloat("lowQ", "Low Q", 0.1f, 20.0f, 2.0f));

  addParameter(midFrequencyParam = new juce::AudioParameterFloat("midFrequency", "Mid Corner frequency", 150.0f, 3000.0f, 750.0f));
  addParameter(midGainParam = new juce::AudioParameterFloat("midGain", "Mid Gain", -24.0f, 12.0f, 2.0f));
  addParameter(midQParam = new juce::AudioParameterFloat("midQ", "Mid Q", 0.1f, 20.0f, 2.0f));

  addParameter(highFrequencyParam = new juce::AudioParameterFloat("highFrequency", "High Corner frequency", 450.0f, 18000.0f, 3000.0f));
  addParameter(highGainParam = new juce::AudioParameterFloat("highGain", "High Gain", -24.0f, 12.0f, 2.0f));
  addParameter(highQParam = new juce::AudioParameterFloat("highQ", "High Q", 0.1f, 20.0f, 2.0f));

}

ParametricEQAudioProcessor::~ParametricEQAudioProcessor()
{
}

//==============================================================================
const juce::String ParametricEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ParametricEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ParametricEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ParametricEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ParametricEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ParametricEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ParametricEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ParametricEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ParametricEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void ParametricEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ParametricEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  lowFilters.clear();
  for (int i = 0; i < getTotalNumInputChannels(); ++i) {
    juce::IIRFilter* filter;
    lowFilters.add(filter = new juce::IIRFilter());
  }
  midFilters.clear();
  for (int i = 0; i < getTotalNumInputChannels(); ++i) {
    juce::IIRFilter* filter;
    midFilters.add(filter = new juce::IIRFilter());
  }
  highFilters.clear();
  for (int i = 0; i < getTotalNumInputChannels(); ++i) {
    juce::IIRFilter* filter;
    highFilters.add(filter = new juce::IIRFilter());
  }
}

void ParametricEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ParametricEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ParametricEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  juce::ScopedNoDenormals noDenormals;
    auto numOutputChannels = getTotalNumOutputChannels();
    auto numInputChannels = getTotalNumInputChannels();
    auto numSamples = buffer.getNumSamples();

    float lowGain = pow(10.0f, lowGainParam->get()/ 20.0f);
    float lowQ = lowQParam->get();
    float lowFrequency = lowFrequencyParam->get();
    float midGain = pow(10.0f, midGainParam->get() / 20.0f);
    float midQ = midQParam->get();
    float midFrequency = midFrequencyParam->get();
    float highGain = pow(10.0f, highGainParam->get() / 20.0f);
    float highQ = highQParam->get();
    float highFrequency = highFrequencyParam->get();

    float sampleRate = (float)juce::AudioProcessor::getSampleRate();
    
    auto lowCoefficients = juce::IIRCoefficients::makeLowShelf(sampleRate, lowFrequency, lowQ, lowGain);
    for (int i = 0; i < lowFilters.size(); i++) lowFilters[i]->setCoefficients(lowCoefficients);
    auto peakCoefficients = juce::IIRCoefficients::makePeakFilter(sampleRate, midFrequency, midQ, midGain);
    for (int i = 0; i < midFilters.size(); i++) midFilters[i]->setCoefficients(peakCoefficients);
    auto highCoefficients = juce::IIRCoefficients::makeHighShelf(sampleRate, highFrequency, highQ, highQ);
    for (int i = 0; i < highFilters.size(); i++) highFilters[i]->setCoefficients(highCoefficients);

    for (int channel = 0; channel < numInputChannels; ++channel) {
      float* channelData = buffer.getWritePointer(channel);
      for (int i = 0; i < numSamples; ++i) channelData[i] = 2.0f * rand() / (float)RAND_MAX - 1.0f;
      lowFilters[channel]->processSamples(channelData, numSamples);
      midFilters[channel]->processSamples(channelData, numSamples);
      highFilters[channel]->processSamples(channelData, numSamples);
    }

    for (auto i = numInputChannels; i < numOutputChannels; ++i) buffer.clear (i, 0, buffer.getNumSamples());
}

//==============================================================================
bool ParametricEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ParametricEQAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void ParametricEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ParametricEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}

