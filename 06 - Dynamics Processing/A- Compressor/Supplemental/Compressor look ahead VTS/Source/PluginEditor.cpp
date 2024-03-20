/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CompressorAudioProcessorEditor::CompressorAudioProcessorEditor (CompressorAudioProcessor& p, AudioProcessorValueTreeState& state)
    : AudioProcessorEditor (&p), audioProcessor (p), params(state)
{
  addSlider("thresh", "Threshold", threshSlider, threshLabel, threshAttachment);
  addSlider("ratio", "Ratio", slopeSlider, slopeLabel, slopeAttachment);
  addSlider("knee", "Knee", kneeSlider, kneeLabel, kneeAttachment);
  addSlider("attack", "Attack", attackSlider, attackLabel, attackAttachment);
  addSlider("release", "Release", releaseSlider, releaseLabel, releaseAttachment);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

CompressorAudioProcessorEditor::~CompressorAudioProcessorEditor()
{
}

//==============================================================================
void CompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void CompressorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
  threshSlider.setBounds(100, 0, 200, 50);
  slopeSlider.setBounds(100, 50, 200, 50);
  kneeSlider.setBounds(100, 100, 200, 50);
  attackSlider.setBounds(100, 150, 200, 50);
  releaseSlider.setBounds(100, 200, 200, 50);
}

void CompressorAudioProcessorEditor::addSlider(String name, String labelText, Slider& slider, Label& label, std::unique_ptr<SliderAttachment>& attachment) {
  addAndMakeVisible(slider);
  attachment.reset(new SliderAttachment(params, name, slider));

  label.setText(labelText, dontSendNotification);
  label.attachToComponent(&slider, true);
  addAndMakeVisible(label);
}

