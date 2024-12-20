/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChorusAudioProcessor::ChorusAudioProcessor()
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

  addParameter(minimumDelayParam = new juce::AudioParameterFloat("minimumDelay", "Minimum delay", { 10.0f, 50.0f, 0.1f }, 30.0f, "ms"));
  addParameter(sweepWidthParam = new juce::AudioParameterFloat("sweepWidth", "Sweep width", { 10.0f, 50.0f, 0.1f }, 20.0f, "ms"));
  addParameter(depthParam = new juce::AudioParameterFloat("depth", "Depth", { 0.0f, 100.0f, 1.0f }, 100.0f, "%"));
  addParameter(voiceCountParam = new juce::AudioParameterInt("voiceCount", "Number of voices", 2, 5, 3));
  addParameter(lfoFrequencyParam = new juce::AudioParameterFloat("lfoFrequency", "LFO frequency", { 0.05f, 2.0f, 0.01f }, 0.2f, "Hz"));
  addParameter(lfoTypeParam = new juce::AudioParameterChoice("lfoType", "LFO Type", { "triangle", "square", "sloped square", "sine" }, 1));
  addParameter(interpolationTypeParam = new juce::AudioParameterChoice("interpolationType", "Interpolation Type",
    { "Nearest neighbour", "Linear", "Quadratic" }, 1));
  addParameter(stereoParam = new juce::AudioParameterBool("stereo", "Stereo", false));
}

ChorusAudioProcessor::~ChorusAudioProcessor()
{
}

//==============================================================================
const juce::String ChorusAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChorusAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChorusAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChorusAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChorusAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChorusAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ChorusAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChorusAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ChorusAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChorusAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ChorusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // Allocate and zero the delay buffer (size will depend on current sample rate)
    // Add extra samples to allow interpolation even at maximum delay
  double maxDelaySec = 0.001 * (minimumDelayParam->getNormalisableRange().end + sweepWidthParam->getNormalisableRange().end);
  delayBufferLength = (int)(maxDelaySec * sampleRate) + 3;
  delayBuffer.setSize(2, delayBufferLength);
  delayBuffer.clear();
  lfoPhase = 0.0f;

  inverseSampleRate = 1.0 / sampleRate;
}

void ChorusAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChorusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ChorusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)

{
  juce::ScopedNoDenormals noDenormals;

  // Helpful information about this block of samples:
  const int numInputChannels = getTotalNumInputChannels();    // How many input channels for our effect?
  const int numOutputChannels = getTotalNumOutputChannels();  // How many output channels for our effect?
  const int numSamples = buffer.getNumSamples();              // How many samples in the buffer for this block?
  float sampleRate = (float)juce::AudioProcessor::getSampleRate();
  auto lfoFrequency = lfoFrequencyParam->get();
  auto depth = depthParam->get();
  auto voiceCount = voiceCountParam->get();
  auto minimumDelay = minimumDelayParam->get();
  auto sweepWidth = sweepWidthParam->get();
  auto lfoType = lfoTypeParam->getIndex();
  auto interpolationType = interpolationTypeParam->getIndex();
  auto stereo = stereoParam->get();

  // working variables
  int dpw = delayWritePosition;   // delay write position (integer)
  float dpr;                      // delay read position (real-valued)
  float currentDelay;
  float ph = lfoPhase;

  // Iterate over each input audio channel. Here, we apply identical effects to each
  // channel, but for some effects you might do something different for each channel.
  int channel;
  for (channel = 0; channel < numInputChannels; ++channel)
  {
    // channelData is an array of length numSamples which contains the audio for one channel
    float* channelData = buffer.getWritePointer(channel);

    // delayData is the circular buffer for implementing delay on this channel
    float* delayData = delayBuffer.getWritePointer(juce::jmin(channel, delayBuffer.getNumChannels() - 1));

    // Make temporary copy of state variables that need to be maintained between calls to processBlock(),
    // so that processing one channel can't affect state variable for next channel.
    dpw = delayWritePosition;
    ph = lfoPhase;

    for (int i = 0; i < numSamples; ++i)
    {
      // Update input phase
      if (channel == 0) {
        inputPhase += 1000 / sampleRate;
        while (inputPhase >= 1.0) inputPhase -= 1.0;
      }
      //const float in = channelData[i];
      float in = sinf(juce::MathConstants<float>::twoPi * inputPhase);
      channelData[i] = in;
      float interpolatedSample = 0.0f;
      float phaseOffset = 0.0f;
      float weight;

      // Chorus can have many voices (original, undelayed signal counts as voice). Here, all voices use same LFO, with
      // different phase offsets. It's also possible to use different waveforms and different frequencies for each voice.

      for (int j = 0; j < voiceCount - 1; ++j)
      {
        if (stereo && (voiceCount > 2))
        {
          // Stereo chorus pans each voice to different location in stereo field, depending on the number of voices:
          // -- 2 voices: N/A (need at least 2 delayed voices for stereo chorus)
          // -- 3 voices: 1 voice left, 1 voice right (0, 1)
          // -- 4 voices: 1 voice left, 1 voice centre, 1 voice right (0, 0.5, 1)
          // -- 5 voices: 1 voice left, 1 voice left-centre,
          //              1 voice right-centre, 1 voice right (0, 0.33, 0.66, 1)
          weight = float(j) / float(voiceCount - 2);

          // Left and right channels are mirrors of each other in weight
          if ((channel % 2) != 0) weight = 1.0f - weight;
        }
        else weight = 1.0f;

        // Add the voice to the mix if it has nonzero weight
        if (weight != 0.0f)
        {
          float lfoSample = getLfoSample(fmodf(ph + phaseOffset, lfoType), lfoType);
          currentDelay = 0.001f * (minimumDelay + lfoSample * sweepWidth );
          dpr = fmodf(float(dpw) - float(currentDelay * getSampleRate()) + float(delayBufferLength), float(delayBufferLength));

          // In this example, the output is the input plus the contents of the delay buffer (weighted by delayMix)
          // The last term implements a tremolo (variable amplitude) on the whole thing.
          interpolatedSample = interpolateSample(interpolationType, dpr, delayData, delayBufferLength);

          // Store the output sample in the buffer, which starts by containing the input sample
          channelData[i] += depth * weight * interpolatedSample;
        }

        // 3-voice chorus uses two voices in quadrature phase (90 degrees apart). Otherwise,
        // spread the voice phases evenly around the unit circle. (For 2-voice chorus, this
        // code doesn't matter since the loop only runs once.)
        if (voiceCount < 3) phaseOffset += 0.25f;
        else phaseOffset += 1.0f / (float)(voiceCount - 1);
      }

      // Store current input in delay buffer (no feedback in chorus, unlike flanger).
      delayData[dpw] = in;

      // Increment write pointer at constant rate. Read pointer moves at different
      // rates depending on the settings of the LFO, the delay and the sweep width.
      if (++dpw >= delayBufferLength) dpw = 0;

      // Update the LFO phase, keeping it in the range 0-1
      ph += float(lfoFrequency * inverseSampleRate);
      while (ph >= 1.0f) ph -= 1.0f;
    }
  }

  // Having made a local copy of the state variables for each channel, now transfer the result
  // back to the main state variable so they will be preserved for the next call of processBlock()
  delayWritePosition = dpw;
  lfoPhase = ph;

  // Clear any output channels that didn't contain input data. They may contain garbage.
  for (int i = numInputChannels; i < numOutputChannels; ++i) buffer.clear(i, 0, buffer.getNumSamples());
}

//==============================================================================
bool ChorusAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ChorusAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void ChorusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ChorusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

// Function for calculating "biased" LFO waveforms with output range [0, 1].
// Phase range [0, 1], output also [0, 1] (not [-1, +1] as for the ordinary Sine function).
float ChorusAudioProcessor::getLfoSample(float phase, int waveform)
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
    case 1://Sawtooth:
        if (phase < 0.5f)
            return 0.5f + phase;
        else
            return phase - 0.5f;
    case 2://InverseSawtooth:
        if (phase < 0.5f)
            return 0.5f - phase;
        else
            return 1.5f - phase;
    case 3://Sine:
    default:
        return 0.5f + 0.5f*sinf(juce::MathConstants<float>::twoPi * phase);
    }
}

float ChorusAudioProcessor::interpolateSample(int type, float delayReadPosition, float* delayData, int delayBufferLength)
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
    default:
      // This line would only be reached if the type argument is invalid
      return 0.0f;
  }
}
// Called whenever LFO waveform, LFO frequency, or sweep width parameters are changed.
void ChorusAudioProcessor::parameterChanged()
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
    return new ChorusAudioProcessor();
}
