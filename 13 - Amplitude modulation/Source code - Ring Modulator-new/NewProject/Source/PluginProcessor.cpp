/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
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
  addParameter(carrierFrequencyParam = new juce::AudioParameterFloat("carrierFrequency", "Carrier frequency", 1.0f, 1000.0f, 100.0f));
  addParameter(carrierTypeParam = new juce::AudioParameterChoice("carrierType", "Carrier type", {  "triangle", "square", "sloped square", "sine"}, 1));
  addParameter(mixParam = new juce::AudioParameterFloat("mix", "Dry/wet mix", 0.0f, 1.0f, 0.5f));
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    float mix = mixParam->get();
    float carrierFrequency = carrierFrequencyParam->get();
    float sampleRate = (float) juce::AudioProcessor::getSampleRate();


    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    auto numOutputChannels = getTotalNumOutputChannels();
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
      float carrierSignal = sinf(juce::MathConstants<float>::twoPi * carrierPhase);
      for (int j = 0; j < numOutputChannels; ++j) buffer.getWritePointer(j)[i] = (2.0f*rand()/RAND_MAX - 1.0f) * ((1.0-mix) + mix * carrierSignal);

      // Update carrier phase, keeping in range [0, 1]
      carrierPhase += carrierFrequency / sampleRate;
      while (carrierPhase >= 1.0) carrierPhase -= 1.0;
    }
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}


// Function for calculating "biased" LFO waveforms with output range [0, 1].
// Phase range [0, 1], output also [0, 1] (not [-1, +1] as for the ordinary Sine function).
float NewProjectAudioProcessor::getLFOSample(float phase, int waveform)
{
  switch (waveform)
  {
  case 0: //Triangle:
    if (phase < 0.25f) return 0.5f + 2.0f * phase;
    else if (phase < 0.75f) return 1.0f - 2.0f * (phase - 0.25f);
    else return 2.0f * (phase - 0.75f);
  case 1: //Square:
    if (phase < 0.5f) return 1.0f;
    else return 0.0f;
  case 2: //SquareSlopedEdges:
    if (phase < 0.48f) return 1.0f;
    else if (phase < 0.5f) return 1.0f - 50.0f * (phase - 0.48f);
    else if (phase < 0.98f) return 0.0f;
    else return 50.0f * (phase - 0.98f);
  case 3: //Sine:
  default:
    return 0.5f + 0.5f * sinf(juce::MathConstants<float>::twoPi * phase);
  }
}
