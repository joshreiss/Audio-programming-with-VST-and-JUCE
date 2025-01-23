/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloWorld_v3AudioProcessorEditor::HelloWorld_v3AudioProcessorEditor (HelloWorld_v3AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
<<<<<<<< HEAD:01 GettingStarted/2c UI- no AudioParam/Source/PluginEditor.cpp
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    gainSlider.setRange(0.0, 1.0, 0.01);
    gainSlider.setValue(0.5);
    gainSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 60, 30);
    addAndMakeVisible(&gainSlider);
    gainSlider.addListener(this);
========
    addAndMakeVisible(&myKeyboard);
    setSize(600, 450);
>>>>>>>> 7116c287bc7bdd20ebb6592c6c9423ce26d51d16:02 Midi/03 MonoSynth/Source/PluginEditor.cpp
}

HelloWorld_v3AudioProcessorEditor::~HelloWorld_v3AudioProcessorEditor()
{
}

//==============================================================================
void HelloWorld_v3AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void HelloWorld_v3AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
<<<<<<<< HEAD:01 GettingStarted/2c UI- no AudioParam/Source/PluginEditor.cpp
  gainSlider.setBounds(40, 30, 160, 50);
}
//PluginEditor.cpp
void HelloWorld_v3AudioProcessorEditor::sliderValueChanged(Slider* slider)
{
  audioProcessor.gainParam = gainSlider.getValue();
}
========
  myKeyboard.setBounds(20, 100, 350, 140);
}
>>>>>>>> 7116c287bc7bdd20ebb6592c6c9423ce26d51d16:02 Midi/03 MonoSynth/Source/PluginEditor.cpp
