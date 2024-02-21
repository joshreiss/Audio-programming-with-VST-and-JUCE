#include "PluginProcessor.h"
#include "PluginEditor.h"

RecorderAudioProcessorEditor::RecorderAudioProcessorEditor (RecorderAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), recordingComponent (p)
{
    addAndMakeVisible(recordingComponent);

    /*
    playButton.setButtonText("Play");
    playButton.onClick = [this] { playButton.setButtonText("Record!"); };
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(true);
    addAndMakeVisible(&playButton);
    */
    setSize(130, 130);
}
RecorderAudioProcessorEditor::~RecorderAudioProcessorEditor() { }
void RecorderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}
void RecorderAudioProcessorEditor::resized()
{
    recordingComponent.setBounds(15, 30, 90, 80);
    //playButton.setBounds(120, 50, 100, 30);
}
