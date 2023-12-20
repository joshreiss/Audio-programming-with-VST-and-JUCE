/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KarplusStrongAudioProcessor::KarplusStrongAudioProcessor()
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
  addParameter(decayParam = new juce::AudioParameterFloat("decay", "Decay", 0.8f, 0.999f, 0.9f));
  addParameter(delayParam = new juce::AudioParameterFloat("delay", "Delay (ms)", 0.0f, 20.0f, 10.0f));
  addParameter(widthParam = new juce::AudioParameterFloat("width", "Width (ms)", 0.0f, 20.0f, 10.0f));
  addParameter(frequencyParam = new juce::AudioParameterFloat("cut-off frequency (Hz)", "Cut-off frequency", 0.0f, 1.0, 0.0f));
  addParameter(pluckParam = new juce::AudioParameterBool("pluck", "Pluck string", 0));
}

KarplusStrongAudioProcessor::~KarplusStrongAudioProcessor()
{
}

//==============================================================================
const juce::String KarplusStrongAudioProcessor::getName() const
{
  return JucePlugin_Name;
}

bool KarplusStrongAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool KarplusStrongAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool KarplusStrongAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double KarplusStrongAudioProcessor::getTailLengthSeconds() const
{
  return 0.0;
}

int KarplusStrongAudioProcessor::getNumPrograms()
{
  return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
  // so this should be at least 1, even if you're not really implementing programs.
}

int KarplusStrongAudioProcessor::getCurrentProgram()
{
  return 0;
}

void KarplusStrongAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String KarplusStrongAudioProcessor::getProgramName(int index)
{
  return {};
}

void KarplusStrongAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void KarplusStrongAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
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

void KarplusStrongAudioProcessor::releaseResources()
{
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KarplusStrongAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

void KarplusStrongAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
  auto pluck = pluckParam->get();
  if (pluck) pluckParam->setValueNotifyingHost(0);

  // update read position based on current value of delay time
  delayReadPosition = (int)(delayWritePosition - (delayTime * getSampleRate()) + delayBufferLength) % delayBufferLength;

  for (int i = 0; i < numSamples; ++i) {

    // Go through each channel of audio that's passed in. We apply identical effects to each channel.
    // For some effects, like stereo chorus or panner, you might do something different for each channel.
    for (int j = 0; j < numInputChannels; ++j) {
      // get the current input value for this channel, this sample
      const float in = buffer.getWritePointer(j)[i];

      // delayData is the circular buffer for implementing delay on this channel
      float* delayData = delayBuffer.getWritePointer(j);

      // write to the buffer. delayData[delayReadPosition] is delay sample we just read, i.e. what came out of buffer
      delayData[delayWritePosition] = in + delayData[delayReadPosition] * feedback;

      // read from the buffer. Output is input plus contents of delay buffer, weighted by dryWetMix.
      float out = (1 - dryWetMix) * in + dryWetMix * delayData[delayReadPosition];

      // set the current output value for this channel, this sample, replacing input
      buffer.getWritePointer(j)[i] = out;
    }
    //increment the pointers, once per sample (not once per channel!)
    if (++delayReadPosition >= delayBufferLength) delayReadPosition = 0;
    if (++delayWritePosition >= delayBufferLength) delayWritePosition = 0;
  }

  // In case we have more outputs than inputs, clear any output channels that didn't contain input data
  for (int i = numInputChannels; i < numOutputChannels; ++i)
  {
    buffer.clear(i, 0, buffer.getNumSamples());
  }
}

//==============================================================================
bool KarplusStrongAudioProcessor::hasEditor() const
{
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* KarplusStrongAudioProcessor::createEditor()
{
  return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void KarplusStrongAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void KarplusStrongAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  // You should use this method to restore your parameters from this memory block,
  // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new KarplusStrongAudioProcessor();
}
