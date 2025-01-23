#include "PluginProcessor.h"
#include "PluginEditor.h"
GenericUIProcessor::GenericUIProcessor()
{
  addParameter(floatParam = new juce::AudioParameterFloat("FLOATPARAM", "Float Slider", 0.0f, 1.0f, 0.5f));// parameterID,parameter name, min, max default value
  addParameter(intParam = new juce::AudioParameterInt("INTPARAM", "Int Slider", 20, 40, 30, "Hz"));

  addParameter(choice3Param = new juce::AudioParameterChoice("THREECHOICE", "Three choice", { "A", "B", "C"}, 1));
  addParameter(choice2Param = new juce::AudioParameterChoice("TWOCHOICE", "Two choice", { "X", "Y" }, 0));
  addParameter(boolParam = new juce::AudioParameterBool("BOOL", "Check box", false));

  addParameter(floatNorm1Param = new juce::AudioParameterFloat("FLOATN1", "float norm 1", { 0.0f, 1.0f, 0.01 }, 0.3f));
  addParameter(floatNorm2Param = new juce::AudioParameterFloat("FLOATN2", "float norm 2", { 0.0f, 60.0f, 0.0f, 1.0f }, 0.f, "dB"));
}
GenericUIProcessor::~GenericUIProcessor()
{
}
const juce::String GenericUIProcessor::getName() const
{
    return JucePlugin_Name;
}
bool GenericUIProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}
bool GenericUIProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}
bool GenericUIProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
double GenericUIProcessor::getTailLengthSeconds() const
{
    return 0.0;
}
int GenericUIProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GenericUIProcessor::getCurrentProgram()
{
    return 0;
}

void GenericUIProcessor::setCurrentProgram (int index)
{
}

const juce::String GenericUIProcessor::getProgramName (int index)
{
    return {};
}

void GenericUIProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GenericUIProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void GenericUIProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GenericUIProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void GenericUIProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    float gSlider = floatParam->get();
    auto* channeldata = buffer.getWritePointer(0); //—1
    for (int i = 0; i < buffer.getNumSamples(); i++) //—2
    {
      auto input = channeldata[i]; //—1
      input = input * gSlider; //—2
      channeldata[i] = input; //—3
    }
}

//==============================================================================
bool GenericUIProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GenericUIProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void GenericUIProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  // Store parameters in memory block, for saving & loading as raw data or using XML or ValueTree classes
  std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("ParamTutorial"));
  xml->setAttribute("floatParam", (double)*floatParam);
  xml->setAttribute("intParam", (int)*intParam);
  if (*floatParam < 0.5f) xml->setAttribute("test", "Low Testing");
  else xml->setAttribute("test", "High Testing");
  copyXmlToBinary(*xml, destData);

  DBG("put state of plugin in XML " << stringParam);
}
void GenericUIProcessor::setStateInformation (const void* data, int sizeInBytes)
{
  std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName("ParamTutorial")) {
      *floatParam = (float)xmlState->getDoubleAttribute("floatParam", 1.0);
      *intParam = (int)xmlState->getIntAttribute("intParam", 1.0); 
      juce::String cat = "test";
      stringParam = xmlState->getStringAttribute(cat);
      DBG("set state of plugin from XML " << stringParam);
     // *stringParam = xmlState->getStringAttribute("test", 1.0);
    }
}
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GenericUIProcessor();
}