#include "PluginProcessor.h"
#include "PluginEditor.h"

ArpeggiatorAudioProcessor::ArpeggiatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
  addParameter(speed = new juce::AudioParameterFloat({ "speed", 1 }, "Arpeggiator Speed", 0.0, 1.0, 0.5));
}

ArpeggiatorAudioProcessor::~ArpeggiatorAudioProcessor()
{
}

const juce::String ArpeggiatorAudioProcessor::getName() const
{
  return "Arpeggiator";
}

bool ArpeggiatorAudioProcessor::acceptsMidi() const
{
   return true;
}

bool ArpeggiatorAudioProcessor::producesMidi() const
{
  return true;
}

bool ArpeggiatorAudioProcessor::isMidiEffect() const
{
  return true;
}

double ArpeggiatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ArpeggiatorAudioProcessor::getNumPrograms()
{
    return 1;
}

int ArpeggiatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ArpeggiatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ArpeggiatorAudioProcessor::getProgramName (int index)
{
    return "None";
}

void ArpeggiatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void ArpeggiatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  //ignoreUnused(samplesPerBlock);

  notes.clear();
  currentNote = 0;
  lastNoteValue = -1;
  time = 0;
  rate = static_cast<float> (sampleRate);
}

void ArpeggiatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ArpeggiatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ArpeggiatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
  // A pure MIDI plugin shouldn't be provided any audio data
  jassert(buffer.getNumChannels() == 0);

  // however we use the buffer to get timing information
  auto numSamples = buffer.getNumSamples();

  // get note duration, from 0.025 to 0.275
  auto noteDuration = static_cast<int> (std::ceil(rate * 0.25f * (0.1f + (1.0f - (*speed)))));

  for (const auto metadata : midi)
  {
    const auto msg = metadata.getMessage();
    if (msg.isNoteOn())  notes.add(msg.getNoteNumber());
    else if (msg.isNoteOff()) notes.removeValue(msg.getNoteNumber());
  }

  midi.clear();

  if ((time + numSamples) >= noteDuration)
  {
    auto offset = juce::jmax(0, juce::jmin((int)(noteDuration - time), numSamples - 1));

    if (lastNoteValue > 0)
    {
      midi.addEvent(juce::MidiMessage::noteOff(1, lastNoteValue), offset);
      lastNoteValue = -1;
    }

    if (notes.size() > 0)
    {
      currentNote = (currentNote + 1) % notes.size();
      lastNoteValue = notes[currentNote];
      midi.addEvent(juce::MidiMessage::noteOn(1, lastNoteValue, (juce::uint8)127), offset);
    }

  }

  time = (time + numSamples) % noteDuration;
}

bool ArpeggiatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ArpeggiatorAudioProcessor::createEditor()
{
  return new juce::GenericAudioProcessorEditor(*this);
}

void ArpeggiatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
  juce::MemoryOutputStream(destData, true).writeFloat(*speed);
}

void ArpeggiatorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  speed->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readFloat());
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ArpeggiatorAudioProcessor();
}
