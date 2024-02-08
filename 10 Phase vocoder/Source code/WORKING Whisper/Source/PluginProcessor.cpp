/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

PassThroughAudioProcessor::PassThroughAudioProcessor()
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
  addParameter(fftSizeParam = new juce::AudioParameterChoice("fftSize", "FFT Size", { "32","64","128","256","512","1024","2048","4096","8192" }, 4));
  addParameter(hopSizeParam = new juce::AudioParameterChoice("hopSize", "Hop Size", { "1/2 Window","1/4 Window","1/8 Window" }, 2));
  addParameter(windowTypeParam = new juce::AudioParameterChoice("windowType", "Window Type", { "Bartlett","Hamm","Hanning" }, 1));
}
  /* paramFftSize (parameters, "FFT size", fftSizeItemsUI, fftSize512,
                        value = (float)(1 << ((int)value + 5)); // 0-> 32 ... 32, 64, 128, 256, 512, 1024...
                        paramFftSize.setCurrentAndTargetValue (value);
     paramHopSize (parameters, "Hop size", hopSizeItemsUI, hopSize8,
                        value = (float)(1 << ((int)value + 1)); // 0-> 2 ... 1/2, 1/4, 1/8
                        paramHopSize.setCurrentAndTargetValue (value);
     paramWindowType (parameters, "Window type", windowTypeItemsUI, STFT::windowTypeHann,
                       [this](float value){
                           paramWindowType.setCurrentAndTargetValue (value);
                       */

PassThroughAudioProcessor::~PassThroughAudioProcessor()
{
}

//==============================================================================

void PassThroughAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    stft.setup (getTotalNumInputChannels());
    stft.updateParameters(512, 4, 1);
    // stft.updateParameters((int)paramFftSize.getTargetValue(), (int)paramHopSize.getTargetValue(), (int)paramWindowType.getTargetValue());
}

void PassThroughAudioProcessor::releaseResources()
{
}

void PassThroughAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const ScopedLock sl (lock);

    ScopedNoDenormals noDenormals;

    const int numInputChannels = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    stft.processBlock (buffer);

    for (int channel = numInputChannels; channel < numOutputChannels; ++channel) buffer.clear (channel, 0, numSamples);
}

void PassThroughAudioProcessor::getStateInformation (MemoryBlock& destData)
{
}

void PassThroughAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

AudioProcessorEditor* PassThroughAudioProcessor::createEditor()
{
  return new juce::GenericAudioProcessorEditor(this);
}

bool PassThroughAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PassThroughAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

const String PassThroughAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PassThroughAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PassThroughAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PassThroughAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PassThroughAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PassThroughAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PassThroughAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PassThroughAudioProcessor::setCurrentProgram (int index)
{
}

const String PassThroughAudioProcessor::getProgramName (int index)
{
    return {};
}

void PassThroughAudioProcessor::changeProgramName (int index, const String& newName)
{
}

// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PassThroughAudioProcessor();
}