void CompressorExpanderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    const double smoothTime = 1e-3;
    paramThreshold.reset (sampleRate, smoothTime);
    paramRatio.reset (sampleRate, smoothTime);
    paramAttack.reset (sampleRate, smoothTime);
    paramRelease.reset (sampleRate, smoothTime);
    paramMakeupGain.reset (sampleRate, smoothTime);
    paramBypass.reset (sampleRate, smoothTime);
    mixedDownInput.setSize (1, samplesPerBlock);
    inputLevel = 0.0f;
    ylPrev = 0.0f;
    inverseSampleRate = 1.0f / (float)getSampleRate();
    inverseE = 1.0f / M_E;
}
void CompressorExpanderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) {
    ScopedNoDenormals noDenormals;
    const int numInputChannels = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    if ((bool)paramBypass.getTargetValue()) return;
    mixedDownInput.clear();
    for (int channel = 0; channel < numInputChannels; ++channel)
        mixedDownInput.addFrom (0, 0, buffer, channel, 0, numSamples, 1.0f / numInputChannels);
    for (int sample = 0; sample < numSamples; ++sample) {
        bool expander = (bool)paramMode.getTargetValue();
        float T = paramThreshold.getNextValue();
        float R = paramRatio.getNextValue();
        float alphaA = calculateAttackOrRelease (paramAttack.getNextValue());
        float alphaR = calculateAttackOrRelease (paramRelease.getNextValue());
        float makeupGain = paramMakeupGain.getNextValue();
        float inputSquared = powf (mixedDownInput.getSample (0, sample), 2.0f);
        if (expander) {
            const float averageFactor = 0.9999f;
            inputLevel = averageFactor * inputLevel + (1.0f - averageFactor) * inputSquared;
        } else inputLevel = inputSquared;
        xg = (inputLevel <= 1e-6f) ? -60.0f : 10.0f * log10f (inputLevel);
        // Expander
        if (expander) {
            if (xg > T) yg = xg;
            else yg = T + (xg - T) * R;
            xl = xg - yg;
            if (xl < ylPrev) yl = alphaA * ylPrev + (1.0f - alphaA) * xl;
            else yl = alphaR * ylPrev + (1.0f - alphaR) * xl;
        // Compressor
        } else {
            if (xg < T) yg = xg;
            else yg = T + (xg - T) / R;
            xl = xg - yg;
            if (xl > ylPrev) yl = alphaA * ylPrev + (1.0f - alphaA) * xl;
            else yl = alphaR * ylPrev + (1.0f - alphaR) * xl;
        }
        control = powf (10.0f, (makeupGain - yl) * 0.05f);
        ylPrev = yl;
        for (int channel = 0; channel < numInputChannels; ++channel) {
            float newValue = buffer.getSample (channel, sample) * control;
            buffer.setSample (channel, sample, newValue);
        }
    }
    for (int channel = numInputChannels; channel < numOutputChannels; ++channel) buffer.clear (channel, 0, numSamples);
}
float CompressorExpanderAudioProcessor::calculateAttackOrRelease (float value)
{
    if (value == 0.0f) return 0.0f;
    else return pow (inverseE, inverseSampleRate / value);
}
