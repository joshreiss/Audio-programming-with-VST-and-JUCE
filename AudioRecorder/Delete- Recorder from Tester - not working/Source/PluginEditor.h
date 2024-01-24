#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class RecorderAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
  RecorderAudioProcessorEditor (RecorderAudioProcessor&);
  ~RecorderAudioProcessorEditor() override;
  void paint (juce::Graphics&) override;
  void resized() override;
  void openButtonClicked();
private:
  std::unique_ptr<juce::AudioFormatReaderSource> playSource;
  juce::AudioFormatManager formatManager;
  juce::TextButton openButton;
  juce::TextButton recordButton;
  juce::TextButton stopButton;

  RecorderAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecorderAudioProcessorEditor)
};
