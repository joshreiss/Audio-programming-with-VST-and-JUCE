/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FMSynthesisAudioProcessor::FMSynthesisAudioProcessor()
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
  addParameter(modulationDepthParam = new juce::AudioParameterFloat("modulationDepth", "Modulation depth", 0.0f, 1000.0f, 200.0f));
  addParameter(modulatorFrequencyParam = new juce::AudioParameterFloat("modulatorFrequency", "Modulator frequency", 10.0f, 200.0f, 100.0f));
  addParameter(carrierFrequencyParam = new juce::AudioParameterFloat("carrierFrequency", "Carrier frequency", 50.0f, 2000.0f, 800.0f));
}

FMSynthesisAudioProcessor::~FMSynthesisAudioProcessor()
{
}

//==============================================================================
const juce::String FMSynthesisAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FMSynthesisAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FMSynthesisAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FMSynthesisAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FMSynthesisAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FMSynthesisAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FMSynthesisAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FMSynthesisAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FMSynthesisAudioProcessor::getProgramName (int index)
{
    return {};
}

void FMSynthesisAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FMSynthesisAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void FMSynthesisAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FMSynthesisAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FMSynthesisAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numInputChannels  = getTotalNumInputChannels();
    auto numOutputChannels = getTotalNumOutputChannels();

    float modulationDepth = modulationDepthParam->get();
    float modulatorFrequency = modulatorFrequencyParam->get();
    float carrierFrequency = carrierFrequencyParam->get();
    float sampleRate = (float) juce::AudioProcessor::getSampleRate();

    for (auto i = numInputChannels; i < numOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
      modulatorSignal = modulationDepth * sinf(juce::MathConstants<float>::twoPi * modulatorPhase);
      modulatedCarrierSignal = sinf(juce::MathConstants<float>::twoPi * modulatedCarrierPhase);
      for (int j = 0; j < numOutputChannels; ++j) buffer.getWritePointer(j)[i] = modulatedCarrierSignal;

      modulatorPhase += modulatorFrequency / sampleRate;
      while (modulatorPhase >= 1.0) modulatorPhase -= 1.0;
      modulatedCarrierPhase += (carrierFrequency + modulatorSignal) / sampleRate;
      while (modulatedCarrierPhase >= 1.0) modulatedCarrierPhase -= 1.0;
    }
}

//==============================================================================
bool FMSynthesisAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FMSynthesisAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void FMSynthesisAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FMSynthesisAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FMSynthesisAudioProcessor();
}


// Function for calculating "biased" LFO waveforms with output range [0, 1].
// Phase range [0, 1], output also [0, 1] (not [-1, +1] as for the ordinary Sine function).
float FMSynthesisAudioProcessor::getLFOSample(float phase, int waveform)
{
  switch (waveform)
  {
  case 0: //Triangle:
    if (phase < 0.25f) return 0.5f + 2.0f * phase;
    else if (phase < 0.75f) return 1.0f - 2.0f * (phase - 0.25f);
    else return 2.0f * (phase - 0.75f);
  case 1: //Square:
    if (phase < 0.5f) return 1.0f;
    else return 0.0f;
  case 2: //SquareSlopedEdges:
    if (phase < 0.48f) return 1.0f;
    else if (phase < 0.5f) return 1.0f - 50.0f * (phase - 0.48f);
    else if (phase < 0.98f) return 0.0f;
    else return 50.0f * (phase - 0.98f);
  case 3: //Sine:
  default:
    return 0.5f + 0.5f * sinf(juce::MathConstants<float>::twoPi * phase);
  }
}
