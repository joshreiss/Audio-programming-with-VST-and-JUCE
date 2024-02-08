#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JVerbAudioProcessor::JVerbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
  addParameter(roomSize = new AudioParameterFloat("roomSize", "Room size", 0.0f, 1.0f, 0.5f));
  addParameter(damping = new AudioParameterFloat("damping", "Damping", 0.0f, 1.0f, 0.5f));
  addParameter(wetLevel = new AudioParameterFloat("wetLevel", "Wet level", 0.0f, 1.0f, 0.33f));
  addParameter(dryLevel = new AudioParameterFloat("dryLevel", "Dry level", 0.0f, 1.0f, 0.4f));
  addParameter(width = new AudioParameterFloat("width", "Width", 0.0f, 1.0f, 1.0f));
  addParameter(freezeMode = new AudioParameterBool("freezeMode", "Freeze mode", 0));
}

JVerbAudioProcessor::~JVerbAudioProcessor()
{
}

//==============================================================================
const String JVerbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JVerbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JVerbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JVerbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double JVerbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JVerbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int JVerbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JVerbAudioProcessor::setCurrentProgram (int index)
{
}

const String JVerbAudioProcessor::getProgramName (int index)
{
    return {};
}

void JVerbAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void JVerbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  reverb.setSampleRate(sampleRate);
}

void JVerbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JVerbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void JVerbAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
  ScopedNoDenormals noDenormals;
  auto numChannels = buffer.getNumChannels();
  Reverb::Parameters params = reverb.getParameters();
  params.roomSize = roomSize->get();
  params.damping = damping->get();
  params.wetLevel = wetLevel->get();
  params.dryLevel = dryLevel->get();
  params.width = width->get();
  params.freezeMode = freezeMode->get();
  reverb.setParameters(params);
  if (numChannels == 1) reverb.processMono(buffer.getWritePointer(0), buffer.getNumSamples());
  else reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
  for (int ch = 2; ch < numChannels; ++ch) buffer.clear(ch, 0, buffer.getNumSamples());
}

//==============================================================================
bool JVerbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* JVerbAudioProcessor::createEditor()
{
	return new GenericAudioProcessorEditor(this);
}

//==============================================================================
void JVerbAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void JVerbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JVerbAudioProcessor();
}
