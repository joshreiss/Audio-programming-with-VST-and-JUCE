#include "CircularBuffer.h"

CircularBuffer::CircularBuffer()
{
  buffer = juce::AudioSampleBuffer();
  writeIndex = readIndex = delayLength = 0;

}

CircularBuffer::CircularBuffer(int bufferSize, int delayLength)
{
  buffer = juce::AudioSampleBuffer(1, bufferSize); //1
  buffer.clear(); //2
  writeIndex = delayLength; //3
  readIndex = 0; //4
  this->delayLength = delayLength; //5

}

float CircularBuffer::getData()
{
  return buffer.getSample(0, readIndex); 

}

void CircularBuffer::setData(float data)
{
  buffer.setSample(0, writeIndex, data); //2
}

void CircularBuffer::nextSample()
{
  int bufferLength = buffer.getNumSamples();
  readIndex = ((bufferLength + writeIndex) - delayLength) % bufferLength;
  writeIndex = (writeIndex + 1) % bufferLength;
}
