/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PitchShiftAudioProcessor::PitchShiftAudioProcessor()
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
  addParameter(shiftParam = new juce::AudioParameterFloat("shift", "Shift (semitones)", { -12.0f, 12.0f, 0.0f }, 0.0f)); //start, end, interval
  addParameter(fftSizeParam = new juce::AudioParameterChoice("fftSize", "FFT Size", { "32","64","128","256","512","1024","2048","4096","8192" }, 4));
  addParameter(hopSizeParam = new juce::AudioParameterChoice("hopSize", "Hop Size", { "1/2 Window","1/4 Window","1/8 Window" }, 2));
  addParameter(windowTypeParam = new juce::AudioParameterChoice("windowType", "Window Type", { "Bartlett","Hamm","Hanning" }, 1));
}

PitchShiftAudioProcessor::~PitchShiftAudioProcessor()
{
}

//==============================================================================
const juce::String PitchShiftAudioProcessor::getName() const
{
  return JucePlugin_Name;
}

bool PitchShiftAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool PitchShiftAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool PitchShiftAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double PitchShiftAudioProcessor::getTailLengthSeconds() const
{
  return 0.0;
}

int PitchShiftAudioProcessor::getNumPrograms()
{
  return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
  // so this should be at least 1, even if you're not really implementing programs.
}

int PitchShiftAudioProcessor::getCurrentProgram()
{
  return 0;
}

void PitchShiftAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String PitchShiftAudioProcessor::getProgramName(int index)
{
  return {};
}

void PitchShiftAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void PitchShiftAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  /*  const double smoothTime = 1e-3;
  shift.reset(sampleRate, smoothTime);
  fftSize.reset(sampleRate, smoothTime);
  hopSizeParam.reset(sampleRate, smoothTime);
  windowTypeParam.reset(sampleRate, smoothTime); */

  //======================================

  updateFftSize();
  //update hop size
  //overlap = (int)paramHopSize.getTargetValue();
  overlap = hopSizeParam->getIndex();
  if (overlap != 0) {
    hopSize = fftSize / overlap;
    outputBufferWritePosition = hopSize % outputBufferLength;
  }
  updateWindow(fftWindow, fftSize);
  updateWindowScaleFactor();
  needToResetPhases = true;
}

void PitchShiftAudioProcessor::releaseResources()
{
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PitchShiftAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
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

void PitchShiftAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  juce::ScopedNoDenormals noDenormals;

  // Helpful information about this block of samples:
  const int numInputChannels = getTotalNumInputChannels();    // How many input channels for our effect?
  const int numOutputChannels = getTotalNumOutputChannels();  // How many output channels for our effect?
  const int numSamples = buffer.getNumSamples();              // How many samples in the buffer for this block?
  float sampleRate = (float)juce::AudioProcessor::getSampleRate();
  //make global
  shift = shiftParam->get();
  fftSize = fftSizeParam->getIndex(); //CHECK ???
  hopSize = hopSizeParam->getIndex();
  windowType = windowTypeParam->getIndex();

  int currentInputBufferWritePosition;
  int currentOutputBufferWritePosition;
  int currentOutputBufferReadPosition;
  int currentSamplesSinceLastFFT;

  float ratio = roundf(shift * (float)hopSize) / (float)hopSize;
  int resampledLength = floorf((float)fftSize / ratio);
  juce::HeapBlock<float> resampledOutput(resampledLength, true);
  juce::HeapBlock<float> synthesisWindow(resampledLength, true);
  updateWindow(synthesisWindow, resampledLength);

  for (int channel = 0; channel < numInputChannels; ++channel) {
    float* channelData = buffer.getWritePointer(channel);

    currentInputBufferWritePosition = inputBufferWritePosition;
    currentOutputBufferWritePosition = outputBufferWritePosition;
    currentOutputBufferReadPosition = outputBufferReadPosition;
    currentSamplesSinceLastFFT = samplesSinceLastFFT;

    for (int sample = 0; sample < numSamples; ++sample) {
      const float in = channelData[sample];
      channelData[sample] = outputBuffer.getSample(channel, currentOutputBufferReadPosition);
      outputBuffer.setSample(channel, currentOutputBufferReadPosition, 0.0f);
      if (++currentOutputBufferReadPosition >= outputBufferLength) currentOutputBufferReadPosition = 0;
      inputBuffer.setSample(channel, currentInputBufferWritePosition, in);
      if (++currentInputBufferWritePosition >= inputBufferLength) currentInputBufferWritePosition = 0;
      if (++currentSamplesSinceLastFFT >= hopSize) {
        currentSamplesSinceLastFFT = 0;
        int inputBufferIndex = currentInputBufferWritePosition;
        for (int index = 0; index < fftSize; ++index) {
          fftTimeDomain[index].real(sqrtf(fftWindow[index]) * inputBuffer.getSample(channel, inputBufferIndex));
          fftTimeDomain[index].imag(0.0f);
          if (++inputBufferIndex >= inputBufferLength) inputBufferIndex = 0;
        }
        fft->perform(fftTimeDomain, fftFrequencyDomain, false);
        for (int index = 0; index < fftSize; ++index) {
          float magnitude = abs(fftFrequencyDomain[index]);
          float phase = arg(fftFrequencyDomain[index]);
          float phaseDeviation = phase - inputPhase.getSample(channel, index) - omega[index] * (float)hopSize;
          float deltaPhi = omega[index] * hopSize + princArg(phaseDeviation);
          float newPhase = princArg(outputPhase.getSample(channel, index) + deltaPhi * ratio);
          inputPhase.setSample(channel, index, phase);
          outputPhase.setSample(channel, index, newPhase);
          fftFrequencyDomain[index] = std::polar(magnitude, newPhase);
        }
        fft->perform(fftFrequencyDomain, fftTimeDomain, true);
        for (int index = 0; index < resampledLength; ++index) {
          float x = (float)index * (float)fftSize / (float)resampledLength;
          int ix = (int)floorf(x);
          float dx = x - (float)ix;
          float sample1 = fftTimeDomain[ix].real();
          float sample2 = fftTimeDomain[(ix + 1) % fftSize].real();
          resampledOutput[index] = sample1 + dx * (sample2 - sample1);
          resampledOutput[index] *= sqrtf(synthesisWindow[index]);
        }
        int outputBufferIndex = currentOutputBufferWritePosition;
        for (int index = 0; index < resampledLength; ++index) {
          float out = outputBuffer.getSample(channel, outputBufferIndex);
          out += resampledOutput[index] * windowScaleFactor;
          outputBuffer.setSample(channel, outputBufferIndex, out);
          if (++outputBufferIndex >= outputBufferLength) outputBufferIndex = 0;
        }
        currentOutputBufferWritePosition += hopSize;
        if (currentOutputBufferWritePosition >= outputBufferLength) currentOutputBufferWritePosition = 0;
      }
    }
  }
  inputBufferWritePosition = currentInputBufferWritePosition;
  outputBufferWritePosition = currentOutputBufferWritePosition;
  outputBufferReadPosition = currentOutputBufferReadPosition;
  samplesSinceLastFFT = currentSamplesSinceLastFFT;
  // In case we have more outputs than inputs, clear any output channels that didn't contain input data
  for (int i = numInputChannels; i < numOutputChannels; ++i) buffer.clear(i, 0, buffer.getNumSamples());
}

void PitchShiftAudioProcessor::updateFftSize()
{
//  fftSize = (int)paramFftSize.getTargetValue();
  fftSize = (int)pow(2,5 + fftSizeParam->getIndex()); // 5 + since index 0 is 2^5 = 32
  fft = std::make_unique<juce::dsp::FFT>(log2(fftSize));

  inputBufferLength = fftSize;
  inputBufferWritePosition = 0;
  inputBuffer.clear();
  inputBuffer.setSize(getTotalNumInputChannels(), inputBufferLength);

  float maxRatio = 0.5; // -12 semitones  powf(2.0f, paramShift.minValue / 12.0f);
  outputBufferLength = (int)floorf((float)fftSize / maxRatio);
  outputBufferWritePosition = 0;
  outputBufferReadPosition = 0;
  outputBuffer.clear();
  outputBuffer.setSize(getTotalNumInputChannels(), outputBufferLength);

  fftWindow.realloc(fftSize);
  fftWindow.clear(fftSize);

  fftTimeDomain.realloc(fftSize);
  fftTimeDomain.clear(fftSize);

  fftFrequencyDomain.realloc(fftSize);
  fftFrequencyDomain.clear(fftSize);

  samplesSinceLastFFT = 0;

  omega.realloc(fftSize);
  for (int index = 0; index < fftSize; ++index) omega[index] = juce::MathConstants<float>::twoPi * index / (float)fftSize;

  inputPhase.clear();
  inputPhase.setSize(getTotalNumInputChannels(), outputBufferLength);

  outputPhase.clear();
  outputPhase.setSize(getTotalNumInputChannels(), outputBufferLength);
}
void PitchShiftAudioProcessor::updateWindow(const juce::HeapBlock<float>& window, const int windowLength)
{
  // windowTypeHann: {
  for (int sample = 0; sample < windowLength; ++sample) 
    window[sample] = 0.5f - 0.5f * cosf(juce::MathConstants<float>::twoPi * (float)sample / (float)(windowLength - 1));
}
void PitchShiftAudioProcessor::updateWindowScaleFactor()
{
  float windowSum = 0.0f;
  for (int sample = 0; sample < fftSize; ++sample) windowSum += fftWindow[sample];
  windowScaleFactor = 0.0f;
  if (overlap != 0 && windowSum != 0.0f) windowScaleFactor = 1.0f / (float)overlap / windowSum * (float)fftSize;
}
float PitchShiftAudioProcessor::princArg(const float phase)
{
  if (phase >= 0.0f) return fmod(phase + juce::MathConstants<float>::pi, juce::MathConstants<float>::twoPi) - juce::MathConstants<float>::pi;
  else return fmod(phase + juce::MathConstants<float>::pi, -juce::MathConstants<float>::twoPi) + juce::MathConstants<float>::pi;
}

bool PitchShiftAudioProcessor::hasEditor() const
{
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PitchShiftAudioProcessor::createEditor()
{
  return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void PitchShiftAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void PitchShiftAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  // You should use this method to restore your parameters from this memory block,
  // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new PitchShiftAudioProcessor();
}
