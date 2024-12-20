/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MidiInputAudioProcessorEditor::MidiInputAudioProcessorEditor (MidiInputAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
  , myKeyboard(audioProcessor.keyboardState, juce::MidiKeyboardComponent::Orientation::horizontalKeyboard)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 450);
    gainSlider.setRange(0.0, 1.0, 0.01);
    gainSlider.setValue(0.5);
    gainSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 60, 30);
    addAndMakeVisible(&gainSlider);
    gainSlider.addListener(this);
    addAndMakeVisible(&myKeyboard);
}

MidiInputAudioProcessorEditor::~MidiInputAudioProcessorEditor()
{
}

//==============================================================================
void MidiInputAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void MidiInputAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
  gainSlider.setBounds(40, 30, 160, 50);
  myKeyboard.setBounds(20, 100, 350, 140);
}
//PluginEditor.cpp
void MidiInputAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
  audioProcessor.gainParam = gainSlider.getValue();
}
