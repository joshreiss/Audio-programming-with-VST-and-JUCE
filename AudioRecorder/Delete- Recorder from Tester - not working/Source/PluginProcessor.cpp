#include "PluginProcessor.h"
#include "PluginEditor.h"

RecorderAudioProcessor::RecorderAudioProcessor()
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

RecorderAudioProcessor::~RecorderAudioProcessor()
{
}

const juce::String RecorderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RecorderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RecorderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RecorderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RecorderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RecorderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RecorderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RecorderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RecorderAudioProcessor::getProgramName (int index)
{
    return {};
}
void RecorderAudioProcessor::changeProgramName (int index, const juce::String& newName) {}
void RecorderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  transport.prepareToPlay(sampleRate, samplesPerBlock);
}
void RecorderAudioProcessor::releaseResources() { }
#ifndef JucePlugin_PreferredChannelConfigurations
bool RecorderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif
    return true;
  #endif
}
#endif

void RecorderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels  = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) buffer.clear (i, 0, buffer.getNumSamples());
  for (int sample = 0; sample < buffer.getNumSamples(); sample++) for (int channel = 0; channel < getTotalNumOutputChannels(); ++channel)
  {
    buffer.getWritePointer(channel)[sample] = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
  }
 // transport.getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));
}
bool RecorderAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* RecorderAudioProcessor::createEditor() { return new RecorderAudioProcessorEditor (*this); }
void RecorderAudioProcessor::getStateInformation (juce::MemoryBlock& destData) { }
void RecorderAudioProcessor::setStateInformation(const void* data, int sizeInBytes) { }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RecorderAudioProcessor(); }

