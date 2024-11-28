#include "PluginProcessor.h"
#include "PluginEditor.h"

SpectrumAudioProcessorEditor::SpectrumAudioProcessorEditor (SpectrumAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), 
      forwardFFT(audioProcessor.fftOrder), window(audioProcessor.fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    setOpaque(true);
    setSize(700,500);
    startTimerHz(30);
}
SpectrumAudioProcessorEditor::~SpectrumAudioProcessorEditor()
{
}
void SpectrumAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setOpacity(1.0f);
    g.setColour(juce::Colours::white);
    drawFrame(g);
}
void SpectrumAudioProcessorEditor::resized()
{
    // This is where you lay out positions of any subcomponents in your editor..
}
void SpectrumAudioProcessorEditor::drawNextFrameOfSpectrum()
{
    // apply a windowing function to our data
    window.multiplyWithWindowingTable(audioProcessor.fftData, audioProcessor.fftSize);       // [1]
    // then render our FFT data..
    forwardFFT.performFrequencyOnlyForwardTransform(audioProcessor.fftData);  // [2]
    auto mindB = -100.0f;
    auto maxdB = 0.0f;
    for (int i = 0; i < audioProcessor.scopeSize; ++i)                         // [3]
    {
        auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)audioProcessor.scopeSize) * 0.2f);
        auto fftDataIndex = juce::jlimit(0, audioProcessor.fftSize / 2, (int)(skewedProportionX * (float)audioProcessor.fftSize * 0.5f));
        auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(audioProcessor.fftData[fftDataIndex])
            - juce::Decibels::gainToDecibels((float)audioProcessor.fftSize)),
            mindB, maxdB, 0.0f, 1.0f);
        audioProcessor.scopeData[i] = level;                                   // [4]
    }
}
void SpectrumAudioProcessorEditor::drawFrame(juce::Graphics& g)
{
    for (int i = 1; i < audioProcessor.scopeSize; ++i)
    {
        auto width = getLocalBounds().getWidth();
        auto height = getLocalBounds().getHeight();
        g.drawLine({ (float)juce::jmap(i - 1, 0, audioProcessor.scopeSize - 1, 0, width),
                              juce::jmap(audioProcessor.scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                      (float)juce::jmap(i, 0, audioProcessor.scopeSize - 1, 0, width),
                              juce::jmap(audioProcessor.scopeData[i],     0.0f, 1.0f, (float)height, 0.0f) });
    }
}
void SpectrumAudioProcessorEditor::pushNextSampleIntoFifo(float) noexcept
{
    // if the fifo contains enough data, set a flag to say that the next line should now be rendered..
}
void SpectrumAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.nextFFTBlockReady)
    {
        drawNextFrameOfSpectrum();
        audioProcessor.nextFFTBlockReady = false;
        repaint();
    }
}
