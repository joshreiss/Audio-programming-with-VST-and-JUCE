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
class FlangerAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    FlangerAudioProcessor();
    ~FlangerAudioProcessor() override;

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
  juce::AudioParameterChoice* lfoTypeParam;
  juce::AudioParameterChoice* interpolationTypeParam;
  juce::AudioParameterFloat* sweepWidthParam;
  juce::AudioParameterFloat* minimumDelayParam;
  juce::AudioParameterFloat* depthParam;
  juce::AudioParameterFloat* feedbackParam;
  juce::AudioParameterBool* stereoParam;
  float lfoPhase = 0;             // Phase of the low-frequency oscillator
  float interpolatedSample;  // For fractional delay

  // Circular buffer variables for implementing delay
  juce::AudioSampleBuffer delayBuffer;
  int delayBufferLength = 0;
  int delayReadPosition; //do i use this?
  int delayWritePosition = 0;

 };