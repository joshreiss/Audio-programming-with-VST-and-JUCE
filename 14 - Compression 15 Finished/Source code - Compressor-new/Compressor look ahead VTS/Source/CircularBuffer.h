/*
  ==============================================================================

    CircularBuffer.h
    Created: 22 May 2023 1:25:03pm
    Author:  justj

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class CircularBuffer {
public:
  CircularBuffer();
  CircularBuffer(int bufferSize, int delayLength);
  float getData();
  void setData(float data);
  void nextSample();

private:
  juce::AudioSampleBuffer buffer;
  int writeIndex;
  int readIndex;
  int delayLength;
};
