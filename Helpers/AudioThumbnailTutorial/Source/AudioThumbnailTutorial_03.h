/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AudioThumbnailTutorial
 version:          3.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays audio files and displays a thumbnail.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

// example implementation of the Exercise 2
//==============================================================================
class SimpleThumbnailComponent : public juce::Component,
                                 private juce::ChangeListener
{
public:
    SimpleThumbnailComponent (int sourceSamplesPerThumbnailSample,
                              juce::AudioFormatManager& formatManager,
                              juce::AudioThumbnailCache& cache)
       : thumbnail (sourceSamplesPerThumbnailSample, formatManager, cache)
    {
        thumbnail.addChangeListener (this);
    }

    void setFile (const juce::File& file)
    {
        thumbnail.setSource (new juce::FileInputSource (file));
    }

    void paint (juce::Graphics& g) override
    {
        if (thumbnail.getNumChannels() == 0)
            paintIfNoFileLoaded (g);
        else
            paintIfFileLoaded (g);
    }

    void paintIfNoFileLoaded (juce::Graphics& g)
    {
        g.fillAll (juce::Colours::white);
        g.setColour (juce::Colours::darkgrey);
        g.drawFittedText ("No File Loaded", getLocalBounds(), juce::Justification::centred, 1);
    }

    void paintIfFileLoaded (juce::Graphics& g)
    {
        g.fillAll (juce::Colours::white);

        g.setColour (juce::Colours::red);
        thumbnail.drawChannels (g, getLocalBounds(), 0.0, thumbnail.getTotalLength(), 1.0f);
    }

    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)
            thumbnailChanged();
    }

private:
    void thumbnailChanged()
    {
        repaint();
    }

    juce::AudioThumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleThumbnailComponent)
};

//------------------------------------------------------------------------------

class SimplePositionOverlay : public juce::Component,
                              private juce::Timer
{
public:
    SimplePositionOverlay (const juce::AudioTransportSource& transportSourceToUse)
       : transportSource (transportSourceToUse)
    {
        startTimer (40);
    }

    void paint (juce::Graphics& g) override
    {
        auto duration = (float) transportSource.getLengthInSeconds();

        if (duration > 0.0)
        {
            auto audioPosition = (float) transportSource.getCurrentPosition();
            auto drawPosition = (audioPosition / duration) * (float) getWidth();

            g.setColour (juce::Colours::green);
            g.drawLine (drawPosition, 0.0f, drawPosition, (float) getHeight(), 2.0f);
        }
    }

private:
    void timerCallback() override
    {
        repaint();
    }

    const juce::AudioTransportSource& transportSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimplePositionOverlay)
};

//------------------------------------------------------------------------------

class MainContentComponent   : public juce::AudioAppComponent,
                               public juce::ChangeListener
{
public:
    MainContentComponent()
      : state (Stopped),
        thumbnailCache (5),
        thumbnailComp (512, formatManager, thumbnailCache),
        positionOverlay (transportSource)
    {
        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (&playButton);
        playButton.setButtonText ("Play");
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setColour (juce::TextButton::buttonColourId, juce::Colours::green);
        playButton.setEnabled (false);

        addAndMakeVisible (&stopButton);
        stopButton.setButtonText ("Stop");
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setColour (juce::TextButton::buttonColourId, juce::Colours::red);
        stopButton.setEnabled (false);

        addAndMakeVisible (&thumbnailComp);
        addAndMakeVisible (&positionOverlay);

        setSize (600, 400);

        formatManager.registerBasicFormats();
        transportSource.addChangeListener (this);

        setAudioChannels (2, 2);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        transportSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        transportSource.releaseResources();
    }

    void resized() override
    {
        openButton.setBounds (10, 10, getWidth() - 20, 20);
        playButton.setBounds (10, 40, getWidth() - 20, 20);
        stopButton.setBounds (10, 70, getWidth() - 20, 20);

        juce::Rectangle<int> thumbnailBounds (10, 100, getWidth() - 20, getHeight() - 120);
        thumbnailComp.setBounds (thumbnailBounds);
        positionOverlay.setBounds (thumbnailBounds);
    }

    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
            transportSourceChanged();
    }

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
                case Stopped:
                    stopButton.setEnabled (false);
                    playButton.setEnabled (true);
                    transportSource.setPosition (0.0);
                    break;

                case Starting:
                    playButton.setEnabled (false);
                    transportSource.start();
                    break;

                case Playing:
                    stopButton.setEnabled (true);
                    break;

                case Stopping:
                    transportSource.stop();
                    break;

                default:
                    jassertfalse;
                    break;
            }
        }
    }

    void transportSourceChanged()
    {
        if (transportSource.isPlaying())
            changeState (Playing);
        else
            changeState (Stopped);
    }

    void openButtonClicked()
    {
        chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                       juce::File{},
                                                       "*.wav");
        auto chooserFlags = juce::FileBrowserComponent::openMode
                          | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            if (file != juce::File{})
            {
                auto* reader = formatManager.createReaderFor (file);

                if (reader != nullptr)
                {
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
                    transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                    playButton.setEnabled (true);
                    thumbnailComp.setFile (file);
                    readerSource.reset (newSource.release());
                }
            }
        });
    }

    void playButtonClicked()
    {
        changeState (Starting);
    }

    void stopButtonClicked()
    {
        changeState (Stopping);
    }

    //==========================================================================
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;
    juce::AudioThumbnailCache thumbnailCache;
    SimpleThumbnailComponent thumbnailComp;
    SimplePositionOverlay positionOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
