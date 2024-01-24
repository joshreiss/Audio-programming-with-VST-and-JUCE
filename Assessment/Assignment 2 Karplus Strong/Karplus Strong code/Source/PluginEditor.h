/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class KarplusStrongAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    KarplusStrongAudioProcessorEditor (KarplusStrongAudioProcessor&);
    ~KarplusStrongAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    KarplusStrongAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KarplusStrongAudioProcessorEditor)
};
