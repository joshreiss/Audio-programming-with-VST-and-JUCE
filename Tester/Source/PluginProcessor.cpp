/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TesterAudioProcessor::TesterAudioProcessor()
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
  addParameter(gainParam = new AudioParameterFloat("GAIN", "Gain", 0.0f, 1.0f, 0.0f));
  addParameter(stimulusParam = new juce::AudioParameterChoice("STIMULUS", "Stimulus", { "Off", "Noise", "Sine wave", "Pulse wave" }, 1));
  addParameter(frequencyParam = new juce::AudioParameterFloat("FREQUENCY", "Frequency", 20.0f, 5000.0f, 1000.f));
  addParameter(lfoFrequencyParam = new juce::AudioParameterFloat("LFOFREQUENCY", "LFO Frequency", 0.1f, 10.0f, 4.f));
  addParameter(choice2Param = new juce::AudioParameterChoice("TWOCHOICE", "Two choice", { "X", "Y" }, 0));
  addParameter(boolParam = new juce::AudioParameterBool("BOOL", "Check box", false));
}

TesterAudioProcessor::~TesterAudioProcessor()
{
}

//==============================================================================
const juce::String TesterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TesterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TesterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TesterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TesterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TesterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TesterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TesterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TesterAudioProcessor::getProgramName (int index)
{
    return {};
}

void TesterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TesterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void TesterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TesterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TesterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  float gainValue = gainParam->get();
  float frequencyValue = frequencyParam->get();
  float lfoFrequencyValue = lfoFrequencyParam->get();
  auto index = stimulusParam->getIndex();
  for (int channel = 0; channel < getTotalNumOutputChannels(); ++channel)
  {
    auto* channelData = buffer.getWritePointer(channel);
    for (int sample = 0; sample < buffer.getNumSamples(); sample++)
    {
      // Update input phase
      if (channel == 0) inputPhase += frequencyValue / getSampleRate();
      while (inputPhase >= 1.0) inputPhase -= 1.0;
      if (channel == 0) lfoPhase += lfoFrequencyValue / getSampleRate();
      while (lfoPhase >= 1.0) lfoPhase -= 1.0;

      if (index == 1) channelData[sample] = 2.0 * gainValue * ((double)rand() / RAND_MAX) - 1.0;
      else if (index == 2) channelData[sample] = gainValue * sinf(juce::MathConstants<float>::twoPi * inputPhase);
      else if (index == 3) {
        // Duration of pulse is duration = maxLFOphase / lfoFreq -> duration = 0.05 * lfoFreq / lfoFreq = 0.05s
        if (lfoPhase < 0.05 * lfoFrequencyValue) channelData[sample] = gainValue * sinf(juce::MathConstants<float>::twoPi * inputPhase);
        else channelData[sample] = 0;
      }
      else channelData[sample] = 0;
    }
  }
}

//==============================================================================
bool TesterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TesterAudioProcessor::createEditor()
{
    return new GenericAudioProcessorEditor(this);
}

//==============================================================================
void TesterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TesterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TesterAudioProcessor();
}
