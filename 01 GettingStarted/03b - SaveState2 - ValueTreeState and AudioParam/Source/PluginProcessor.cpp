/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SaveState2AudioProcessor::SaveState2AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
   : AudioProcessor (BusesProperties()
       #if ! JucePlugin_IsMidiEffect
         #if ! JucePlugin_IsSynth
           .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
         #endif
           .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
       #endif
      )
    , state(*this, nullptr, Identifier("params"), { 
      std::make_unique<AudioParameterFloat>(
        "gain",
        "Gain",
        NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f)
    })

#endif
{
}

SaveState2AudioProcessor::~SaveState2AudioProcessor()
{
}

//==============================================================================
const juce::String SaveState2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SaveState2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SaveState2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SaveState2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SaveState2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SaveState2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SaveState2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void SaveState2AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SaveState2AudioProcessor::getProgramName (int index)
{
    return {};
}

void SaveState2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SaveState2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
  gainParam = state.getRawParameterValue("gain");
}

void SaveState2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SaveState2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SaveState2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    for (int i = 0; i < buffer.getNumSamples(); i++) {
      for (int channel = 0; channel < getTotalNumOutputChannels(); channel++) {
        auto* data = buffer.getWritePointer(channel);
        data[i] = (2.0 * (float)rand()/RAND_MAX -1.0) * *gainParam;
      }
    }
}

//==============================================================================
bool SaveState2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SaveState2AudioProcessor::createEditor()
{
    return new SaveState2AudioProcessorEditor (*this, state);
}

//==============================================================================
void SaveState2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto stateTree = state.copyState();
    std::unique_ptr<XmlElement> xml(stateTree.createXml());
    copyXmlToBinary(*xml, destData);
}

void SaveState2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(state.state.getType()))
      state.replaceState(ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SaveState2AudioProcessor();
}
