/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "AudioRecordingComponent.h"

//==============================================================================
/**
*/
class RecorderAudioProcessorEditor  :
    public AudioProcessorEditor
{
public:
    RecorderAudioProcessorEditor (RecorderAudioProcessor&);
    ~RecorderAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
  juce::TextButton playButton;
  RecorderAudioProcessor& processor;
  AudioRecordingComponent recordingComponent;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecorderAudioProcessorEditor)
};
