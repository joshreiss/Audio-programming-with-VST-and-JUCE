/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <JuceHeader.h>
#include "STFT.h"

//==============================================================================
/**
*/
class FrequencyDomainAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    FrequencyDomainAudioProcessor();
    ~FrequencyDomainAudioProcessor() override;

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
    class PassThrough : public STFT
    {
    private:
      void modification() override
      {
        fft->perform(timeDomainBuffer, frequencyDomainBuffer, false);

        for (int index = 0; index < fftSize / 2 + 1; ++index) {
          float magnitude = abs(frequencyDomainBuffer[index]);
          float phase = arg(frequencyDomainBuffer[index]);

          frequencyDomainBuffer[index].real(magnitude * cosf(phase));
          frequencyDomainBuffer[index].imag(magnitude * sinf(phase));
          if (index > 0 && index < fftSize / 2) {
            frequencyDomainBuffer[fftSize - index].real(magnitude * cosf(phase));
            frequencyDomainBuffer[fftSize - index].imag(magnitude * sinf(-phase));
          }
        }

        fft->perform(frequencyDomainBuffer, timeDomainBuffer, true);
      }
    };

    //======================================
    PassThrough stft;
private:
    juce::AudioParameterChoice* hopSizeParam;
    juce::AudioParameterChoice* fftSizeParam;
    juce::AudioParameterChoice* windowSizeParam;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencyDomainAudioProcessor)
};
