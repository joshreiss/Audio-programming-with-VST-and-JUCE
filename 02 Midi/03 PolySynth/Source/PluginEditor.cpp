/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PolySynthAudioProcessorEditor::PolySynthAudioProcessorEditor (PolySynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
  , myKeyboard(audioProcessor.keyboardState, juce::MidiKeyboardComponent::Orientation::horizontalKeyboard)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 450);
    addAndMakeVisible(&myKeyboard);
}

PolySynthAudioProcessorEditor::~PolySynthAudioProcessorEditor()
{
}

//==============================================================================
void PolySynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void PolySynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
  myKeyboard.setBounds(20, 100, 350, 140);
}
