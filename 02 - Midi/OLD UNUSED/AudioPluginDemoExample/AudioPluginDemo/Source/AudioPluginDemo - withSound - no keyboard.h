#pragma once
class SineWaveSound : public SynthesiserSound // A demo synth sound that's just a basic sine wave
{
public:
    SineWaveSound() {}
    bool appliesToNote (int /*midiNoteNumber*/) override    { return true; }
    bool appliesToChannel (int /*midiChannel*/) override    { return true; }
};
class SineWaveVoice   : public SynthesiserVoice { // Simple demo synth voice that plays sine wave
public:
    SineWaveVoice() {}
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    bool canPlaySound (SynthesiserSound* sound) override { return dynamic_cast<SineWaveSound*> (sound) != nullptr; }
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* /*sound*/, int) override {
       currentAngle = 0.0;
       level = velocity * 0.15;
       angleDelta = MathConstants<double>::twoPi * MidiMessage::getMidiNoteInHertz(midiNoteNumber) / getSampleRate();
    }
    void stopNote (float /*velocity*/, bool allowTailOff) override {        
      clearCurrentNote();
      angleDelta = 0.0;
    }
    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
      if (angleDelta != 0.0) {
        while (--numSamples >= 0) {
          auto currentSample = (float) (sin (currentAngle) * level);
          for (auto i = outputBuffer.getNumChannels(); --i >= 0;) outputBuffer.addSample (i, startSample, currentSample);
          currentAngle += angleDelta;
          ++startSample;
        }
      }
    }
private:
    double currentAngle = 0.0, angleDelta = 0.0, level = 0.0;
};
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
    void prepareToPlay(double newSampleRate, int /*samplesPerBlock*/) override {
    //  synth.addVoice(new SineWaveVoice());
    //  synth.addSound(new SineWaveSound());
    //  synth.setCurrentPlaybackSampleRate(newSampleRate);
      keyboardState.reset();

      auto message = juce::MidiMessage::noteOn(1, 100, (juce::uint8)100); //???
      reset();
    }
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override {
      auto numSamples = buffer.getNumSamples();
      for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i) buffer.clear(i, 0, numSamples);
      // Pass incoming midi messages to keyboard state object, adds messages to buffer if user clicking on on-screen keys
      keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);
//      synth.renderNextBlock(buffer, midiMessages, 0, numSamples); // Synth processes these midi events & generate output
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
    Synthesiser synth;
    static BusesProperties getBusesProperties() {
        return BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                .withOutput ("Output", AudioChannelSet::stereo(), true);
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceDemoPluginAudioProcessor)
};
