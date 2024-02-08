/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DistortionAudioProcessor::DistortionAudioProcessor()
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
  addParameter(gainParam = new juce::AudioParameterFloat("gain", "Gain", 0.0f, 30, 0.0f));
  addParameter(distortionTypeParam = new juce::AudioParameterChoice("distortionType", "Distortion type", {
    "HardClipping", "SoftClipping", "SoftClippingExp", "FullWaveRectifier", "HalfWaveRectifier"}, 1));
}

DistortionAudioProcessor::~DistortionAudioProcessor()
{
}

//==============================================================================
const juce::String DistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void DistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Nothing to set here
}

void DistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numOutputChannels = getTotalNumOutputChannels();
    auto numInputChannels = getTotalNumInputChannels();

    float gain = pow(10.0f, gainParam->get() / 20.0f); //linear gain
    int distortionType = distortionTypeParam->getIndex();

    float sampleRate = (float) juce::AudioProcessor::getSampleRate();

    // Clear any output channels that didn't contain input data.
    for (auto i = numOutputChannels; i < numInputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
      float inputSignal = 0.44 * sinf(juce::MathConstants<float>::twoPi * inputPhase) + 0.38 * sinf(juce::MathConstants<float>::twoPi * inputPhase) + 0.3f;
      auto sample = gain * inputSignal;
      switch (distortionType)
      {
        case 0://kHardClipping:
        {
          const float threshold = 0.5f;
          if (sample > threshold) sample = threshold;         // positive clip
          else if (sample < -threshold) sample = -threshold;  // negative clip
          break;
        }
        case 1://kSoftClipping:
        {
          const float threshold1 = 1.0f / 3.0f;
          const float threshold2 = 2.0f / 3.0f;
          if (sample > threshold1)
          {
            if (sample > threshold2) sample = 0.5f;         // positive clip
            else sample = (3.0f - (2.0f - 3.0f * sample) * (2.0f - 3.0f * sample)) / 6.0f;// soft knee (positive)
          }
          else if (sample < -threshold1)
          {
            if (sample < -threshold2) sample = -0.5f;       // negative clip
            else sample = -(3.0f - (2.0f + 3.0f * sample) * (2.0f + 3.0f * sample)) / 6.0f;// soft knee (negative)
          }
          break;
        }
        case 2: //SoftClippingExp:
        {
          if (sample > 0.0f) sample = 1.0f - expf(-sample);// positive
          else sample = -1.0f + expf(sample);// negative
          break;
        }
        case 3: //kFullWaveRectifier:
        {
          sample = fabs(sample);
          break;
        }
        case 4: //kHalfWaveRectifier:
        {
          sample = 0.5f * (fabs(sample) + sample);
          break;
        }
        default:
          break;
      }

      for (int j = 0; j < numInputChannels; ++j) buffer.getWritePointer(j)[i] = sample;
      //for (int j = 0; j < numInputChannels; ++j) buffer.getWritePointer(j)[i] = buffer.getReadPointer(j)[i] * (1.0f - depth * amount);

      // Update input phase
      inputPhase += 1000 / sampleRate;
      while (inputPhase >= 1.0) inputPhase -= 1.0;

    }
}

//==============================================================================
bool DistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DistortionAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void DistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionAudioProcessor();
}
