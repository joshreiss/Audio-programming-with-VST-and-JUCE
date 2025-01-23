#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class STFTAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    STFTAudioProcessorEditor (STFTAudioProcessor&);
    ~STFTAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void drawNextLineOfSpectrogram();
private:
    // This reference is quick way for your editor to access the processor object that created it.
    STFTAudioProcessor& audioProcessor;
    juce::dsp::FFT forwardFFT;  
    juce::Image spectrogramImage;
    void timerCallback() final;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (STFTAudioProcessorEditor)
};
