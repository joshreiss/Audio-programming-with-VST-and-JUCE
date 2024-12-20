/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SaveState2AudioProcessorEditor::SaveState2AudioProcessorEditor (SaveState2AudioProcessor& p, AudioProcessorValueTreeState& state)
    : AudioProcessorEditor (&p), audioProcessor (p), params(state)
{
  addSlider("gain", "Gain", gainSlider, gainLabel, gainAttachment);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

SaveState2AudioProcessorEditor::~SaveState2AudioProcessorEditor()
{
}

//==============================================================================
void SaveState2AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void SaveState2AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
  gainSlider.setBounds(100, 0, 200, 50);
}

void SaveState2AudioProcessorEditor::addSlider(String name, String labelText, Slider& slider, Label& label, std::unique_ptr<SliderAttachment>& attachment) {
  addAndMakeVisible(slider);
  attachment.reset(new SliderAttachment(params, name, slider));

  label.setText(labelText, dontSendNotification);
  label.attachToComponent(&slider, true);
  addAndMakeVisible(label);
}

