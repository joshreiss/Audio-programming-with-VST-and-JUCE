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
class VibratoAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    VibratoAudioProcessor();
    ~VibratoAudioProcessor() override;

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
    //==============================================================================
  float getLfoSample(float phase, int waveform);
  float interpolateSample(int type, float delayReadPosition, float* delayData, int delayBufferLength);
  void parameterChanged();
  juce::AudioParameterFloat* lfoFrequencyParam;
  juce::AudioParameterFloat* sweepWidthParam;
  juce::AudioParameterChoice* lfoTypeParam;
  juce::AudioParameterChoice* interpolationTypeParam;

  float lfoPhase;             // Phase of the low-frequency oscillator
  double inverseSampleRate;   // Cache inverse of sample rate (more efficient to multiply than divide)

  // Circular buffer variables for implementing delay
  juce::AudioSampleBuffer delayBuffer;
  int delayBufferLength;
  int delayReadPosition, delayWritePosition = 0;

  float inputPhase = 0, phase = 0;
 };
