#pragma once
class SineWaveSound : public SynthesiserSound // A demo synth sound that's just a basic sine wave
{
public:
    SineWaveSound() {}
    bool appliesToNote (int /*midiNoteNumber*/) override    { return true; }
    bool appliesToChannel (int /*midiChannel*/) override    { return true; }
};
/** A simple demo synth voice that just plays a sine wave.. */
class SineWaveVoice   : public SynthesiserVoice
{
public:
    SineWaveVoice() {}
    bool canPlaySound (SynthesiserSound* sound) override {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }
    void startNote (int midiNoteNumber, float velocity,
                    SynthesiserSound* /*sound*/,
                    int /*currentPitchWheelPosition*/) override {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;
        auto cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        auto cyclesPerSample = cyclesPerSecond / getSampleRate();
        angleDelta = cyclesPerSample * MathConstants<double>::twoPi;
    }
    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff) {// Set this flag to start tail-off. 
            // Render callback will pick up on this & do fade out, calling clearCurrentNote() when done
            if (tailOff == 0.0) // Only need to begin tail-off if not already doing so, stopNote method could be called more than once.
                tailOff = 1.0;
        } else {
            // we're being told to stop playing immediately, so reset everything
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved(int /*newValue*/) override {}
    void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override {}
    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta != 0.0) {
            if (tailOff > 0.0) {
                while (--numSamples >= 0) {
                    auto currentSample = (float) (sin (currentAngle) * level * tailOff);
                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;) outputBuffer.addSample (i, startSample, currentSample);
                    currentAngle += angleDelta;
                    ++startSample;
                    tailOff *= 0.99;
                    if (tailOff <= 0.005) {
                        // tells the synth that this voice has stopped
                        clearCurrentNote();
                        angleDelta = 0.0;
                        break;
                    }
                }
            } else {
              while (--numSamples >= 0) {
                auto currentSample = (float) (sin (currentAngle) * level);
                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                  outputBuffer.addSample (i, startSample, currentSample);
                  currentAngle += angleDelta;
                  ++startSample;
              }
            }
        }
    }
    using SynthesiserVoice::renderNextBlock;
private:
    double currentAngle = 0.0;
    double angleDelta   = 0.0;
    double level        = 0.0;
    double tailOff      = 0.0;
};
/** As the name suggest, this class does the actual audio processing. */
class JuceDemoPluginAudioProcessor  : public AudioProcessor
{
public:
    JuceDemoPluginAudioProcessor()
        : AudioProcessor (getBusesProperties()),
          state (*this, nullptr, "state",
                 { std::make_unique<AudioParameterFloat> (ParameterID { "gain",  1 }, "Gain",           NormalisableRange<float> (0.0f, 1.0f), 0.9f),
                   std::make_unique<AudioParameterFloat> (ParameterID { "delay", 1 }, "Delay Feedback", NormalisableRange<float> (0.0f, 1.0f), 0.5f) })
    {
        // Add a sub-tree to store the state of our UI
        state.state.addChild ({ "uiState", { { "width",  400 }, { "height", 200 } }, {} }, -1, nullptr);
        initialiseSynth();
    }

    ~JuceDemoPluginAudioProcessor() override = default;

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        // Only mono/stereo and input/output must have same layout
        const auto& mainOutput = layouts.getMainOutputChannelSet();
        const auto& mainInput  = layouts.getMainInputChannelSet();

        // input and output layout must either be the same or the input must be disabled altogether
        if (! mainInput.isDisabled() && mainInput != mainOutput)
            return false;

        // only allow stereo and mono
        if (mainOutput.size() > 2)
            return false;

        return true;
    }

    void prepareToPlay (double newSampleRate, int /*samplesPerBlock*/) override
    {
        // Use this method as the place to do any pre-playback
        // initialisation that you need..
        synth.setCurrentPlaybackSampleRate (newSampleRate);
        keyboardState.reset();

        if (isUsingDoublePrecision())
        {
            delayBufferDouble.setSize (2, 12000);
            delayBufferFloat .setSize (1, 1);
        }
        else
        {
            delayBufferFloat .setSize (2, 12000);
            delayBufferDouble.setSize (1, 1);
        }
        reset();
    }
    void releaseResources() override {
        // When playback stops, use as an opportunity to free up any spare memory, etc.
        keyboardState.reset();
    }
    void reset() override {
        // Use as place to clear any delay lines, buffers, etc, as it means there's been a break in the audio's continuity.
        delayBufferFloat .clear();
        delayBufferDouble.clear();
    }
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override {
        jassert (! isUsingDoublePrecision());
        process (buffer, midiMessages, delayBufferFloat);
    }
    void processBlock (AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override {
        jassert (isUsingDoublePrecision());
        process (buffer, midiMessages, delayBufferDouble);
    }
    bool hasEditor() const override                                   { return true; }
    AudioProcessorEditor* createEditor() override {
        return new JuceDemoPluginAudioProcessorEditor (*this);
    }
    const String getName() const override                             { return "AudioPluginDemo"; }
    bool acceptsMidi() const override                                 { return true; }
    bool producesMidi() const override                                { return true; }
    double getTailLengthSeconds() const override                      { return 0.0; }
    int getNumPrograms() override                                     { return 0; }
    int getCurrentProgram() override                                  { return 0; }
    void setCurrentProgram (int) override                             {}
    const String getProgramName (int) override                        { return "None"; }
    void changeProgramName (int, const String&) override              {}
    void getStateInformation (MemoryBlock& destData) override {
        // Store an xml representation of our state.
        if (auto xmlState = state.copyState().createXml()) copyXmlToBinary (*xmlState, destData);
    }
    void setStateInformation (const void* data, int sizeInBytes) override { }
    void updateTrackProperties (const TrackProperties& properties) override
    {
      {
         const ScopedLock sl (trackPropertiesLock);
         trackProperties = properties;
      }
      MessageManager::callAsync ([this] {
        if (auto* editor = dynamic_cast<JuceDemoPluginAudioProcessorEditor*> (getActiveEditor())) editor->updateTrackProperties();
      });
    }
    TrackProperties getTrackProperties() const {
        const ScopedLock sl (trackPropertiesLock);
        return trackProperties;
    }
    class SpinLockedPosInfo {
    public:
        // Wait-free, but setting new info may fail if main thread currently calling `get`. Uunlikely 
        // to matter in practice because we'll be calling `set` much more frequently than `get`.
        void set (const AudioPlayHead::PositionInfo& newInfo) {
            const juce::SpinLock::ScopedTryLockType lock (mutex);
            if (lock.isLocked()) info = newInfo;
        }

        AudioPlayHead::PositionInfo get() const noexcept {
            const juce::SpinLock::ScopedLockType lock (mutex);
            return info;
        }
    private:
        juce::SpinLock mutex;
        AudioPlayHead::PositionInfo info;
    };
    // Public properties so editor component can access them

    // Kept up to date with arriving midi messages. UI component registers with it so it can represent incoming messages
    MidiKeyboardState keyboardState;

    // this keeps a copy of the last set of time info that was acquired during an audio
    // callback - the UI component will read this and display it.
    SpinLockedPosInfo lastPosInfo;

    // Our plug-in's current state
    AudioProcessorValueTreeState state;

private:
    //==============================================================================
    /** This is the editor component that our filter will display. */
    class JuceDemoPluginAudioProcessorEditor  : public AudioProcessorEditor,
                                                private Timer,
                                                private Value::Listener
    {
    public:
        JuceDemoPluginAudioProcessorEditor (JuceDemoPluginAudioProcessor& owner)
            : AudioProcessorEditor (owner),
              midiKeyboard         (owner.keyboardState, MidiKeyboardComponent::horizontalKeyboard),
              gainAttachment       (owner.state, "gain",  gainSlider),
              delayAttachment      (owner.state, "delay", delaySlider)
        {
            // add some sliders..
            addAndMakeVisible (gainSlider);
            gainSlider.setSliderStyle (Slider::Rotary);

            addAndMakeVisible (delaySlider);
            delaySlider.setSliderStyle (Slider::Rotary);

            // add some labels for the sliders..
            gainLabel.attachToComponent (&gainSlider, false);
            gainLabel.setFont (Font (11.0f));

            delayLabel.attachToComponent (&delaySlider, false);
            delayLabel.setFont (Font (11.0f));
            addAndMakeVisible (midiKeyboard);// add the midi keyboard component.

            // add a label that will display the current timecode and status..
            addAndMakeVisible (timecodeDisplayLabel);
            timecodeDisplayLabel.setFont (Font (Font::getDefaultMonospacedFontName(), 15.0f, Font::plain));

            // set resize limits for this plug-in
            setResizeLimits (400, 200, 1024, 700);
            setResizable (true, owner.wrapperType != wrapperType_AudioUnitv3);
            lastUIWidth .referTo (owner.state.state.getChildWithName ("uiState").getPropertyAsValue ("width",  nullptr));
            lastUIHeight.referTo (owner.state.state.getChildWithName ("uiState").getPropertyAsValue ("height", nullptr));
            // set our component's initial size to be the last one that was stored in the filter's settings
            setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
            lastUIWidth. addListener (this);
            lastUIHeight.addListener (this);
            updateTrackProperties();
            // start a timer which will keep our timecode display updated
            startTimerHz (30);
        }
        ~JuceDemoPluginAudioProcessorEditor() override {}
        void paint (Graphics& g) override {
            g.setColour (backgroundColour);
            g.fillAll();
        }
        void resized() override {
            // This lays out our child components...
            auto r = getLocalBounds().reduced (8);
            timecodeDisplayLabel.setBounds (r.removeFromTop (26));
            midiKeyboard        .setBounds (r.removeFromBottom (70));
            r.removeFromTop (20);
            auto sliderArea = r.removeFromTop (60);
            gainSlider.setBounds  (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth() / 2)));
            delaySlider.setBounds (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth())));
            lastUIWidth  = getWidth();
            lastUIHeight = getHeight();
        }
        void timerCallback() override {
            updateTimecodeDisplay (getProcessor().lastPosInfo.get());
        }
        void hostMIDIControllerIsAvailable (bool controllerIsAvailable) override {
            midiKeyboard.setVisible (! controllerIsAvailable);
        }
        int getControlParameterIndex (Component& control) override {
            if (&control == &gainSlider) return 0;
            if (&control == &delaySlider) return 1;
            return -1;
        }
        void updateTrackProperties() {
            auto trackColour = getProcessor().getTrackProperties().colour;
            auto& lf = getLookAndFeel();
            backgroundColour = (trackColour == Colour() ? lf.findColour (ResizableWindow::backgroundColourId)
                                                        : trackColour.withAlpha (1.0f).withBrightness (0.266f));
            repaint();
        }
    private:
        MidiKeyboardComponent midiKeyboard;
        Label timecodeDisplayLabel,
              gainLabel  { {}, "Throughput level:" },
              delayLabel { {}, "Delay:" };
        Slider gainSlider, delaySlider;
        AudioProcessorValueTreeState::SliderAttachment gainAttachment, delayAttachment;
        Colour backgroundColour;
        // these are used to persist UI's size - values are stored along with filter's other parameters, & UI 
        // component will update them when it gets resized.
        Value lastUIWidth, lastUIHeight;
        JuceDemoPluginAudioProcessor& getProcessor() const {
            return static_cast<JuceDemoPluginAudioProcessor&> (processor);
        }
        // quick-and-dirty function to format a timecode string
        static String timeToTimecodeString (double seconds) {
            auto millisecs = roundToInt (seconds * 1000.0);
            auto absMillisecs = std::abs (millisecs);
            return String::formatted ("%02d:%02d:%02d.%03d",
                                      millisecs / 3600000, (absMillisecs / 60000) % 60,
                                      (absMillisecs / 1000)  % 60, absMillisecs % 1000);
        }
        // quick-and-dirty function to format a bars/beats string
        static String quarterNotePositionToBarsBeatsString (double quarterNotes, AudioPlayHead::TimeSignature sig)
        {
            if (sig.numerator == 0 || sig.denominator == 0) return "1|1|000";
            auto quarterNotesPerBar = (sig.numerator * 4 / sig.denominator);
            auto beats  = (fmod (quarterNotes, quarterNotesPerBar) / quarterNotesPerBar) * sig.numerator;
            auto bar    = ((int) quarterNotes) / quarterNotesPerBar + 1;
            auto beat   = ((int) beats) + 1;
            auto ticks  = ((int) (fmod (beats, 1.0) * 960.0 + 0.5));
            return String::formatted ("%d|%d|%03d", bar, beat, ticks);
        }
        // Updates the text in our position label.
        void updateTimecodeDisplay (const AudioPlayHead::PositionInfo& pos) {
            MemoryOutputStream displayText;
            const auto sig = pos.getTimeSignature().orFallback (AudioPlayHead::TimeSignature{});
            displayText << "[" << SystemStats::getJUCEVersion() << "]   "
                        << String (pos.getBpm().orFallback (120.0), 2) << " bpm, "
                        << sig.numerator << '/' << sig.denominator
                        << "  -  " << timeToTimecodeString (pos.getTimeInSeconds().orFallback (0.0))
                        << "  -  " << quarterNotePositionToBarsBeatsString (pos.getPpqPosition().orFallback (0.0), sig);
            if (pos.getIsRecording()) displayText << "  (recording)";
            else if (pos.getIsPlaying()) displayText << "  (playing)";
            timecodeDisplayLabel.setText (displayText.toString(), dontSendNotification);
        }
        // called when the stored window size changes
        void valueChanged (Value&) override {
            setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
        }
    };
    template <typename FloatType>
    void process (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages, AudioBuffer<FloatType>& delayBuffer)
    {
        auto gainParamValue  = state.getParameter ("gain") ->getValue();
        auto delayParamValue = state.getParameter ("delay")->getValue();
        auto numSamples = buffer.getNumSamples();
        // Clear any output channels that didn't contain input data
        for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i) buffer.clear (i, 0, numSamples);
        // Now pass any incoming midi messages to our keyboard state object, and let it
        // add messages to the buffer if the user is clicking on the on-screen keys
        keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);
        // and now get our synth to process these midi events and generate its output.
        synth.renderNextBlock (buffer, midiMessages, 0, numSamples);
        // Apply our delay effect to the new output..
        applyDelay (buffer, delayBuffer, delayParamValue);
        // Apply our gain change to the outgoing data..
        applyGain (buffer, delayBuffer, gainParamValue);
        // Now ask the host for the current time so we can store it to be displayed later...
        updateCurrentTimeInfoFromHost();
    }
    template <typename FloatType>
    void applyGain (AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer, float gainLevel) {
        ignoreUnused (delayBuffer);
        for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
            buffer.applyGain (channel, 0, buffer.getNumSamples(), gainLevel);
    }
    template <typename FloatType>
    void applyDelay (AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer, float delayLevel) {
        auto numSamples = buffer.getNumSamples();
        auto delayPos = 0;
        for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel) {
            auto channelData = buffer.getWritePointer (channel);
            auto delayData = delayBuffer.getWritePointer (jmin (channel, delayBuffer.getNumChannels() - 1));
            delayPos = delayPosition;
            for (auto i = 0; i < numSamples; ++i) {
                auto in = channelData[i];
                channelData[i] += delayData[delayPos];
                delayData[delayPos] = (delayData[delayPos] + in) * delayLevel;
                if (++delayPos >= delayBuffer.getNumSamples()) delayPos = 0;
            }
        }
        delayPosition = delayPos;
    }
    AudioBuffer<float> delayBufferFloat;
    AudioBuffer<double> delayBufferDouble;
    int delayPosition = 0;
    Synthesiser synth;
    CriticalSection trackPropertiesLock;
    TrackProperties trackProperties;
    void initialiseSynth() {
        auto numVoices = 8;
        // Add some voices...
        for (auto i = 0; i < numVoices; ++i) synth.addVoice (new SineWaveVoice());
        // ..and give the synth a sound to play
        synth.addSound (new SineWaveSound());
    }
    void updateCurrentTimeInfoFromHost() {
        const auto newInfo = [&] {
            if (auto* ph = getPlayHead())
                if (auto result = ph->getPosition()) return *result;
            // If the host fails to provide the current time, we'll just use default values
            return AudioPlayHead::PositionInfo{};
        }();
        lastPosInfo.set (newInfo);
    }
    static BusesProperties getBusesProperties() {
        return BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                .withOutput ("Output", AudioChannelSet::stereo(), true);
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceDemoPluginAudioProcessor)
};