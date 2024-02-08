/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PanningAudioProcessor::PanningAudioProcessor()
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
  addParameter(methodParam = new juce::AudioParameterChoice("Method", "Method", { "Panorama + Precedence", "ITD + ILD"}, 0));
  addParameter(panningParam = new juce::AudioParameterFloat("Panning", "Panning", -1.0f, 1.0, 0.0f));
}

PanningAudioProcessor::~PanningAudioProcessor()
{
}

//==============================================================================
const juce::String PanningAudioProcessor::getName() const
{
  return JucePlugin_Name;
}

bool PanningAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool PanningAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool PanningAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double PanningAudioProcessor::getTailLengthSeconds() const
{
  return 0.0;
}

int PanningAudioProcessor::getNumPrograms()
{
  return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
  // so this should be at least 1, even if you're not really implementing programs.
}

int PanningAudioProcessor::getCurrentProgram()
{
  return 0;
}

void PanningAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String PanningAudioProcessor::getProgramName(int index)
{
  return {};
}

void PanningAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void PanningAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  const double smoothTime = 1e-3;
  maximumDelayInSamples = (int)(1e-3f * (float)getSampleRate());
  delayLineL.setup(maximumDelayInSamples);
  delayLineR.setup(maximumDelayInSamples);
}

void PanningAudioProcessor::releaseResources()
{
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PanningAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

void PanningAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  juce::ScopedNoDenormals noDenormals;

  // Helpful information about this block of samples:
  const int numInputChannels = getTotalNumInputChannels();    // How many input channels for our effect?
  const int numOutputChannels = getTotalNumOutputChannels();  // How many output channels for our effect?
  const int numSamples = buffer.getNumSamples();              // How many samples in the buffer for this block?
  float sampleRate = (float)juce::AudioProcessor::getSampleRate();
  auto method = methodParam->getIndex();
  auto panning = panningParam->get();
  float* channelDataL = buffer.getWritePointer(0);
  float* channelDataR = buffer.getWritePointer(1);
  switch (method) {
    case 0: {
      // Panorama
      float theta = 30.0f * juce::MathConstants<float>::pi / 180.0f;
      float phi = -panning * theta;
      float cos_theta = cosf(theta);
      float cos_phi = cosf(phi);
      float sin_theta = sinf(theta);
      float sin_phi = sinf(phi);
      float gainL = (cos_phi * sin_theta + sin_phi * cos_theta);
      float gainR = (cos_phi * sin_theta - sin_phi * cos_theta);
      float norm = 1.0f / sqrtf(gainL * gainL + gainR * gainR);

      // Precedence
      float delayFactor = (panning + 1.0f) / 2.0f;
      float delayTimeL = (float)maximumDelayInSamples * (delayFactor);
      float delayTimeR = (float)maximumDelayInSamples * (1.0f - delayFactor);
      for (int sample = 0; sample < numSamples; ++sample) {
        const float in = channelDataL[sample];
        delayLineL.writeSample(in);
        delayLineR.writeSample(in);
        channelDataL[sample] = delayLineL.readSample(delayTimeL) * gainL * norm;
        channelDataR[sample] = delayLineR.readSample(delayTimeR) * gainR * norm;
      }
      break;
    }
    case 1: {
      float headRadius = 8.5e-2f;
      float speedOfSound = 340.0f;
      float headFactor = (float)getSampleRate() * headRadius / speedOfSound;

      // Interaural Time Difference (ITD)
      auto Td = [headFactor](const float angle) {
        if (abs(angle) < juce::MathConstants<float>::halfPi)
          return headFactor * (1.0f - cosf(angle));
        else
          return headFactor * (abs(angle) + 1.0f - juce::MathConstants<float>::halfPi);
      };
      float theta = 90.0f * juce::MathConstants<float>::pi / 180.0f;
      float phi = panning * theta;
      float currentDelayTimeL = Td(phi + juce::MathConstants<float>::halfPi);
      float currentDelayTimeR = Td(phi - juce::MathConstants<float>::halfPi);
      for (int sample = 0; sample < numSamples; ++sample) {
        const float in = channelDataL[sample];
        delayLineL.writeSample(in);
        delayLineR.writeSample(in);
        channelDataL[sample] = delayLineL.readSample(currentDelayTimeL);
        channelDataR[sample] = delayLineR.readSample(currentDelayTimeR);
      }

      // Interaural Level Difference (ILD)
      filterL.updateCoefficients((double)phi + juce::MathConstants<float>::halfPi, (double)(headRadius / speedOfSound));
      filterR.updateCoefficients((double)phi - juce::MathConstants<float>::halfPi, (double)(headRadius / speedOfSound));
      filterL.processSamples(channelDataL, numSamples);
      filterR.processSamples(channelDataR, numSamples);
      break;
    }
  }

  // In case we have more outputs than inputs, clear any output channels that didn't contain input data
  for (int i = numInputChannels; i < numOutputChannels; ++i)
  {
    buffer.clear(i, 0, buffer.getNumSamples());
  }
}

//==============================================================================
bool PanningAudioProcessor::hasEditor() const
{
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PanningAudioProcessor::createEditor()
{
  return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void PanningAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void PanningAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  // You should use this method to restore your parameters from this memory block,
  // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new PanningAudioProcessor();
}
