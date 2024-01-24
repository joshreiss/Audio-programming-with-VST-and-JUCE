#include "PluginProcessor.h"
#include "PluginEditor.h"

RecorderAudioProcessor::RecorderAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
  : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
    .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
    .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
  )
#endif
{
    backgroundThread.startThread();
}

RecorderAudioProcessor::~RecorderAudioProcessor()
{
    stop();
}

const String RecorderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}
bool RecorderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}
bool RecorderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}
bool RecorderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
double RecorderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}
int RecorderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}
int RecorderAudioProcessor::getCurrentProgram()
{
    return 0;
}
void RecorderAudioProcessor::setCurrentProgram (int index)
{
}
const String RecorderAudioProcessor::getProgramName (int index)
{
    return {};
}
void RecorderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

void RecorderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    this->sampleRate = sampleRate;
}

void RecorderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    this->sampleRate = 0;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RecorderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void RecorderAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    const ScopedLock sl (writerLock);

    if (activeWriter.load() != nullptr)
    {
        activeWriter.load()->write (buffer.getArrayOfReadPointers(), buffer.getNumSamples());
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        const float* inputData = buffer.getReadPointer(channel);
        auto* outputData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = inputData[sample];
            outputData[sample] = inputSample;
        }
    }
}
bool RecorderAudioProcessor::hasEditor() const
{
    return true;
}
AudioProcessorEditor* RecorderAudioProcessor::createEditor()
{
    return new RecorderAudioProcessorEditor (*this);
}
void RecorderAudioProcessor::getStateInformation(juce::MemoryBlock& destData) { }
void RecorderAudioProcessor::setStateInformation(const void* data, int sizeInBytes) { }

void RecorderAudioProcessor::startRecording(const File& file)
{
  stop();
  if (sampleRate > 0)
  {
    // Create an OutputStream to write to our destination file...
    file.deleteFile();
    if (auto fileStream = std::unique_ptr<FileOutputStream>(file.createOutputStream()))
    {
      // Now create a WAV writer object that writes to our output stream...
      WavAudioFormat wavFormat;
      if (auto writer = wavFormat.createWriterFor(fileStream.get(), sampleRate, getTotalNumInputChannels(), 24, {}, 0))
      {
        fileStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)

        // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
        // write the data to disk on our background thread.
        threadedWriter.reset(new AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32768));

        // And now, swap over our active writer pointer so that the audio callback will start using it..
        const ScopedLock sl(writerLock);
        activeWriter = threadedWriter.get();
      }
    }
  }
}
void RecorderAudioProcessor::stop()
{
  // First, clear this pointer to stop the audio callback from using our writer object..
  {
    const ScopedLock sl(writerLock);
    activeWriter = nullptr;
  }
  // Now delete the writer object. It's done in this order because deletion could take a little time while 
  // remaining data gets flushed to disk, so it's best to avoid blocking the audio callback while this happens.
  threadedWriter.reset();
}
bool RecorderAudioProcessor::isRecording()
{
  return activeWriter.load() != nullptr;
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RecorderAudioProcessor();
}
