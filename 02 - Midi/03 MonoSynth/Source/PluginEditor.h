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
class MonoSynthAudioProcessorEditor  : public juce::AudioProcessorEditor, private Slider::Listener
{
public:
    MonoSynthAudioProcessorEditor (MonoSynthAudioProcessor&);
    ~MonoSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MonoSynthAudioProcessor& audioProcessor;
    Slider gainSlider;
    MidiKeyboardComponent myKeyboard;
    void sliderValueChanged(Slider* slider);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonoSynthAudioProcessorEditor)
};