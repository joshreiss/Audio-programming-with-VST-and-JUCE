#include "PluginProcessor.h"
#include "PluginEditor.h"
ParametricEQAudioProcessor::ParametricEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
  : AudioProcessor (BusesProperties().withInput ("Input",  juce::AudioChannelSet::stereo(), true)
                                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true) )
#endif
{ }
#ifndef JucePlugin_PreferredChannelConfigurations
bool ParametricEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet()!=juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet()!=juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) return false;
   #endif
    return true;
}
#endif

void ParametricEQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) { 
  //SEE https://github.com/TheAudioProgrammer/juceIIRFilter
  auto newCoefficients = juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate, 1000.0, 1.0);

  juce::dsp::ProcessorChain<juce::dsp::Gain<double>, juce::dsp::Gain<double> > chain1;
  chain1.get<0>().setGainLinear(5.0);
  chain1.get<1>().setGainLinear(5.0);

  juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>> chain;
  chain.get<0>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 1.0f, 2.0f);

  // auto filter = new juce::IIRFilter(juce::IIRCoefficients::makeHighPass(sampleRate, 1000, 1.0)); // in prepareToPlay
  // filter->processSamples(buffer.getWritePointer(i), buffer.getNumSamples());    // in processBlock


}
void ParametricEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  auto numOutputChannels = getTotalNumOutputChannels();
  auto numInputChannels = getTotalNumInputChannels();

  float Q = 2.0f;
  float centreFrequency = 1000.0f;
  float sampleRate = (float)juce::AudioProcessor::getSampleRate();
  float normalisedFrequency = juce::MathConstants<float>::twoPi * centreFrequency / sampleRate;
  float gain = 1.0f;
  float bandwidth = normalisedFrequency / Q;

  const double two_cos_wc = -2.0 * cos(normalisedFrequency);
  const double tan_half_bw = tan(bandwidth / 2.0);
  const double g_tan_half_bw = gain * tan_half_bw;
  const double sqrt_g = sqrt(gain);

  auto filter = new juce::IIRFilter;
  filter->setCoefficients(juce::IIRCoefficients(sqrt_g + g_tan_half_bw,  // b0
    sqrt_g * two_cos_wc,     // b1
    sqrt_g - g_tan_half_bw,  // b2
    sqrt_g + tan_half_bw,    // a0
    sqrt_g * two_cos_wc,     // a1
    sqrt_g - tan_half_bw   // a2
  ));
  if (2.0f * rand() / (float)RAND_MAX < 0.0901) DBG(numInputChannels << " " << numOutputChannels);

  for (int i = 0; i < buffer.getNumSamples(); ++i) buffer.getWritePointer(0)[i] = 2.0f * rand() / (float)RAND_MAX - 1.0f;
  filter->processSamples(buffer.getWritePointer(0), buffer.getNumSamples());
}

void ParametricEQAudioProcessor::changeProgramName(int index, const juce::String& newName) { }
bool ParametricEQAudioProcessor::acceptsMidi() const { return false; }
bool ParametricEQAudioProcessor::producesMidi() const { return false; }
bool ParametricEQAudioProcessor::isMidiEffect() const { return false; }
ParametricEQAudioProcessor::~ParametricEQAudioProcessor() { }
const juce::String ParametricEQAudioProcessor::getName() const { return JucePlugin_Name; }
double ParametricEQAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int ParametricEQAudioProcessor::getNumPrograms() { return 1; }
int ParametricEQAudioProcessor::getCurrentProgram() { return 0; }
void ParametricEQAudioProcessor::setCurrentProgram(int index) { }
const juce::String ParametricEQAudioProcessor::getProgramName(int index) { return {}; }
void ParametricEQAudioProcessor::releaseResources() { }
bool ParametricEQAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* ParametricEQAudioProcessor::createEditor() { return new juce::GenericAudioProcessorEditor(this); }
void ParametricEQAudioProcessor::getStateInformation(juce::MemoryBlock& destData) { }
void ParametricEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes) { }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ParametricEQAudioProcessor(); }

