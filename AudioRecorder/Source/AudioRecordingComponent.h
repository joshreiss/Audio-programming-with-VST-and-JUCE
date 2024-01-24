#pragma once
#include "PluginProcessor.h"
class AudioRecordingComponent  : public Component {
public:
    AudioRecordingComponent (RecorderAudioProcessor& p) : processor(p) {
      recordButton.setButtonText("Play");
      recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
      recordButton.setEnabled(true);
      recordButton.setClickingTogglesState(true);
      recordButton.setButtonText("Record");
      recordButton.onClick = [this] {
          if (processor.isRecording()) stopRecording();
          else {
            juce::FileChooser chooser("Save audio file", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav");
            //if user chooses a file
            if (chooser.browseForFileToSave(true)) {
              myFile = chooser.getResult();
              DBG("Created " << myFile.getFullPathName());
            }
            startRecording();
          }
        };
        addAndMakeVisible(recordButton);
    }
    ~AudioRecordingComponent() override  { }    
    void paint (Graphics& g) override { g.fillAll (Colour (0xff1d1d33)); }
    void resized() override {
        recordButton.setBounds(0, 0, 90, 80);
    }
private:
    RecorderAudioProcessor& processor;
    juce::TextButton recordButton;
    File lastRecording;
    juce::File myFile;
    void startRecording() {
      lastRecording = myFile;
      processor.startRecording(lastRecording);
      recordButton.setButtonText("Stop");
    }
    void stopRecording() {
        processor.stop();
        recordButton.setButtonText("Record");
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioRecordingComponent)
};
