#pragma once
#include <JuceHeader.h>
class SpectrumAudioProcessor  : public juce::AudioProcessor
{
public:
    SpectrumAudioProcessor();
    ~SpectrumAudioProcessor() override;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void pushNextSampleIntoFifo(float sample) noexcept;
    enum
    {
        fftOrder = 11,             // [1]
        fftSize = 1 << fftOrder,  // [2]
        scopeSize = 512             // [3]
    };
    float fifo[fftSize];                           // [6]
    float fftData[2 * fftSize];                    // [7]
    int fifoIndex = 0;                              // [8]
    bool nextFFTBlockReady = false;                 // [9]
    float scopeData[scopeSize];                    // [10]
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAudioProcessor)
};
