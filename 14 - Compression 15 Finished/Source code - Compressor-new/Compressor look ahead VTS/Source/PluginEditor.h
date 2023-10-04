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
class CompressorAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CompressorAudioProcessorEditor (CompressorAudioProcessor&, AudioProcessorValueTreeState&);
    ~CompressorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override; 
    void addSlider(String name, String labelText, Slider& slider, Label& label, std::unique_ptr<SliderAttachment>& attachment);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CompressorAudioProcessor& audioProcessor;
    AudioProcessorValueTreeState& params;
    Slider threshSlider, slopeSlider, kneeSlider, attackSlider, releaseSlider;
    Label threshLabel, slopeLabel, kneeLabel, attackLabel, releaseLabel;
    std::unique_ptr<SliderAttachment> threshAttachment, slopeAttachment, kneeAttachment, attackAttachment, releaseAttachment;
 //   void addSlider(String name, String labelText, Slider& slider, Label& label, std::unique_ptr<SliderAttachment>&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorAudioProcessorEditor)
};
