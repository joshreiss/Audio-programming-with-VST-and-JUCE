/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    float gainParam;

    MidiKeyboardState keyboardState;
    Synthesiser synth;
    float phase = 0.0;
    float frequency = 0.0;
    float amplitude = 0.0;
    float mySampleRate = 44100;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};
class SineWaveSound : public SynthesiserSound // A demo synth sound that's just a basic sine wave
{
public:
  SineWaveSound() {}
  bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
  bool appliesToChannel(int /*midiChannel*/) override { return true; }
};
class SineWaveVoice : public SynthesiserVoice { // Simple demo synth voice that plays sine wave
public:
  SineWaveVoice() {}
  void pitchWheelMoved(int) override {}
  void controllerMoved(int, int) override {}
  bool canPlaySound(SynthesiserSound* sound) override { return dynamic_cast<SineWaveSound*> (sound) != nullptr; }
  void startNote(int midiNoteNumber, float velocity, SynthesiserSound* /*sound*/, int) override {
    currentAngle = 0.0;
    level = velocity * 0.15;
    angleDelta = MathConstants<double>::twoPi * MidiMessage::getMidiNoteInHertz(midiNoteNumber) / getSampleRate();
  }
  void stopNote(float /*velocity*/, bool allowTailOff) override {
    clearCurrentNote();
    angleDelta = 0.0;
  }
  void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
    if (angleDelta != 0.0) {
      while (--numSamples >= 0) {
        auto currentSample = (float)(sin(currentAngle) * level);
        for (auto i = outputBuffer.getNumChannels(); --i >= 0;) outputBuffer.addSample(i, startSample, currentSample);
        currentAngle += angleDelta;
        ++startSample;
      }
    }
  }
private:
  double currentAngle = 0.0, angleDelta = 0.0, level = 0.0;
};
