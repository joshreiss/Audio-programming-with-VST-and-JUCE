/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
using namespace juce;
typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;


//==============================================================================
/**
*/
class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&, AudioProcessorValueTreeState&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override; 
    void addSlider(String name, String labelText, Slider& slider, Label& label, std::unique_ptr<SliderAttachment>& attachment);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;
    AudioProcessorValueTreeState& params;
    Slider gainSlider;
    Label gainLabel;
    std::unique_ptr<SliderAttachment> gainAttachment;
 //   void addSlider(String name, String labelText, Slider& slider, Label& label, std::unique_ptr<SliderAttachment>&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
