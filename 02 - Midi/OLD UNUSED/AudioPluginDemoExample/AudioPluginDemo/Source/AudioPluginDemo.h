#pragma once
class JuceDemoPluginAudioProcessor  : public AudioProcessor { // This class does the audio processing
public:
    JuceDemoPluginAudioProcessor() : AudioProcessor (getBusesProperties()) { }
    ~JuceDemoPluginAudioProcessor() override = default;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override { return true; }
    void releaseResources() override {}
    void reset() override { }
    bool hasEditor() const override                                   { return true; }
    AudioProcessorEditor* createEditor() override { return new JuceDemoPluginAudioProcessorEditor (*this);}
    const String getName() const override                             { return "AudioPluginDemo"; }
    bool acceptsMidi() const override                                 { return true; }
    bool producesMidi() const override                                { return true; }
    double getTailLengthSeconds() const override                      { return 0.0; }
    int getNumPrograms() override                                     { return 0; }
    int getCurrentProgram() override                                  { return 0; }
    void setCurrentProgram (int) override                             {}
    const String getProgramName (int) override                        { return "None"; }
    void changeProgramName (int, const String&) override              {}
    void getStateInformation (MemoryBlock& destData) override         {}
    void setStateInformation (const void* data, int sizeInBytes) override { }
    void prepareToPlay(double newSampleRate, int /*samplesPerBlock*/) override { }
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override {
      auto numSamples = buffer.getNumSamples();
      for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i) buffer.clear(i, 0, numSamples);
      // Pass incoming midi messages to keyboard state object, adds messages to buffer if user clicking on on-screen keys
      keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);
      for (const auto metadata : midiMessages) if (metadata.getMessage().isNoteOn()) DBG("Note On");
    }
    void updateTrackProperties(const TrackProperties& properties) override {}
    // Kept up to date with arriving midi messages. UI component registers with it so it can represent incoming messages
    MidiKeyboardState keyboardState;    
private:
    class JuceDemoPluginAudioProcessorEditor  : public AudioProcessorEditor, private Value::Listener {
    public:
      JuceDemoPluginAudioProcessorEditor (JuceDemoPluginAudioProcessor& owner)
          : AudioProcessorEditor (owner), midiKeyboard (owner.keyboardState, MidiKeyboardComponent::horizontalKeyboard)
      {
          addAndMakeVisible (midiKeyboard);// add the midi keyboard component.
          setSize (500,200);
      }
      ~JuceDemoPluginAudioProcessorEditor() override {}
      void paint (Graphics& g) override { }
      int getControlParameterIndex(Component& control) override { return -1; }
      void resized() override {
        auto r = getLocalBounds().reduced (8);
        midiKeyboard .setBounds (r.removeFromBottom (70));
        r.removeFromTop (20);
      }
      void hostMIDIControllerIsAvailable (bool controllerIsAvailable) override {
        midiKeyboard.setVisible (! controllerIsAvailable);
      }
    private:
        MidiKeyboardComponent midiKeyboard;
        JuceDemoPluginAudioProcessor& getProcessor() const { return static_cast<JuceDemoPluginAudioProcessor&> (processor); }
        void valueChanged (Value&) override { } // called when the stored window size changes
    };
    static BusesProperties getBusesProperties() {
        return BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                .withOutput ("Output", AudioChannelSet::stereo(), true);
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceDemoPluginAudioProcessor)
};
