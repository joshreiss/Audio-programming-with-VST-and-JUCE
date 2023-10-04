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
class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


    static juce::String getMidiMessageDescription(const juce::MidiMessage& m);
    void setNoteNumber(int noteNumber);
    void logMessage(const juce::String& m);
    void addMessageToList(const juce::MidiMessage& message);
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;

    juce::TextButton bassDrumButton;
    juce::TextButton snareDrumButton;
    juce::TextButton closedHiHatButton;
    juce::TextButton openHiHatButton;
    juce::Label volumeLabel;
    juce::Slider volumeSlider;
    juce::TextEditor midiMessagesBox;
    int midiChannel = 10;
    double startTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
