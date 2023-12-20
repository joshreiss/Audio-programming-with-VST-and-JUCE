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
  addParameter(centreFrequencyParam = new juce::AudioParameterFloat("centreFrequency", "Centre frequency", 10.0f, 20000.0f, 1000.0f));
  addParameter(gainParam = new juce::AudioParameterFloat("gain", "Gain", -12.0f, 12.0f, 2.0f));
  addParameter(qParam = new juce::AudioParameterFloat("Q", "Q", 0.1f, 20.0f, 2.0f));
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
  filters.clear();
  for (int i = 0; i < getTotalNumInputChannels(); ++i) {
    juce::IIRFilter* filter;
    filters.add(filter = new juce::IIRFilter());
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

    float gain = gainParam->get();
    float Q = qParam->get();
    float centreFrequency = centreFrequencyParam->get();
    float sampleRate = (float)juce::AudioProcessor::getSampleRate();
    float normalisedFrequency = juce::MathConstants<float>::twoPi * centreFrequency / sampleRate;
    float linearGain = pow(10.0f, gain / 20.0f);
    
    const float bandwidth = normalisedFrequency / Q;
    const double two_cos_wc = -2.0*cos(normalisedFrequency);
    const double tan_half_bw = tan(bandwidth / 2.0);
    const double g_tan_half_bw = linearGain * tan_half_bw;
    const double sqrt_g = sqrt(linearGain);
        
    // setCoefficients takes arguments: b0, b1, b2, a0, a1, a2. Normalises filter according to a0 for time-domain implementations
    auto coefficients = juce::IIRCoefficients(sqrt_g + g_tan_half_bw, /* b0 */
      sqrt_g * two_cos_wc, /* b1 */
      sqrt_g - g_tan_half_bw, /* b2 */
      sqrt_g + tan_half_bw, /* a0 */
      sqrt_g * two_cos_wc, /* a1 */
      sqrt_g - tan_half_bw /* a2 */);

    for (int i = 0; i < filters.size(); i++) filters[i]->setCoefficients(coefficients);

    for (int channel = 0; channel < numInputChannels; ++channel) {
      float* channelData = buffer.getWritePointer(channel);
      for (int i = 0; i < numSamples; ++i) channelData[i] = 2.0f * rand() / (float)RAND_MAX - 1.0f;
      filters[channel]->processSamples(channelData, numSamples);
      filters[channel]->processSamples(channelData, numSamples);
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

