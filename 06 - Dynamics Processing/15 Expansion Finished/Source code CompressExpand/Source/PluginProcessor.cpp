/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CompressExpandAudioProcessor::CompressExpandAudioProcessor()
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
  addParameter(modeParam = new juce::AudioParameterChoice("mode", "Mode", { "Compressor / Limiter", "Expander / Noise gate" }, 1));
  addParameter(thresholdParam = new juce::AudioParameterFloat("threshold", "Threshold (dB)", -60.0f, 0.0f, -24.0f));
  addParameter(ratioParam = new juce::AudioParameterFloat("ratio", "Ratio", 1.0f, 100.0, 50.0f));
  addParameter(attackTimeParam = new juce::AudioParameterFloat("attackTime", "Attack time (ms)", 0.1f, 100.0f, 2.0f));
  addParameter(releaseTimeParam= new juce::AudioParameterFloat("releaseTime", "Release time (ms)", 10.0f, 1000.0f, 300.0f));
  addParameter(makeUpGainParam = new juce::AudioParameterFloat("makeUpGain", "Make-up gain (dB)", -12.0f, 12.0f, 0.0f));
  addParameter(bypassParam = new juce::AudioParameterBool("bypass", "Bypass", 0));
}

CompressExpandAudioProcessor::~CompressExpandAudioProcessor()
{
}

//==============================================================================
const juce::String CompressExpandAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CompressExpandAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CompressExpandAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CompressExpandAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CompressExpandAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CompressExpandAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CompressExpandAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CompressExpandAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CompressExpandAudioProcessor::getProgramName (int index)
{
    return {};
}

void CompressExpandAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CompressExpandAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  inputLevel = 0.0f;
  y_l_prev = 0.0f;
  inverseSampleRate = 1.0f / (float)getSampleRate();
  inverseE = 1.0f / juce::MathConstants<float>::euler;
}

void CompressExpandAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CompressExpandAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CompressExpandAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  juce::ScopedNoDenormals noDenormals;
  const int numInputChannels = getTotalNumInputChannels();
  const int numOutputChannels = getTotalNumOutputChannels();
  const int numSamples = buffer.getNumSamples();
  if ((bool) bypassParam->get()) return;
  mixedDownInput.clear();
  for (int channel = 0; channel < numInputChannels; ++channel)
    mixedDownInput.addFrom(0, 0, buffer, channel, 0, numSamples, 1.0f / numInputChannels);
  for (int sample = 0; sample < numSamples; ++sample) {
    bool expander = (bool)modeParam->getIndex();
    float T = thresholdParam->get();
    float R = ratioParam->get();
    float alphaA = calculateAttackOrRelease(attackTimeParam->get()); //check millisecond conversion
    float alphaR = calculateAttackOrRelease(releaseTimeParam->get());//check millisecond conversion
    float makeupGain = makeUpGainParam->get();

    // Level detection- estimate level using peak detector
    float inputSquared = powf(mixedDownInput.getSample(0, sample), 2.0f);
    if (expander) {
      const float averageFactor = 0.9999f;
      inputLevel = averageFactor * inputLevel + (1.0f - averageFactor) * inputSquared;
    }
    else inputLevel = inputSquared;
    x_g = (inputLevel <= 1e-6f) ? -60.0f : 10.0f * log10f(inputLevel);
    // Expander
    if (expander) {

      // Gain computer- static apply input/output curve
      if (x_g > T) y_g = x_g;
      else y_g = T + (x_g - T) * R;
      x_l = x_g - y_g;

      // Ballistics- smoothing of the gain 
      if (x_l < y_l_prev) y_l = alphaA * y_l_prev + (1.0f - alphaA) * x_l;
      else y_l = alphaR * y_l_prev + (1.0f - alphaR) * x_l;

    } else { // Compressor

      // Gain computer- static apply input/output curve
      if (x_g < T) y_g = x_g;
      else y_g = T + (x_g - T) / R;
      x_l = x_g - y_g;

      // Ballistics- smoothing of the gain 
      if (x_l > y_l_prev) y_l = alphaA * y_l_prev + (1.0f - alphaA) * x_l;
      else y_l = alphaR * y_l_prev + (1.0f - alphaR) * x_l;

    }

    // find control
    control = powf(10.0f, (makeupGain - y_l) * 0.05f);
    y_l_prev = y_l;
    for (int channel = 0; channel < numInputChannels; ++channel) {
      float newValue = buffer.getSample(channel, sample) * control;
      buffer.setSample(channel, sample, newValue);
    }
  }
  for (int channel = numInputChannels; channel < numOutputChannels; ++channel) buffer.clear(channel, 0, numSamples);


  /////////////////////////////////////////////////////////////////
//      float inputSignal = sinf(juce::MathConstants<float>::twoPi * inputPhase);
      // Update input phase
//      inputPhase += 1000 / sampleRate;
//      while (inputPhase >= 1.0) inputPhase -= 1.0;
      //////////////////////////////////
}

float CompressExpandAudioProcessor::calculateAttackOrRelease(float value)
{
  if (value == 0.0f) return 0.0f;
  else return pow(inverseE, inverseSampleRate / value);
}


//==============================================================================
bool CompressExpandAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CompressExpandAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void CompressExpandAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CompressExpandAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressExpandAudioProcessor();
}

