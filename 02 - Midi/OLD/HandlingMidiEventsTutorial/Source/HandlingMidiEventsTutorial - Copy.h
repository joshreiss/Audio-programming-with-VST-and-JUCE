#pragma once
class MainContentComponent  : public juce::Component, private juce::MidiInputCallback, private juce::MidiKeyboardStateListener
{
public:
    MainContentComponent()
      : keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
        startTime (juce::Time::getMillisecondCounterHiRes() * 0.001)
    {
        setOpaque (true);
        addAndMakeVisible (midiInputListLabel);
        midiInputListLabel.setText ("MIDI Input:", juce::dontSendNotification);
        midiInputListLabel.attachToComponent (&midiInputList, true);
        addAndMakeVisible (midiInputList);
        midiInputList.setTextWhenNoChoicesAvailable ("No MIDI Inputs Enabled");
        auto midiInputs = juce::MidiInput::getAvailableDevices();
        juce::StringArray midiInputNames;
        for (auto input : midiInputs) midiInputNames.add (input.name);
        midiInputList.addItemList (midiInputNames, 1);
        midiInputList.onChange = [this] { setMidiInput (midiInputList.getSelectedItemIndex()); };

        // find the first enabled device and use that by default
        for (auto input : midiInputs)
        {
            if (deviceManager.isMidiInputDeviceEnabled (input.identifier))
            {
                setMidiInput (midiInputs.indexOf (input));
                break;
            }
        }

        // if no enabled devices were found just use the first one in the list
        if (midiInputList.getSelectedId() == 0) setMidiInput (0);

        addAndMakeVisible (keyboardComponent);
        keyboardState.addListener (this);

        addAndMakeVisible (midiMessagesBox);
        midiMessagesBox.setMultiLine (true);
        midiMessagesBox.setReturnKeyStartsNewLine (true);
        midiMessagesBox.setReadOnly (true);
        midiMessagesBox.setScrollbarsShown (true);
        midiMessagesBox.setCaretVisible (false);
        midiMessagesBox.setPopupMenuEnabled (true);
        midiMessagesBox.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x32ffffff));
        midiMessagesBox.setColour (juce::TextEditor::outlineColourId, juce::Colour (0x1c000000));
        midiMessagesBox.setColour (juce::TextEditor::shadowColourId, juce::Colour (0x16000000));
        setSize (600, 400);
    }
    ~MainContentComponent() override {
        keyboardState.removeListener (this);
        deviceManager.removeMidiInputDeviceCallback (juce::MidiInput::getAvailableDevices()[midiInputList.getSelectedItemIndex()].identifier, this);
    }
    void paint (juce::Graphics& g) override { g.fillAll (juce::Colours::black); }
    void resized() override {
        auto area = getLocalBounds();
        midiInputList    .setBounds (area.removeFromTop (36).removeFromRight (getWidth() - 150).reduced (8));
        keyboardComponent.setBounds (area.removeFromTop (80).reduced(8));
        midiMessagesBox  .setBounds (area.reduced (8));
    }
private:
    static juce::String getMidiMessageDescription (const juce::MidiMessage& m)
    {
        if (m.isNoteOn())           return "Note on "          + juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
        if (m.isNoteOff())          return "Note off "         + juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
        if (m.isProgramChange())    return "Program change "   + juce::String (m.getProgramChangeNumber());
        if (m.isPitchWheel())       return "Pitch wheel "      + juce::String (m.getPitchWheelValue());
        if (m.isAftertouch())       return "After touch "      + juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) +  ": " + juce::String (m.getAfterTouchValue());
        if (m.isChannelPressure())  return "Channel pressure " + juce::String (m.getChannelPressureValue());
        if (m.isAllNotesOff())      return "All notes off";
        if (m.isAllSoundOff())      return "All sound off";
        if (m.isMetaEvent())        return "Meta event";
        if (m.isMidiClock())        return "Clock";

        if (m.isController())
        {
            juce::String name (juce::MidiMessage::getControllerName (m.getControllerNumber()));

            if (name.isEmpty())
                name = "[" + juce::String (m.getControllerNumber()) + "]";

            return "Controller " + name + ": " + juce::String (m.getControllerValue());
        }

        return"ARGH " + juce::String::toHexString(m.getRawData(), m.getRawDataSize());
    }
    void logMessage (const juce::String& m)
    {
        midiMessagesBox.moveCaretToEnd();
        midiMessagesBox.insertTextAtCaret (m + juce::newLine);
    }
    /** Starts listening to a MIDI input device, enabling it if necessary. */
    void setMidiInput (int index) {
      auto list = juce::MidiInput::getAvailableDevices();
      deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, this);
      auto newInput = list[index];
      if (! deviceManager.isMidiInputDeviceEnabled (newInput.identifier)) deviceManager.setMidiInputDeviceEnabled (newInput.identifier, true);
      deviceManager.addMidiInputDeviceCallback (newInput.identifier, this);
      midiInputList.setSelectedId (index + 1, juce::dontSendNotification);
      lastInputIndex = index;
    }
    // These methods handle callbacks from the midi device + on-screen keyboard..
    void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message) override {
        const juce::ScopedValueSetter<bool> scopedInputFlag (isAddingFromMidiInput, true);
        keyboardState.processNextMidiEvent (message);
        postMessageToList (message, source->getName());
    }
    void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override {
        if (! isAddingFromMidiInput) {
          auto m = juce::MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity);
          m.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
          postMessageToList (m, "On-Screen Keyboard");
        }
    }
    void handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override {
        if (! isAddingFromMidiInput) {
            auto m = juce::MidiMessage::noteOff (midiChannel, midiNoteNumber);
            m.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
            postMessageToList (m, "On-Screen Keyboard");
        }
    }
    // This is used to dispach an incoming message to the message thread
    class IncomingMessageCallback  : public juce::CallbackMessage
    {
    public:
        IncomingMessageCallback (MainContentComponent* o, const juce::MidiMessage& m, const juce::String& s)
           : owner (o), message (m), source (s)
        {}
        void messageCallback() override { if (owner != nullptr) owner->addMessageToList (message, source); }
        Component::SafePointer<MainContentComponent> owner;
        juce::MidiMessage message;
        juce::String source;
    };
    void postMessageToList (const juce::MidiMessage& message, const juce::String& source) {
        (new IncomingMessageCallback (this, message, source))->post();
    }
    void addMessageToList (const juce::MidiMessage& message, const juce::String& source) {
        auto time = message.getTimeStamp() - startTime;
        auto hours   = ((int) (time / 3600.0)) % 24;
        auto minutes = ((int) (time / 60.0)) % 60;
        auto seconds = ((int) time) % 60;
        auto millis  = ((int) (time * 1000.0)) % 1000;
        auto timecode = juce::String::formatted ("%02d:%02d:%02d.%03d",hours,minutes,seconds,millis);
        auto description = getMidiMessageDescription (message);
        juce::String midiMessageString (timecode + "  -  " + description + " (" + source + ")"); // [7]
        if (description != "Clock") logMessage(midiMessageString);
    }
    juce::AudioDeviceManager deviceManager;           // [1]
    juce::ComboBox midiInputList;                     // [2]
    juce::Label midiInputListLabel;
    int lastInputIndex = 0;                           // [3]
    bool isAddingFromMidiInput = false;               // [4]
    juce::MidiKeyboardState keyboardState;            // [5]
    juce::MidiKeyboardComponent keyboardComponent;    // [6]
    juce::TextEditor midiMessagesBox;
    double startTime;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
