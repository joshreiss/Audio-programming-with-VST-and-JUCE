#include "PluginProcessor.h"
#include "PluginEditor.h"

TesterAudioProcessorEditor::TesterAudioProcessorEditor (TesterAudioProcessor& p)
  : AudioProcessorEditor(&p), audioProcessor(p)
{
  addAndMakeVisible(openButton);
  openButton.setButtonText("Open");
  openButton.onClick = [this] { openButtonClicked(); };

  playButton.setButtonText("Play");
  playButton.onClick = [this] { playButtonClicked(); };
  playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
  playButton.setEnabled(true);
  addAndMakeVisible(&playButton);

  stopButton.setButtonText("Stop");
  stopButton.onClick = [this] { stopButtonClicked(); };
  stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
  stopButton.setEnabled(false);
  addAndMakeVisible(&stopButton);
  setSize(400, 300);

  addAndMakeVisible(&testSignalBox); // 1
  testSignalBox.addItem("Noise", 1);
  testSignalBox.addItem("Sinusoid", 2);
  testSignalBox.addItem("Pulse wave", 3);
  testSignalBox.addItem("Play file", 4);
  testSignalBox.onChange = [this] { audioProcessor.testSignal = testSignalBox.getSelectedItemIndex(); };
  testSignalBox.setSelectedId(1);
  addAndMakeVisible(testSignalLabel);
  testSignalLabel.setText("Test signal", juce::dontSendNotification);
  testSignalLabel.attachToComponent(&testSignalBox, true);

  frequencySlider.setRange(20.0, 2000.0, 1.0);
  frequencySlider.setValue(1000.0);
  frequencySlider.onValueChange = [this] { audioProcessor.frequencyValue = frequencySlider.getValue(); };
  addAndMakeVisible(&frequencySlider);
  addAndMakeVisible(frequencyLabel);
  frequencyLabel.setText("Frequency", juce::dontSendNotification);
  frequencyLabel.attachToComponent(&frequencySlider, true);

  gainSlider.setRange(0.0, 1.0, 0.01);
  gainSlider.setValue(0.5);
  gainSlider.onValueChange = [this] { audioProcessor.gainValue = gainSlider.getValue(); };
  addAndMakeVisible(&gainSlider);
  addAndMakeVisible(gainLabel);
  gainLabel.setText("Gain", juce::dontSendNotification);
  gainLabel.attachToComponent(&gainSlider, true);

  lfoFrequencySlider.setRange(0.2, 5.0, 0.1);
  lfoFrequencySlider.setValue(1.0);
  lfoFrequencySlider.onValueChange = [this] { audioProcessor.lfoFrequencyValue = lfoFrequencySlider.getValue(); };
  addAndMakeVisible(&lfoFrequencySlider);
  addAndMakeVisible(lfoFrequencyLabel);
  lfoFrequencyLabel.setText("LFO frequency", juce::dontSendNotification);
  lfoFrequencyLabel.attachToComponent(&lfoFrequencySlider, true);

  addAndMakeVisible(&channelBox); 
  channelBox.addItem("Left", 1);
  channelBox.addItem("Centre", 2);
  channelBox.addItem("Right", 3);
  channelBox.onChange = [this] { audioProcessor.channelValue = channelBox.getSelectedItemIndex(); };
  channelBox.setSelectedId(2);
  addAndMakeVisible(channelLabel);
  channelLabel.setText("Channel", juce::dontSendNotification);
  channelLabel.attachToComponent(&channelBox, true);

  formatManager.registerBasicFormats();
  //audioProcessor.transport.addChangeListener(this);
}

TesterAudioProcessorEditor::~TesterAudioProcessorEditor()
{
}
void TesterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}
void TesterAudioProcessorEditor::resized()
{
  testSignalBox.setBounds(80, 10, getWidth() - 130, 30);
  gainSlider.setBounds(80, 50, getWidth() - 90, 30);
  frequencySlider.setBounds(80, 90, getWidth() - 90, 30);
  lfoFrequencySlider.setBounds(80, 130, getWidth() - 90, 30);
  channelBox.setBounds(80, 170, getWidth() - 130, 30);
  openButton.setBounds(10, 210, 80, 30);
  playButton.setBounds(120, 210, 100, 30);
  stopButton.setBounds(230, 210, 100, 30);
}
void TesterAudioProcessorEditor::openButtonClicked()
{
  DBG("clicked");  

  //choose a file
  juce::FileChooser chooser("Choose an audio file", 
    juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav,*.mp3");

  //if the user chooses a file
  if (chooser.browseForFileToOpen())
  {
    juce::File myFile;

    //what did the user choose?
    myFile = chooser.getResult();

    //read the file
    juce::AudioFormatReader* reader = formatManager.createReaderFor(myFile);

    if (reader != nullptr)
    {
      //get the file ready to play
      std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));

      audioProcessor.transport.setSource(tempSource.get());
      transportStateChanged(stopped);

      playSource.reset(tempSource.release());
      DBG("Format name is " << reader->getFormatName());
    }
  }
}
void TesterAudioProcessorEditor::playButtonClicked()
{
  transportStateChanged(starting);
}
void TesterAudioProcessorEditor::stopButtonClicked()
{
  transportStateChanged(stopping);
}


void TesterAudioProcessorEditor::transportStateChanged(TransportState newState)
{
  if (newState != state) {
    state = newState;

    switch (state) {
    case stopped:
      playButton.setEnabled(true);
      audioProcessor.transport.setPosition(0.0);
      break;
    case playing:
      playButton.setEnabled(true);
      break;
    case starting:
      stopButton.setEnabled(true);
      playButton.setEnabled(false);
      audioProcessor.transport.setPosition(0.0);
      audioProcessor.transport.start();
      break;
    case stopping:
      playButton.setEnabled(true);
      stopButton.setEnabled(false);
      audioProcessor.transport.stop();
      break;
    }
  }
}
