#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <JuceHeader.h>
#include "STFT.h"
class RobotisationAudioProcessor : public AudioProcessor
{
public:

    RobotisationAudioProcessor();
    ~RobotisationAudioProcessor();

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

    StringArray fftSizeItemsUI = {
        "32",
        "64",
        "128",
        "256",
        "512",
        "1024",
        "2048",
        "4096",
        "8192"
    };

    enum fftSizeIndex {
        fftSize32 = 0,
        fftSize64,
        fftSize128,
        fftSize256,
        fftSize512,
        fftSize1024,
        fftSize2048,
        fftSize4096,
        fftSize8192,
    };

    StringArray hopSizeItemsUI = {
        "1/2 Window",
        "1/4 Window",
        "1/8 Window",
    };

    enum hopSizeIndex {
        hopSize2 = 0,
        hopSize4,
        hopSize8,
    };

    StringArray windowTypeItemsUI = {
        "Rectangular",
        "Bartlett",
        "Hann",
        "Hamming",
    };

    class Robotisation : public STFT
    {
    private:
        void modification() override
        {
            fft->perform (timeDomainBuffer, frequencyDomainBuffer, false);

            for (int index = 0; index < fftSize / 2 + 1; ++index) {
                float magnitude = abs (frequencyDomainBuffer[index]);
                float phase = arg (frequencyDomainBuffer[index]);

                frequencyDomainBuffer[index].real (magnitude * cosf (phase));
                frequencyDomainBuffer[index].imag (magnitude * sinf (phase));
                if (index > 0 && index < fftSize / 2) {
                    frequencyDomainBuffer[fftSize - index].real (magnitude * cosf (phase));
                    frequencyDomainBuffer[fftSize - index].imag (magnitude * sinf (-phase));
                }
            }

            fft->perform (frequencyDomainBuffer, timeDomainBuffer, true);
        }
    };

    CriticalSection lock;
    Robotisation stft;


    juce::AudioParameterChoice* fftSizeParam;
    juce::AudioParameterChoice* hopSizeParam;
    juce::AudioParameterChoice* windowTypeParam;
    int fftSize;
    int hopSize; 
    int windowType;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RobotisationAudioProcessor)
};
