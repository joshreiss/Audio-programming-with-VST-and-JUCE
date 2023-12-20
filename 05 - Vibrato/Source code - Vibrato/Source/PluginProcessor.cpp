/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VibratoAudioProcessor::VibratoAudioProcessor()
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
  addParameter(lfoFrequencyParam = new juce::AudioParameterFloat("lfoFrequency", "LFO frequency",0.1f,5.0f, 1.0f));
  addParameter(sweepWidthParam = new juce::AudioParameterFloat("sweepWidth", "Sweep width", 0.001f, 0.05f, 0.01f));
  addParameter(lfoTypeParam = new juce::AudioParameterChoice("lfoType", "LFO Type", { "triangle", "square", "sawtooth", "inverse sawtooth", "sine" }, 1));
  addParameter(interpolationTypeParam = new juce::AudioParameterChoice("interpolationType", "Interpolation Type",
    { "Nearest neighbour", "Linear", "Quadratic","Cubic" }, 1));
}

VibratoAudioProcessor::~VibratoAudioProcessor()
{
}

//==============================================================================
const juce::String VibratoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VibratoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VibratoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VibratoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VibratoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VibratoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VibratoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VibratoAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VibratoAudioProcessor::getProgramName (int index)
{
    return {};
}

void VibratoAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VibratoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  // Allocate and zero the delay buffer (size will depend on current sample rate)
  // Add 3 extra samples to allow cubic interpolation even at maximum delay
  float sweepWidthMax = sweepWidthParam->getNormalisableRange().end;
  delayBufferLength = (int)(sweepWidthMax * sampleRate) + 3;
  delayBuffer.setSize(2, delayBufferLength);
  delayBuffer.clear();
  lfoPhase = 0.0f;
}

void VibratoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VibratoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void VibratoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)

{
  juce::ScopedNoDenormals noDenormals;

  // Helpful information about this block of samples:
  const int numInputChannels = getTotalNumInputChannels();    // How many input channels for our effect?
  const int numOutputChannels = getTotalNumOutputChannels();  // How many output channels for our effect?
  const int numSamples = buffer.getNumSamples();              // How many samples in the buffer for this block?
  float sampleRate = (float)juce::AudioProcessor::getSampleRate();
  auto lfoFrequency = lfoFrequencyParam->get();
  auto sweepWidth = sweepWidthParam->get();
  auto lfoType = lfoTypeParam->getIndex();
  auto interpolationType = interpolationTypeParam->getIndex();

  // working variables
  float currentDelay;

  // Iterate over each (multichannel) audio sample in the block.
  for (int i = 0; i < numSamples; ++i) {
    // Recalculate read pointer position with respect to write pointer.
    // First calculate current delay from vibrato parameters and lfo phase
    currentDelay = sweepWidth * getLfoSample(lfoPhase, lfoType);
    // Now use that to find read pointer position
    // Subtract 3 samples to delay pointer to make sure we have enough previous samples for interpolation
    delayReadPosition = fmodf(float(delayWritePosition - (currentDelay * getSampleRate()) + delayBufferLength - 3.0), 
                              float(delayBufferLength));

    // Iterate over each input audio channel. We apply identical effects to each channel.
    for (int j = 0; j < numInputChannels; ++j) {
      const float in = buffer.getWritePointer(j)[i];
      // delayData is the circular buffer for implementing delay on this channel
      float* delayData = delayBuffer.getWritePointer(j);
      
      // implement fractional delay
      interpolatedSample = interpolateSample(interpolationType, delayReadPosition, delayData, delayBufferLength);

      // Store the current information in the delay buffer. With feedback, what we read is included in what 
      // gets stored in the buffer, otherwise it's just a simple delay line of the input signal.
      delayData[delayWritePosition] = in;

      // set the current output value for this channel, this sample, replacing input. In vibrato,
      // delayed sample is the only component of the output (no mixing with the dry signal)
      buffer.getWritePointer(j)[i] = interpolatedSample;
    }
    // Increment write pointer at constant rate. Read pointer rate depends on LFO settings, delay and sweep width.
    if (++delayWritePosition >= delayBufferLength) delayWritePosition = 0;

    // Update the LFO phase, keeping it in the range 0-1
    lfoPhase += float(lfoFrequency / sampleRate);
    while (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
  }
  
  // Clear any output channels that didn't contain input data. They may contain garbage.
  for (int i = numInputChannels; i < numOutputChannels; ++i) buffer.clear(i, 0, buffer.getNumSamples());
}

//==============================================================================
bool VibratoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VibratoAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void VibratoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VibratoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

// Function for calculating "biased" LFO waveforms with output range [0, 1].
// Phase range [0, 1], output also [0, 1] (not [-1, +1] as for the ordinary Sine function).
float VibratoAudioProcessor::getLfoSample(float phase, int waveform)
{
    switch (waveform)
    {
    case 0://Triangle:
        if (phase < 0.25f)
            return 0.5f + 2.0f*phase;
        else if (phase < 0.75f)
            return 1.0f - 2.0f*(phase - 0.25f);
        else
            return 2.0f*(phase - 0.75f);
    case 1://square
      if (phase < 0.5f)
        return 1.0f;
      else
        return 0.0f;
    case 2://Sawtooth:
        if (phase < 0.5f)
            return 0.5f + phase;
        else
            return phase - 0.5f;
    case 3://InverseSawtooth:
        if (phase < 0.5f)
            return 0.5f - phase;
        else
            return 1.5f - phase;
    case 4://Sine:
    default:
        return 0.5f + 0.5f*sinf(juce::MathConstants<float>::twoPi * phase);
    }
}

float VibratoAudioProcessor::interpolateSample(int type, float delayReadPosition, float* delayData, int delayBufferLength)
{
  switch (type)
  {
    case 0: //NearestNeighbour:
    {
      // Find the nearest input sample by rounding the fractional index to the
      // nearest integer. It's possible this will round it to the end of the buffer,
      // in which case we need to roll it back to the beginning.
      int closestSampleIndex = (int)floorf(delayReadPosition + 0.5f);
      if (closestSampleIndex == delayBufferLength) closestSampleIndex = 0;
      return delayData[closestSampleIndex];
    }
    case 1: // linear
    {
      // Find the fraction by which the read pointer sits between two
      // samples and use this to adjust weights of the samples
      float fraction = delayReadPosition - floorf(delayReadPosition);
      int previousSample = (int)floorf(delayReadPosition);
      int nextSample = (previousSample + 1) % delayBufferLength;
      return fraction * delayData[nextSample] + (1.0f - fraction) * delayData[previousSample];
    }
    case 2: //quadratic
    {
      // Find the fraction by which the read pointer sits between three
      // samples and use this to adjust weights of the samples
      int sample1 = (int)floorf(delayReadPosition + 0.5) % delayBufferLength;
      int sample2 = (sample1 + 1) % delayBufferLength;
      int sample0 = (sample1 - 1) % delayBufferLength;
      float fraction = delayReadPosition - sample1;

      return (fraction * (fraction - 1) * delayData[sample0]
           - 2 * (fraction - 1) * (fraction + 1) * delayData[sample1]
           + fraction * (fraction + 1) * delayData[sample2]) / 2.0f;
    }
    case 3: // Cubic:
    {
      // Cubic interpolation will produce cleaner results at expense of more computation. 
      // This code uses the Catmull-Rom variant of cubic interpolation. To reduce the load, 
      // calculate some quantities in advance that will be used several times in the equation:
      int sample1 = (int)floorf(delayReadPosition);
      int sample2 = (sample1 + 1) % delayBufferLength;
      int sample3 = (sample2 + 1) % delayBufferLength;
      int sample0 = (sample1 - 1 + delayBufferLength) % delayBufferLength;

      float fraction = delayReadPosition - floorf(delayReadPosition);
      float frsq = fraction * fraction;

      float a0 = -0.5f * delayData[sample0] + 1.5f * delayData[sample1]
        - 1.5f * delayData[sample2] + 0.5f * delayData[sample3];
      float a1 = delayData[sample0] - 2.5f * delayData[sample1]
        + 2.0f * delayData[sample2] - 0.5f * delayData[sample3];
      float a2 = -0.5f * delayData[sample0] + 0.5f * delayData[sample2];
      float a3 = delayData[sample1];

      return a0 * fraction * frsq + a1 * frsq + a2 * fraction + a3;
    }
    default:
      // This line would only be reached if the type argument is invalid
      return 0.0f;
  }
}
// Called whenever LFO waveform, LFO frequency, or sweep width parameters are changed.
void VibratoAudioProcessor::parameterChanged()
{
  // The amount of pitch shift depends on the derivative of the delay, which
  // is given by: delay = width * f(frequency * t)
  // where f(x) is one of:
  //   sine --> 0.5 + 0.5*sin(2*pi*x) --> derivative pi*cos(x)*dx
  //   triangle --> {2.0*x or 1.0-(2.0*(x-0.5)) ---> derivative +/- 2.0*dx
  //   sawtooth rising --> x --> derivative 1.0*dx
  //   sawtooth falling --> 1.0 - x --> derivative -1.0*dx
  // For f(frequency*t), "dx" = frequency

  float maxSpeed = 1.0, minSpeed = 1.0;
  float maxPitch = 0.0, minPitch = 0.0;

  float lfoFreqTimesSweep = lfoFrequencyParam->get() * sweepWidthParam->get();
  const float FLT_PI = 3.1415926f;
  switch (lfoTypeParam->getIndex())
  {
  case 0: //Sine:
    maxSpeed = 1.0f + FLT_PI * lfoFreqTimesSweep;
    minSpeed = 1.0f - FLT_PI * lfoFreqTimesSweep;
    break;
  case 1: //Triangle:
    maxSpeed = 1.0f + 2.0f * lfoFreqTimesSweep;
    minSpeed = 1.0f - 2.0f * lfoFreqTimesSweep;
    break;
  case 2: //Sawtooth:
    // Standard (rising) sawtooth means delay is increasing --> pitch is lower
    maxSpeed = 1.0f;
    minSpeed = 1.0f - lfoFreqTimesSweep;
    break;
  case 3: //InverseSawtooth:
    // Inverse (falling) sawtooth means delay is decreasing --> pitch is higher
    maxSpeed = 1.0f + lfoFreqTimesSweep;
    minSpeed = 1.0f;
    break;
  }
  // Convert speed to pitch shift --> semitones = 12*log2(speed)
  maxPitch = 12.0f * logf(maxSpeed) / logf(2.0f);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VibratoAudioProcessor();
}