/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
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
  addParameter(delayTimeParam = new juce::AudioParameterFloat("delayTime", "Delay time", 0.0f, 2.0f, 0.0f));
  addParameter(feedbackParam = new juce::AudioParameterFloat("feedback", "Feedback", 0.0f, 0.999f, 0.0f));
  addParameter(dryWetMixParam = new juce::AudioParameterFloat("dryWetMix", "Dry/wet mix", 0.0f, 1.0, 0.0f));
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // Allocate and zero the delay buffer (size will depend on current sample rate)
    // Sanity check the result so we don't end up with any zero-length calculations
  auto delayTime = delayTimeParam->get();
  delayBufferLength = (int)(2.0 * sampleRate);
  if (delayBufferLength < 1) delayBufferLength = 1;
  delayBuffer.setSize(2, delayBufferLength);
  delayBuffer.clear();

  // This method gives us the sample rate. Use this to figure out what the delay position
  // offset should be (since it is specified in seconds, and we need to convert it to a number
  // of samples)
  delayReadPosition = (int)(delayWritePosition - (delayTime * getSampleRate())
    + delayBufferLength) % delayBufferLength;
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)

{
  juce::ScopedNoDenormals noDenormals;

  // Helpful information about this block of samples:
  const int numInputChannels = getTotalNumInputChannels();    // How many input channels for our effect?
  const int numOutputChannels = getTotalNumOutputChannels();  // How many output channels for our effect?
  const int numSamples = buffer.getNumSamples();              // How many samples in the buffer for this block?
  float sampleRate = (float)juce::AudioProcessor::getSampleRate();
  auto dryWetMix = dryWetMixParam->get();
  auto feedback = feedbackParam->get();
  auto delayTime = delayTimeParam->get();
  int channel, dpr, dpw; // dpr = delay read pointer; dpw = delay write pointer
  delayReadPosition = (int)(delayWritePosition - (delayTime * getSampleRate())
    + delayBufferLength) % delayBufferLength;

  // Go through each channel of audio that's passed in. We apply identical effects to each channel. 
  // For some effects, like stereo chorus or panner, you might do something different for each channel.
  for (channel = 0; channel < numInputChannels; ++channel)
  {
    // channelData is an array of length numSamples which contains the audio for one channel
    float* channelData = buffer.getWritePointer(channel);

    // delayData is the circular buffer for implementing delay on this channel
    float* delayData = delayBuffer.getWritePointer(juce::jmin(channel, delayBuffer.getNumChannels() - 1));

    // Make temporary copy of state variables that need to be maintained between calls to processBlock(). Each 
    // channel needs to be processed identically, processing one channel can't affect state variable for next channel.
    dpr = delayReadPosition;
    dpw = delayWritePosition;

    for (int i = 0; i < numSamples; ++i)
    {
      // const float in = channelData[i];
      float in = sinf(juce::MathConstants<float>::twoPi * inputPhase);
      // Update input phase
      if (channel == 0) {
        inputPhase += 1000 / sampleRate;
        while (inputPhase >= 1.0) inputPhase -= 1.0;
        if (channel == 0) phase += 1 / sampleRate;
        while (phase >= 1.0) phase -= 1.0;
        if (phase > 0.05) in = 0;
      }

      float out = 0.0;
      // In this example, output is input plus contents of delay buffer (weighted by delayMix)
      // The last term implements a tremolo (variable amplitude) on the whole thing.

      out = (1- dryWetMix) * in + dryWetMix * delayData[dpr];

      // Store the current information in the delay buffer. delayData[dpr] is the delay sample we just read,
      // i.e. what came out of the buffer. delayData[dpw] is what we write to the buffer, i.e. what goes in

      delayData[dpw] = in + (delayData[dpr] * feedback);

      if (++dpr >= delayBufferLength) dpr = 0;
      if (++dpw >= delayBufferLength) dpw = 0;

      // Store the output sample in the buffer, replacing the input
      channelData[i] = out;
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
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}

