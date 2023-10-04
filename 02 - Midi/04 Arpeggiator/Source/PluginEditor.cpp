#include "PluginProcessor.h"
#include "PluginEditor.h"

ArpeggiatorAudioProcessorEditor::ArpeggiatorAudioProcessorEditor (ArpeggiatorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

  setSize (400, 300);
}

ArpeggiatorAudioProcessorEditor::~ArpeggiatorAudioProcessorEditor()
{
}

//==============================================================================
void ArpeggiatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ArpeggiatorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
