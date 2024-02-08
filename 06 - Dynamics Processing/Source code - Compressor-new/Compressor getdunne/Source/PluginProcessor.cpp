/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CompressorAudioProcessor::CompressorAudioProcessor()
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
  addParameter(thresholdParam = new juce::AudioParameterFloat("threshold", "Threshold", -60.0f, 0.0f, 0.0f));
  addParameter(ratioParam = new juce::AudioParameterFloat("ratio", "Ratio", 1.0f, 20.0, 1.0f));
  addParameter(attackTimeParam = new juce::AudioParameterFloat("attackTime", "Attack time", 0.1f, 80.0f, 15.0f));
  addParameter(releaseTimeParam= new juce::AudioParameterFloat("releaseTime", "Release time", 1.0f, 1000.0f, 100.0f));
  addParameter(makeUpGainParam = new juce::AudioParameterFloat("makeUpGain", "Make-up gain", 0.0f, 24.0, 0.0f));
}

CompressorAudioProcessor::~CompressorAudioProcessor()
{
}

//==============================================================================
const juce::String CompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void CompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void CompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    float threshold = thresholdParam->get();
    float ratio = ratioParam->get();
    float attackTime = attackTimeParam->get();
    float releaseTime = releaseTimeParam->get();
    float makeUpGain = makeUpGainParam->get();

    float sampleRate = (float) juce::AudioProcessor::getSampleRate();

    float alphaAttack = exp(-1.0f / (0.001f * sampleRate * attackTime));
    float alphaRelease = exp(-1.0f / (0.001f * sampleRate * releaseTime));

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
    auto numInputChannels = getTotalNumInputChannels();
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
      float inputSignal = sinf(juce::MathConstants<float>::twoPi * inputPhase);

      // Level detection- estimate level using peak detector
      if (fabs(inputSignal) < 0.000001f) x_g = -120;
      else x_g = 20 * log10(fabs(inputSignal));

      // Gain computer- static apply input/output curve
      if (x_g >= threshold) y_g = threshold + (x_g - threshold) / ratio;
      else y_g = x_g;
      x_l = x_g - y_g;

      // Ballistics- smoothing of the gain 
      if (x_l > yL_prev) y_l = alphaAttack * yL_prev + (1 - alphaAttack) * x_l;
      else y_l = alphaRelease * yL_prev + (1 - alphaRelease) * x_l;

      // find control
      c = pow(10.0f, (makeUpGain - y_l) / 20.0f);
      yL_prev = y_l;

      for (int j = 0; j < numInputChannels; ++j) buffer.getWritePointer(j)[i] = inputSignal * c;
      //for (int j = 0; j < numInputChannels; ++j) buffer.getWritePointer(j)[i] = buffer.getReadPointer(j)[i] * (1.0f - depth * amount);

      // Update input phase
      inputPhase += 1000 / sampleRate;
      while (inputPhase >= 1.0) inputPhase -= 1.0;

    }
}

//==============================================================================
bool CompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CompressorAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void CompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressorAudioProcessor();
}

