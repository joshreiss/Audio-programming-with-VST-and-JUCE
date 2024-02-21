/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

EmptyPhaseVocoderAudioProcessor::EmptyPhaseVocoderAudioProcessor()
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

EmptyPhaseVocoderAudioProcessor::~EmptyPhaseVocoderAudioProcessor()
{
}

//==============================================================================

void EmptyPhaseVocoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    stft.setup (getTotalNumInputChannels());
    stft.updateParameters(512, 4, 1);
}

void EmptyPhaseVocoderAudioProcessor::releaseResources()
{
}

void EmptyPhaseVocoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
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

void EmptyPhaseVocoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
}

void EmptyPhaseVocoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

AudioProcessorEditor* EmptyPhaseVocoderAudioProcessor::createEditor()
{
  return new juce::GenericAudioProcessorEditor(this);
}

bool EmptyPhaseVocoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EmptyPhaseVocoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

const String EmptyPhaseVocoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EmptyPhaseVocoderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EmptyPhaseVocoderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EmptyPhaseVocoderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EmptyPhaseVocoderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EmptyPhaseVocoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EmptyPhaseVocoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EmptyPhaseVocoderAudioProcessor::setCurrentProgram (int index)
{
}

const String EmptyPhaseVocoderAudioProcessor::getProgramName (int index)
{
    return {};
}

void EmptyPhaseVocoderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EmptyPhaseVocoderAudioProcessor();
}