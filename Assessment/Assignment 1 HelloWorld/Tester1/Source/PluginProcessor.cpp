/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Tester1AudioProcessor::Tester1AudioProcessor()
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
  addParameter(testSignalParam = new  juce::AudioParameterChoice("testSignal", "Test Signal", { "Noise", "Sinusoid", "Pulse wave"}, 0));
  addParameter(gainParam = new juce::AudioParameterFloat("gain", "Gain", 0.0f, 1.0f, 0.0f));
  addParameter(frequencyParam = new juce::AudioParameterFloat("frequency", "Frequency", 20.0f, 2000.0f, 1000.0f));
  addParameter(lfoFrequencyParam = new juce::AudioParameterFloat("lfoFrequency", "LFO frequency", 0.2f, 5.0f, 1.0f));
  addParameter(channelParam = new  juce::AudioParameterChoice("channel", "Channel", { "Left", "Centre", "Right" }, 1));
}

Tester1AudioProcessor::~Tester1AudioProcessor()
{
}

//==============================================================================
const juce::String Tester1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Tester1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Tester1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Tester1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Tester1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Tester1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Tester1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Tester1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Tester1AudioProcessor::getProgramName (int index)
{
    return {};
}

void Tester1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Tester1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Tester1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Tester1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Tester1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    testSignal = testSignalParam->getIndex();
    gainValue = gainParam->get();
    frequencyValue = frequencyParam->get();
    lfoFrequencyValue = lfoFrequencyParam->get();
    channelValue = channelParam->getIndex();
    for (int sample = 0; sample < buffer.getNumSamples(); sample++)
    {
      // Update input phases
      inputPhase += frequencyValue / getSampleRate();
      while (inputPhase >= 1.0) inputPhase -= 1.0;
      lfoPhase += lfoFrequencyValue / getSampleRate();
      while (lfoPhase >= 1.0) lfoPhase -= 1.0;
      for (int channel = 0; channel < getTotalNumOutputChannels(); ++channel)
      {
        auto* channelData = buffer.getWritePointer(channel);
        if (testSignal == 0) channelData[sample] = 2.0 * gainValue * ((double)rand() / RAND_MAX) - 1.0;
        else if (testSignal == 1) channelData[sample] = gainValue * sinf(juce::MathConstants<float>::twoPi * inputPhase);
        else if (testSignal == 2) {
          // Duration of pulse is duration = phase / lfoFreq -> maxDuration = (0.05 * lfoFreq) / lfoFreq = 0.05s
          if (lfoPhase > 0.05 * lfoFrequencyValue) channelData[sample] = 0;
          else channelData[sample] = gainValue * sinf(juce::MathConstants<float>::twoPi * inputPhase);
        }
        else channelData[sample] = 0;
      }
      if (channelValue == 0) buffer.getWritePointer(1)[sample] = 0;
      if (channelValue == 2) buffer.getWritePointer(0)[sample] = 0;
    }

}

//==============================================================================
bool Tester1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Tester1AudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void Tester1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Tester1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Tester1AudioProcessor();
}
