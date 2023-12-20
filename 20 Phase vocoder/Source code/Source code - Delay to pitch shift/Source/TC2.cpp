







/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessor::DelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
  : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
    .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
    .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
  )
#endif
{
  addParameter(delayTimeParam = new juce::AudioParameterFloat("delayTime", "Delay time", 0.0f, 2.0f, 0.0f));
  addParameter(feedbackParam = new juce::AudioParameterFloat("feedback", "Feedback", 0.0f, 0.999f, 0.0f));
  addParameter(dryWetMixParam = new juce::AudioParameterFloat("dryWetMix", "Dry/wet mix", 0.0f, 1.0, 0.0f));
}

DelayAudioProcessor::~DelayAudioProcessor()
{
}

//==============================================================================
const juce::String DelayAudioProcessor::getName() const
{
  return JucePlugin_Name;
}

bool DelayAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool DelayAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool DelayAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double DelayAudioProcessor::getTailLengthSeconds() const
{
  return 0.0;
}

int DelayAudioProcessor::getNumPrograms()
{
  return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
  // so this should be at least 1, even if you're not really implementing programs.
}

int DelayAudioProcessor::getCurrentProgram()
{
  return 0;
}

void DelayAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String DelayAudioProcessor::getProgramName(int index)
{
  return {};
}

void DelayAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void DelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  // Allocate and zero the delay buffer (size depends on sample rate)
  auto delayTime = delayTimeParam->get();
  delayBufferLength = (int)(2.0 * sampleRate);
  // Sanity check the result so we don't end up with any zero-length calculations
  if (delayBufferLength < 1) delayBufferLength = 1;
  delayBuffer.setSize(2, delayBufferLength);
  delayBuffer.clear();

  // Since delayTime is in seconds, use sample rate to find out what delay position offset should be
  delayReadPosition = (int)(delayWritePosition - (delayTime * getSampleRate()) + delayBufferLength) % delayBufferLength;
}

void DelayAudioProcessor::releaseResources()
{
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
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

void DelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)

{
  juce::ScopedNoDenormals noDenormals;

  // Helpful information about this block of samples:
  const int numInputChannels = getTotalNumInputChannels();    // How many input channels for our effect?
  const int numOutputChannels = getTotalNumOutputChannels();  // How many output channels for our effect?
  const int numSamples = buffer.getNumSamples();              // How many samples in the buffer for this block?
  float sampleRate = (float)juce::AudioProcessor::getSampleRate();
  auto delayTime = delayTimeParam->get();
  auto feedback = feedbackParam->get();
  auto dryWetMix = dryWetMixParam->get();
  delayReadPosition = (int)(delayWritePosition - (delayTime * getSampleRate()) + delayBufferLength) % delayBufferLength;
  if ((float)rand() / (float)RAND_MAX < 0.01) DBG(numInputChannels << " " << delayTime << " " << delayReadPosition << " " << delayWritePosition);
  int channel, dpr, dpw;
  // Go through each channel of audio that's passed in. We apply identical effects to each channel. 
 // For some effects, like stereo chorus or panner, you might do something different for each channel.
  for (channel = 0; channel < numInputChannels; ++channel) {
    // channelData is an array of length numSamples which contains the audio for one channel
    float* channelData = buffer.getWritePointer(channel);
    if ((float)rand() / (float)RAND_MAX < 0.01) DBG(channel << " " << numInputChannels << " " << delayTime << " " << delayReadPosition << " " << delayWritePosition);

    // delayData is the circular buffer for implementing delay on this channel
    float* delayData = delayBuffer.getWritePointer(juce::jmin(channel, delayBuffer.getNumChannels() - 1));

    // Make temporary copy of state variables that need to be maintained between calls to processBlock(). Each 
    // channel needs to be processed identically, processing one channel can't affect state variable for next channel.
    dpr = delayReadPosition;
    dpw = delayWritePosition;

    for (int i = 0; i < numSamples; ++i) {
      const float in = channelData[i];

      // Store current information in the delay buffer. delayData[dpr] is delay sample we just read,
      // i.e. what came out of buffer. delayData[dpw] is what we write to buffer, i.e. what goes in
      delayData[dpw] = in + (delayData[dpr] * feedback);

      // Output is input plus contents of delay buffer, weighted by dryWetMix. Store output in buffer, replacing input
      channelData[i] = (1 - dryWetMix) * in + dryWetMix * delayData[dpr];

      if (++dpr >= delayBufferLength) dpr = 0;
      if (++dpw >= delayBufferLength) dpw = 0;
    }
  }

  // Having made a local copy of the state variables for each channel, now transfer the result
  // back to the main state variable so they will be preserved for the next call of processBlock()
  delayReadPosition = dpr;
  delayWritePosition = dpw;

  // In case we have more outputs than inputs, we'll clear any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  for (int i = numInputChannels; i < numOutputChannels; ++i)
  {
    buffer.clear(i, 0, buffer.getNumSamples());
  }
}

//==============================================================================
bool DelayAudioProcessor::hasEditor() const
{
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayAudioProcessor::createEditor()
{
  return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void DelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void DelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  // You should use this method to restore your parameters from this memory block,
  // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new DelayAudioProcessor();
}
