#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SpectrumAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    SpectrumAudioProcessorEditor (SpectrumAudioProcessor&);
    ~SpectrumAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void pushNextSampleIntoFifo(float) noexcept;    
    void drawNextFrameOfSpectrum();
    void drawFrame(juce::Graphics&);
    void timerCallback() final;
private:
    // This reference is quick way for your editor to access the processor object that created it.
    SpectrumAudioProcessor& audioProcessor;
    juce::dsp::FFT forwardFFT;                      // [4]
    juce::dsp::WindowingFunction<float> window;     // [5]
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAudioProcessorEditor)
};
