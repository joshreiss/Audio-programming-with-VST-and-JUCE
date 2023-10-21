/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WahwahAudioProcessor::WahwahAudioProcessor()
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
  addParameter(centreFrequencyParam = new juce::AudioParameterFloat("centreFrequency", "Centre frequency",400.0f,2000.0f, 400.0f));
  addParameter(qParam = new juce::AudioParameterFloat("Q", "Quality factor Q", 2.0f, 20.0f, 5.0f));
}

WahwahAudioProcessor::~WahwahAudioProcessor()
{
}

//==============================================================================
const juce::String WahwahAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WahwahAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WahwahAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WahwahAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WahwahAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WahwahAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int WahwahAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WahwahAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String WahwahAudioProcessor::getProgramName (int index)
{
    return {};
}

void WahwahAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void WahwahAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  inverseSampleRate = 1.0 / sampleRate;

  // create as many identical filters as there are input channels
  for (int i = 0; i < getTotalNumInputChannels(); i++) filters.add(new ResonantLowpassFilter);

  // Update the filter settings to work with the current parameters and sample rate
  updateFilters();
}

void WahwahAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WahwahAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void WahwahAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)

{
  juce::ScopedNoDenormals noDenormals;

  int numInputChannels = getTotalNumInputChannels();
  int numFilters = filters.size();    // should be the same
  int numOutputChannels = getTotalNumOutputChannels();
  int numSamples = buffer.getNumSamples();
  float sampleRate = (float)juce::AudioProcessor::getSampleRate();
  auto centreFrequency = centreFrequencyParam->get();
  auto Q = qParam->get();
  updateFilters();

  for (int i = 0; i < buffer.getNumSamples(); ++i) buffer.getWritePointer(0)[i] = 2.0f * rand() / (float)RAND_MAX - 1.0f;

  int ch = 0;

  for (; ch < juce::jmin(numInputChannels, numFilters); ch++)
  {
    // Run the samples through the IIR filter whose coefficients define the parametric
    // equaliser. See juce_IIRFilter.cpp for the implementation.
    filters[ch]->processSamples(buffer.getWritePointer(ch), numSamples);
  }

  // If more outputs than inputs or not enough filters, clear remaining output channels 
  while (ch < numOutputChannels) buffer.clear(ch++, 0, numSamples);
  
}

//==============================================================================
bool WahwahAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WahwahAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void WahwahAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void WahwahAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

float WahwahAudioProcessor::interpolateSample(int type, float delayReadPosition, float* delayData, int delayBufferLength)
{
  switch (type)
  {
    default:
      // This line would only be reached if the type argument is invalid
      return 0.0f;
  }
}
// Called whenever LFO waveform, LFO frequency, or sweep width parameters are changed.
void WahwahAudioProcessor::parameterChanged()
{
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WahwahAudioProcessor();
}

void WahwahAudioProcessor::updateFilters()
{
  // The filter will produce a resonant peak of amplitude Q; bring everything
  // down somewhat to compensate, though try to maintain some perceptual balance
  // of being similar loudness. (This factor has been chosen somewhat arbitrarily.)
  const double kWahwahFilterGain = 0.5;

  for (auto& filter : filters)
  {
    filter->makeResonantLowpass(inverseSampleRate, centreFrequencyParam->get(), qParam->get(),
      kWahwahFilterGain);
  }
}
