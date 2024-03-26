#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <JuceHeader.h>
#include "STFT.h"
class WhisperAudioProcessor : public AudioProcessor
{
public:

    WhisperAudioProcessor();
    ~WhisperAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect () const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    class Whisper : public STFT
    {
    private:
        void modification() override
        {
            fft->perform(timeDomainBuffer, frequencyDomainBuffer, false);

            for (int index = 0; index < fftSize / 2 + 1; ++index) {
              float magnitude = abs(frequencyDomainBuffer[index]);
              float phase = 2.0f * M_PI * (float)rand() / (float)RAND_MAX; // random phases for whisperisation

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

    CriticalSection lock;
    Whisper stft;

    juce::AudioParameterChoice* fftSizeParam;
    juce::AudioParameterChoice* hopSizeParam;
    juce::AudioParameterChoice* windowTypeParam;
    int fftSize;
    int hopSize; 
    int windowType;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WhisperAudioProcessor)
};
