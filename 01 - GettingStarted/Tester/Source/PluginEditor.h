#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TesterAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
  TesterAudioProcessorEditor (TesterAudioProcessor&);
  ~TesterAudioProcessorEditor() override;
  void paint (juce::Graphics&) override;
  void resized() override;
  void openButtonClicked();
  void playButtonClicked();
  void stopButtonClicked();
  enum TransportState {
    stopped,
    starting,
    stopping,
    playing
  };
  TransportState state = stopped;
  void transportStateChanged(TransportState newState);
private:
  std::unique_ptr<juce::AudioFormatReaderSource> playSource;
  juce::AudioFormatManager formatManager;
  juce::TextButton openButton;
  juce::TextButton playButton;
  juce::TextButton stopButton;

  juce::Slider gainSlider;
  juce::Label gainLabel;
  juce::Slider frequencySlider;
  juce::Label frequencyLabel;
  juce::Slider lfoFrequencySlider;
  juce::Label lfoFrequencyLabel;
  juce::ComboBox testSignalBox;
  juce::Label testSignalLabel;
  juce::ComboBox channelBox;
  juce::Label channelLabel;

  TesterAudioProcessor& audioProcessor;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TesterAudioProcessorEditor)
};
