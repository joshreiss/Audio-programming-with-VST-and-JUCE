/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class PitchShiftAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    PitchShiftAudioProcessor();
    ~PitchShiftAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
  std::unique_ptr<int> test;
  void updateFftSize();
  void updateWindow(const juce::HeapBlock<float>& window, const int windowLength);
  void updateWindowScaleFactor();
  float princArg(const float phase);

  juce::CriticalSection lock;

  // be sure to add the juce dsp module
  std::unique_ptr<juce::dsp::FFT> fft;

  int inputBufferLength;
  int inputBufferWritePosition;
  juce::AudioSampleBuffer inputBuffer;

  int outputBufferLength;
  int outputBufferWritePosition;
  int outputBufferReadPosition;
  juce::AudioSampleBuffer outputBuffer;

  juce::HeapBlock<float> fftWindow;
  juce::HeapBlock<juce::dsp::Complex<float>> fftTimeDomain;
  juce::HeapBlock<juce::dsp::Complex<float>> fftFrequencyDomain;

  int samplesSinceLastFFT;

  float windowScaleFactor;

  juce::HeapBlock<float> omega;
  juce::AudioSampleBuffer inputPhase;
  juce::AudioSampleBuffer outputPhase;

  juce::AudioParameterFloat* shiftParam;
  juce::AudioParameterChoice* fftSizeParam;
  juce::AudioParameterChoice* hopSizeParam;
  juce::AudioParameterChoice* windowTypeParam;
  float shift;
  int fftSize;
  int overlap; // based on hopSizeParam
  int hopSize; // hopSize = fftSize / overlap
  int windowType;
  bool needToResetPhases;
 };
