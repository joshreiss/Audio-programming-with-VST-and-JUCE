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
class PanningAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    PanningAudioProcessor();
    ~PanningAudioProcessor() override;

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
  juce::AudioParameterChoice* methodParam;
  juce::AudioParameterFloat* panningParam;
  class DelayLine
  {
  public:
    void setup(const int maxDelayTimeInSamples)
    {
      delayBufferSamples = maxDelayTimeInSamples + 2;
      if (delayBufferSamples < 1) delayBufferSamples = 1;
      delayBuffer.setSize(1, delayBufferSamples);
      delayBuffer.clear();
      delayWritePosition = 0;
    }
    void writeSample(const float sampleToWrite)
    {
      delayBuffer.setSample(0, delayWritePosition, sampleToWrite);
      if (++delayWritePosition >= delayBufferSamples) delayWritePosition -= delayBufferSamples;
    }
    float readSample(const float delayTime) {
      float readPosition = fmodf((float)(delayWritePosition - 1) - delayTime + (float)delayBufferSamples, delayBufferSamples);
      int localReadPosition = floorf(readPosition);
      float fraction = readPosition - (float)localReadPosition;
      float delayed1 = delayBuffer.getSample(0, (localReadPosition + 0));
      float delayed2 = delayBuffer.getSample(0, (localReadPosition + 1) % delayBufferSamples);
      return delayed1 + fraction * (delayed2 - delayed1);
    }
  private:
    juce::AudioSampleBuffer delayBuffer;
    int delayBufferSamples;
    int delayWritePosition;
  };
  DelayLine delayLineL;
  DelayLine delayLineR;
  class Filter : public juce::IIRFilter
  {
  public:
    void updateCoefficients(const double angle, const double headFactor) noexcept
    {
      double alpha = 1.0 + cos(angle);

      coefficients = juce::IIRCoefficients(/* b0 */ headFactor + alpha,
        /* b1 */ headFactor - alpha,
        /* b2 */ 0.0,
        /* a0 */ headFactor + 1.0,
        /* a1 */ headFactor - 1.0,
        /* a2 */ 0.0);
      setCoefficients(coefficients);
    }
  };

  Filter filterL;
  Filter filterR;
  int maximumDelayInSamples;
 };
