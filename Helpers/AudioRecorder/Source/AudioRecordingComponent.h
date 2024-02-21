#pragma once
#include "PluginProcessor.h"
class AudioRecordingComponent  : public Component {
public:
    AudioRecordingComponent (RecorderAudioProcessor& p) : processor(p) {
      recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
      recordButton.setEnabled(true);
      recordButton.setClickingTogglesState(true);
      recordButton.setButtonText("Record");
      recordButton.onClick = [this] {
        if (processor.isRecording()) {
          processor.stop();
          recordButton.setButtonText("Record");
          recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
        }
        else {
          juce::FileChooser chooser("Save audio file", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav");
          if (chooser.browseForFileToSave(true))
          {
            myFile = chooser.getResult();
            lastRecording = myFile;
            processor.startRecording(lastRecording);
            recordButton.setButtonText("Stop");
            recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
          }
        }
      };
      addAndMakeVisible(recordButton);
    }
    ~AudioRecordingComponent() override  { }    
    void paint (Graphics& g) override { 
      g.fillAll (Colour (0xff1d1d33)); 
    }
    void resized() override {
        recordButton.setBounds(0, 0, 90, 80);
    }
private:
    RecorderAudioProcessor& processor;
    juce::TextButton recordButton;
    File lastRecording;
    juce::File myFile;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioRecordingComponent)
};
