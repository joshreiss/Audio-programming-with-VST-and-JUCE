/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PingPongDelayAudioProcessor::PingPongDelayAudioProcessor()
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
  addParameter(leftRightDelayTimeParam = new juce::AudioParameterFloat("LRdelayTime", "L-R delay time", 0.0f, 2.0f, 0.5f));
  addParameter(rightLeftDelayTimeParam = new juce::AudioParameterFloat("RLdelayTime", "R-L delay time", 0.0f, 2.0f, 0.5f));
  addParameter(feedbackParam = new juce::AudioParameterFloat("feedback", "Feedback", 0.0f, 0.999f, 0.75f));
  addParameter(dryWetMixParam = new juce::AudioParameterFloat("dryWetMix", "Dry/wet mix", 0.0f, 1.0, 0.5f));
  addParameter(linkDelaysParam = new juce::AudioParameterBool("linkDelays", "Link delays", false));
  addParameter(reverseChannelsParam = new juce::AudioParameterBool("reverseChannels", "Reverse channels", false));
}

PingPongDelayAudioProcessor::~PingPongDelayAudioProcessor()
{
}

//==============================================================================
const juce::String PingPongDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PingPongDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PingPongDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PingPongDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PingPongDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PingPongDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PingPongDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PingPongDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PingPongDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void PingPongDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PingPongDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..


    // Allocate and zero the delay buffer (size will depend on current sample rate)
  delayBufferLength = (int)(2.0 * sampleRate);
  delayBuffer.setSize(2, delayBufferLength);
  delayBuffer.clear();

  // This method gives us the sample rate. Use this to figure out what the delay position
  // offset should be (since it is specified in seconds, and we need to convert it to a number
  // of samples)

  auto leftRightDelayTime = leftRightDelayTimeParam->get();
  auto rightLeftDelayTime = rightLeftDelayTimeParam->get();
  delayReadPositionLeft = (int)(delayWritePosition - (leftRightDelayTime * sampleRate)
    + delayBufferLength) % delayBufferLength;
  delayReadPositionRight = (int)(delayWritePosition - (rightLeftDelayTime * sampleRate)
    + delayBufferLength) % delayBufferLength;
}

void PingPongDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PingPongDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PingPongDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)

{
  juce::ScopedNoDenormals noDenormals;

  // Helpful information about this block of samples:
  const int numInputChannels = getTotalNumInputChannels();    // How many input channels for our effect?
  const int numOutputChannels = getTotalNumOutputChannels();  // How many output channels for our effect?
  const int numSamples = buffer.getNumSamples();              // How many samples in the buffer for this block?
  float sampleRate = (float)juce::AudioProcessor::getSampleRate();
  auto dryWetMix = dryWetMixParam->get();
  auto feedback = feedbackParam->get();
  auto leftRightDelayTime = leftRightDelayTimeParam->get();
  auto rightLeftDelayTime = rightLeftDelayTimeParam->get();
  auto reverseChannels = reverseChannelsParam->get();

  // This effect only makes sense if there are at least 2 channels to work with
  if (buffer.getNumChannels() < 2) return;

  // If there is one input only, the second channel may not contain anything useful.
  // start with a blank buffer in this case
  if (numInputChannels < 2) buffer.clear(1, 0, numSamples);

  // channelDataL and channelDataR are arrays of length numSamples which contain
  // the audio for one channel
  float* channelDataL = buffer.getWritePointer(0);
  float* channelDataR = buffer.getWritePointer(1);

  // delayDataL and delayDataR are the circular buffers for implementing delay
  float* delayDataL = delayBuffer.getWritePointer(0);
  float* delayDataR = delayBuffer.getWritePointer(1);

  for (int i = 0; i < numSamples; ++i)
  {
      //const float inL = channelDataL[i];
      //const float inR = channelDataR[i];

      float inL = sinf(juce::MathConstants<float>::twoPi * inputPhase);
      float inR = sinf(juce::MathConstants<float>::twoPi * inputPhase);
      // Update input phase
        inputPhase += 1000 / sampleRate;
        while (inputPhase >= 1.0) inputPhase -= 1.0;
        phase += 1 / sampleRate;
        while (phase >= 1.0) phase -= 1.0;
        if (phase > 0.05) inL = 0;
        if ((phase > 0.35) || (phase < 0.3)) inR = 0;

      float outL, outR;

      if (reverseChannels)
      {
        outL = (inL + dryWetMix * delayDataR[delayReadPositionLeft]);
        outR = (inR + dryWetMix * delayDataL[delayReadPositionRight]);
      }
      else
      {
        outL = (inL + dryWetMix * delayDataL[delayReadPositionLeft]);
        outR = (inR + dryWetMix * delayDataR[delayReadPositionRight]);
      }

      // Store the output of one delay buffer into the other, producing
      // the ping-pong effect
      delayDataR[delayWritePosition] = inR + (delayDataL[delayReadPositionLeft] * feedback);
      delayDataL[delayWritePosition] = inL + (delayDataR[delayReadPositionRight] * feedback);

      if (++delayReadPositionLeft >= delayBufferLength) delayReadPositionLeft = 0;
      if (++delayReadPositionRight >= delayBufferLength) delayReadPositionRight = 0;
      if (++delayWritePosition >= delayBufferLength) delayWritePosition = 0;

      // Store the output samples in the buffer, replacing the input
      channelDataL[i] = outL;
      channelDataR[i] = outR;
  }

  // In case we have more outputs than inputs, we'll clear any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  for (int i = numInputChannels; i < numOutputChannels; ++i)
  {
    buffer.clear(i, 0, buffer.getNumSamples());
  }
}

//==============================================================================
bool PingPongDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PingPongDelayAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void PingPongDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PingPongDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PingPongDelayAudioProcessor();
}
