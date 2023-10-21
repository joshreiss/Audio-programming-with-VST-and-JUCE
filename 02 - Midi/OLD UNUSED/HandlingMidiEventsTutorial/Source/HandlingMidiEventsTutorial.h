#pragma once
class MainContentComponent  : public juce::Component, private juce::MidiInputCallback, private juce::MidiKeyboardStateListener {
public:
  MainContentComponent() : keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard) {
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    juce::StringArray midiInputNames;
    for (auto input : midiInputs) midiInputNames.add (input.name); 
    setMidiInput (0);// if no enabled devices found, use first one in list
    addAndMakeVisible (keyboardComponent);
    keyboardState.addListener (this);
    setSize (600, 200);
  }
  ~MainContentComponent() override { }
  void paint (juce::Graphics& g) override { g.fillAll (juce::Colours::black); }
  void resized() override { keyboardComponent.setBounds (getLocalBounds().removeFromTop (80).reduced(8)); }
private:
  static juce::String getMidiMessageDescription (const juce::MidiMessage& m) {
    if (m.isNoteOn())           return "Note on "          + juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
    if (m.isNoteOff())          return "Note off "         + juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
    if (m.isMidiClock())        return "Clock";
    return"ARGH";
  }    
  void setMidiInput (int index) {// Start listening to MIDI input device, enable it if needed
    auto list = juce::MidiInput::getAvailableDevices();
    deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, this);
    auto newInput = list[index];
    if (!deviceManager.isMidiInputDeviceEnabled (newInput.identifier)) deviceManager.setMidiInputDeviceEnabled(newInput.identifier, true);
    deviceManager.addMidiInputDeviceCallback (newInput.identifier, this);
    lastInputIndex = index;
  }
  // These methods handle callbacks from the midi device + on-screen keyboard..
  void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message) override {
    const juce::ScopedValueSetter<bool> scopedInputFlag (isAddingFromMidiInput, true);
    keyboardState.processNextMidiEvent (message);
    auto description = getMidiMessageDescription(message);
    if (description != "Clock") DBG("A " << description << " " << source->getName());
  }
  void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override {
    DBG("adsfsd");
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
  // Used to dispach incoming message to message thread
  class IncomingMessageCallback  : public juce::CallbackMessage {
  public:
    IncomingMessageCallback (MainContentComponent* o, const juce::MidiMessage& m, const juce::String& s): owner(o),message(m),source(s) {}
    void messageCallback() override {
      auto description = getMidiMessageDescription(message);
      if (description != "Clock") DBG("B " << description << " " << source);
    }
    Component::SafePointer<MainContentComponent> owner;
    juce::MidiMessage message;
    juce::String source;
  };
  void postMessageToList(const juce::MidiMessage& m, const juce::String& s) {(new IncomingMessageCallback(this,m,s))->post();}
  juce::AudioDeviceManager deviceManager;           // [1]
  int lastInputIndex = 0;                           // [3]
  bool isAddingFromMidiInput = false;               // [4]
  juce::MidiKeyboardState keyboardState;            // [5]
  juce::MidiKeyboardComponent keyboardComponent;    // [6]
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
