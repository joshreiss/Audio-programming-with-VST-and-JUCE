#pragma once
#include <JuceHeader.h>
class SineWaveSound : public juce::SynthesiserSound // A demo synth sound that's just a basic sine wave
{
public:
    SineWaveSound() {}
    bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel(int /*midiChannel*/) override { return true; }
};
class SineWaveVoice : public juce::SynthesiserVoice { // Simple demo synth voice that plays sine wave
public:
    SineWaveVoice() {}
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    bool canPlaySound(juce::SynthesiserSound* sound) override { return dynamic_cast<SineWaveSound*> (sound) != nullptr; }
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* /*sound*/, int) override {
        currentAngle = 0.0;
        level = velocity * 0.15;
        angleDelta = juce::MathConstants<double>::twoPi * juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber) / getSampleRate();
    }
    void stopNote(float /*velocity*/, bool allowTailOff) override {
        clearCurrentNote();
        angleDelta = 0.0;
    }
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        if (angleDelta != 0.0) {
            while (--numSamples >= 0) {
                auto currentSample = (float)(sin(currentAngle) * level);
                for (auto i = outputBuffer.getNumChannels(); --i >= 0;) outputBuffer.addSample(i, startSample, currentSample);
                currentAngle += angleDelta;
                ++startSample;
            }
        }
    }
private:
    double currentAngle = 0.0, angleDelta = 0.0, level = 0.0;
};
