#include "PluginProcessor.h"
#include "PluginEditor.h"

RecorderAudioProcessorEditor::RecorderAudioProcessorEditor (RecorderAudioProcessor& p)
  : AudioProcessorEditor(&p), audioProcessor(p)
{
  addAndMakeVisible(openButton);
  openButton.setButtonText("Open");
  openButton.onClick = [this] { 
    openButtonClicked(); 
  };

  recordButton.setButtonText("Record");
  recordButton.onClick = [this] { 
    stopButton.setEnabled(true);
    recordButton.setEnabled(false);
    audioProcessor.transport.setPosition(0.0);
    audioProcessor.transport.start(); 
  };
  recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
  recordButton.setEnabled(true);
  addAndMakeVisible(&recordButton);

  stopButton.setButtonText("Stop");
  stopButton.onClick = [this] {
    recordButton.setEnabled(true);
    stopButton.setEnabled(false);
    audioProcessor.transport.stop();
  };
  stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
  stopButton.setEnabled(false);
  addAndMakeVisible(&stopButton);
  setSize(400, 160);

  formatManager.registerBasicFormats();
}
RecorderAudioProcessorEditor::~RecorderAudioProcessorEditor()
{
}
void RecorderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}
void RecorderAudioProcessorEditor::resized()
{
  openButton.setBounds(10, 50, 80, 30);
  recordButton.setBounds(120, 50, 100, 30);
  stopButton.setBounds(230, 50, 100, 30);
}
void RecorderAudioProcessorEditor::openButtonClicked()
{
  DBG("clicked");  
  //choose a file
  juce::FileChooser chooser("Choose an audio file", 
    juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav");

  //if the user chooses a file
  if (chooser.browseForFileToOpen()) {
    juce::File myFile;
    //what did the user choose?
    myFile = chooser.getResult();
    //read the file
    juce::AudioFormatReader* reader = formatManager.createReaderFor(myFile);
    if (reader != nullptr) {
      //get the file ready to play
      std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));

      audioProcessor.transport.setSource(tempSource.get());
      recordButton.setEnabled(true);
      audioProcessor.transport.setPosition(0.0);
      playSource.reset(tempSource.release());
      DBG("Format name is " << reader->getFormatName());
    }
  }
}