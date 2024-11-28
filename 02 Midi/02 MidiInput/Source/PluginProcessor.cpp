/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MidiInputAudioProcessor::MidiInputAudioProcessor()
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
}

MidiInputAudioProcessor::~MidiInputAudioProcessor()
{
}

//==============================================================================
const juce::String MidiInputAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MidiInputAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MidiInputAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MidiInputAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MidiInputAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MidiInputAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MidiInputAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MidiInputAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MidiInputAudioProcessor::getProgramName (int index)
{
    return {};
}

void MidiInputAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MidiInputAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MidiInputAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MidiInputAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MidiInputAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) {
  auto numSamples = buffer.getNumSamples();
  // Pass incoming midi messages to keyboard state object, adds messages to buffer if user clicking on on-screen keys
  keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);
  for (const auto metadata : midiMessages) {
    auto message = metadata.getMessage();
    if (message.isNoteOn()) DBG("Note On");
    if (message.isNoteOff()) DBG("Note Off");
    if (message.isNoteOn() || message.isNoteOff()) 
    {
      auto noteNumber = message.getNoteNumber();
      auto noteName = juce::MidiMessage::getMidiNoteName(noteNumber, true, true, 3);
      auto noteInHertz = message.getMidiNoteInHertz(noteNumber);
      auto velocity = message.getVelocity();
      auto channel = message.getChannel();
      DBG("number " << noteNumber);
      DBG("name " << noteName);
      DBG("note In Hertz " << noteInHertz);
      DBG("velocity " << velocity);
      DBG("channel " << channel);
      DBG(message.getDescription());
    }
  }
}

//==============================================================================
bool MidiInputAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MidiInputAudioProcessor::createEditor()
{
    return new MidiInputAudioProcessorEditor(*this);
}

//==============================================================================
void MidiInputAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MidiInputAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidiInputAudioProcessor();
}
