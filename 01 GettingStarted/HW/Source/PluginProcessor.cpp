/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloWorld_v2AudioProcessor::HelloWorld_v2AudioProcessor()
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
  addParameter(gainParam = new AudioParameterFloat("gain", "Gain", 0.0f, 1.0f, 0.0f));
}

HelloWorld_v2AudioProcessor::~HelloWorld_v2AudioProcessor()
{
}

//==============================================================================
const juce::String HelloWorld_v2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HelloWorld_v2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HelloWorld_v2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HelloWorld_v2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HelloWorld_v2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HelloWorld_v2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HelloWorld_v2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void HelloWorld_v2AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String HelloWorld_v2AudioProcessor::getProgramName (int index)
{
    return {};
}

void HelloWorld_v2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void HelloWorld_v2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void HelloWorld_v2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HelloWorld_v2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void HelloWorld_v2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  float gainValue = gainParam->get();
  for (int channel = 0; channel < getTotalNumOutputChannels(); ++channel)
  {
    auto* channelData = buffer.getWritePointer(channel);
    for (int sample = 0; sample < buffer.getNumSamples(); sample++)
    {
      channelData[sample] = gainValue * channelData[sample];
    }
  }
}

//==============================================================================
bool HelloWorld_v2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HelloWorld_v2AudioProcessor::createEditor()
{
    return new GenericAudioProcessorEditor(this);
}

//==============================================================================
void HelloWorld_v2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void HelloWorld_v2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HelloWorld_v2AudioProcessor();
}
