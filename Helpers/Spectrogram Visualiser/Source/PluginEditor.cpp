#include "PluginProcessor.h"
#include "PluginEditor.h"

STFTAudioProcessorEditor::STFTAudioProcessorEditor (STFTAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), forwardFFT(audioProcessor.fftOrder), spectrogramImage(juce::Image::RGB, 512, 512, true)
{
    setOpaque(true);
    setSize(700,500);
    startTimerHz(60);
}

STFTAudioProcessorEditor::~STFTAudioProcessorEditor()
{
}

void STFTAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setOpacity(1.0f);
    g.drawImage(spectrogramImage, getLocalBounds().toFloat());
}

void STFTAudioProcessorEditor::resized()
{
    // This is where you lay out positions of any subcomponents in your editor..
}

void STFTAudioProcessorEditor::drawNextLineOfSpectrogram()
{
    auto rightHandEdge = spectrogramImage.getWidth() - 1;
    auto imageHeight = spectrogramImage.getHeight();
    // first, shuffle image leftwards by 1 pixel..
    spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);              // [1]
    // now render FFT data..
    forwardFFT.performFrequencyOnlyForwardTransform(audioProcessor.fftData.data());                        // [2]
    // find the range of values produced, so we can scale our rendering to show up the detail clearly
    auto maxLevel = juce::FloatVectorOperations::findMinAndMax(audioProcessor.fftData.data(), audioProcessor.fftSize / 2);// [3]
    for (auto y = 1; y < imageHeight; ++y)                                                  // [4]
    {
        auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
        auto fftDataIndex = (size_t)juce::jlimit(0, audioProcessor.fftSize / 2, (int)(skewedProportionY * audioProcessor.fftSize / 2));
        auto level = juce::jmap(audioProcessor.fftData[fftDataIndex], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
        spectrogramImage.setPixelAt(rightHandEdge, y, juce::Colour::fromHSV(level, 1.0f, level, 1.0f)); // [5]
    }
}
void STFTAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.nextFFTBlockReady)
    {
        drawNextLineOfSpectrogram();
        audioProcessor.nextFFTBlockReady = false;
        repaint();
    }
}
