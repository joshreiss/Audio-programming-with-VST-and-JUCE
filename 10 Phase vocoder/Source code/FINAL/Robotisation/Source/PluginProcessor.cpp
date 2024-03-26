/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

RobotAudioProcessor::RobotAudioProcessor()
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
  addParameter(fftSizeParam = new juce::AudioParameterChoice("fftSize", "FFT size", { "32","64","128","256","512","1024","2048","4096","8192" }, 4));
  addParameter(hopSizeParam = new juce::AudioParameterChoice("hopSize", "Hop size", { "1/2 Window","1/4 Window","1/8 Window" }, 2));
  addParameter(windowTypeParam = new juce::AudioParameterChoice("windowType", "Window type", { "Rectangular", "Bartlett","Hamm","Hanning" }, 1));
}

RobotAudioProcessor::~RobotAudioProcessor()
{
}

//==============================================================================

void RobotAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    stft.setup (getTotalNumInputChannels());
    stft.updateParameters(512, 4, 1);
}

void RobotAudioProcessor::releaseResources()
{
}

void RobotAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const ScopedLock sl (lock);

    ScopedNoDenormals noDenormals;
    if (fftSize != fftSizeParam->getIndex()) DBG("New fft size " << fftSizeParam->getIndex());
    if (hopSize != hopSizeParam->getIndex()) DBG("New hop size " << hopSizeParam->getIndex());
    if (windowType != windowTypeParam->getIndex()) DBG("New window type " << windowTypeParam->getIndex());
    if ( (fftSize != fftSizeParam->getIndex()) 
      || (hopSize != hopSizeParam->getIndex())
      || (windowType != windowTypeParam->getIndex()) )
    {
      fftSize = fftSizeParam->getIndex();
      hopSize = hopSizeParam->getIndex();
      windowType = windowTypeParam->getIndex();
      stft.updateParameters((float)(1 << ((int)fftSize + 5)), (float)(1 << ((int)hopSize + 1)), windowType);
    }

    const int numInputChannels = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    stft.processBlock (buffer);

    for (int channel = numInputChannels; channel < numOutputChannels; ++channel) buffer.clear (channel, 0, numSamples);
}

void RobotAudioProcessor::getStateInformation (MemoryBlock& destData)
{
}

void RobotAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

AudioProcessorEditor* RobotAudioProcessor::createEditor()
{
  return new juce::GenericAudioProcessorEditor(this);
}

bool RobotAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RobotAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

const String RobotAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RobotAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RobotAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RobotAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RobotAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RobotAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RobotAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RobotAudioProcessor::setCurrentProgram (int index)
{
}

const String RobotAudioProcessor::getProgramName (int index)
{
    return {};
}

void RobotAudioProcessor::changeProgramName (int index, const String& newName)
{
}

// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RobotAudioProcessor();
}