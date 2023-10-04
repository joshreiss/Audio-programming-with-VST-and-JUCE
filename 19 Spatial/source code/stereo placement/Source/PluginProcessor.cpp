#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DemoProjectAudioProcessor::DemoProjectAudioProcessor()
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
	addParameter(pan = new AudioParameterFloat("pan", "Pan", -1.0f, 1.0f, 0.0f));
}

DemoProjectAudioProcessor::~DemoProjectAudioProcessor()
{
}

//==============================================================================
const String DemoProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DemoProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DemoProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DemoProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DemoProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DemoProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DemoProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DemoProjectAudioProcessor::setCurrentProgram (int index)
{
}

const String DemoProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void DemoProjectAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void DemoProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void DemoProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DemoProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DemoProjectAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	auto* channeldataL = buffer.getWritePointer(0);
	auto* channeldataR = buffer.getWritePointer(1); 
  float p = pan->get();

	for (int i = 0; i < buffer.getNumSamples(); i++)
	{
    float in = sinf(juce::MathConstants<float>::twoPi * inputPhase);
    // Update input phase
    inputPhase += 1000 / getSampleRate();
    while (inputPhase >= 1.0) inputPhase -= 1.0;

		channeldataL[i] = in * cos((p + 1.0) * float_Pi / 4);
		channeldataR[i] = in * sin((p + 1.0) * float_Pi / 4);
    /*
    auto pIncrement = 2 * LFOfrequency / sampleRate;
    if ((p + pIncrement > 1.0) && (sign == 1)) sign = -1;
    if ((p - pIncrement < -1.0) && (sign == -1)) sign = +1;
    p = p + sign * pIncrement;
    */
	}
}

//==============================================================================
bool DemoProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DemoProjectAudioProcessor::createEditor()
{
	return new GenericAudioProcessorEditor(this);
}

//==============================================================================
void DemoProjectAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DemoProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DemoProjectAudioProcessor();
}
