/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 5.0.1

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
/*
==============================================================================

This file is part of JUCE examples from https://github.com/harry-g.
Copyright (c) 2017 Harry G.

Permission is granted to use this software under the terms of the GPL v2 (or any later version)

Details of these licenses can be found at: www.gnu.org/licenses

JUCE examples are distributed in the hope that they will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

==============================================================================
*/
#include "JuceHeader.h"
#include "AudioPlayerPlugin.h"

//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    The Editor for the AudioPlayerPlugin which is used as InternalFilter for the Plugin Host.
    It can play multiple files in parallel.
    It controls an optional AudioSlaveRecorderPlugin via a SharedResourcePointer<AudioPlayerPluginSharedData>.
    File number is configuerd via JUCE multibus configuration.

    !!!UI is created with the Projucer - only change code in user areas!!!

    @see AudioPlayerPlugin, AudioSlaveRecorderPlugin, AudioPlayerPluginSharedData
                                                                    //[/Comments]
*/
class AudioPlayerEditor  : public AudioProcessorEditor,
                           private Timer,
                           public ButtonListener,
                           public SliderListener
{
public:
    //==============================================================================
    AudioPlayerEditor (AudioPlayerPlugin &processor);
    ~AudioPlayerEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
	class AudioFileUiBundle;
	class ThumbnailComp;

	void updateLoopState(bool);
	TransportState getTransportState() { return processor.getTransportState(); }
	bool isPlaying() { return processor.isPlaying(); }
    bool isRecording();
	double getCurrentPosition() { return processor.getCurrentPosition(); }
	double getLengthInSeconds() { return processor.getLengthInSeconds(); }
	int64 getNextReadPosition() const { return processor.getNextReadPosition(); }
	int64 getTotalLength() const { return processor.getTotalLength(); }

    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

    // Binary resources:
    static const char* back_png;
    static const int back_pngSize;
    static const char* pause_png;
    static const int pause_pngSize;
    static const char* play_png;
    static const int play_pngSize;
    static const char* stop_png;
    static const int stop_pngSize;
    static const char* folder_png;
    static const int folder_pngSize;
    static const char* reclightred_png;
    static const int reclightred_pngSize;
    static const char* record_png;
    static const int record_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
protected:
	friend class ThumbnailComp;
	void changeState(TransportState state) { return processor.changeState(state); }
    int getNumFiles(bool = false);
    void setPosition(double pos);
    void setReadPosition(int64 );
private:
	const unsigned UI_TIMER_MS = 100; // timer in milliseconds for UI update
    const String NO_FILE = String("(no file)");
	const unsigned Y_OFFSET = 85; // offset to UI part from Projucer GUI editor
	const unsigned Y_LINE_HEIGHT = 100; // height of UI for one file

    CriticalSection layoutLock;

    // some vars to cache internal state
    double previousPos = -1;
    TransportState oldState = NoFile;
    bool wasRecording = false;
    bool hadRecorder = false;
    int oldNoOfFiles = -1;
    bool wasMuted = false;
    bool fileUiInitialized = false;

    OwnedArray<AudioFileUiBundle> audioFiles;
    AudioFormatManager formatManager;
	AudioPlayerPlugin& processor;

    void hideThumbnail(int);
    void updateThumbnail(int);
    void repaintThumbnails();
    void updateFilename(int);

	Rectangle<int> getFileArea();
	unsigned getFileLineHeight();
	void updateUiLayout();
	void updatePlayPauseButton(bool);

	void updateRecordState();
    bool toggleRecordState();
    bool prepareRecording();
    void recordingStopped();

    void updateTime();
	void timerCallback() override;

    ScopedPointer<LiveScrollingAudioDisplay> audioInputThumb;

    //[/UserVariables]

    //==============================================================================
    ScopedPointer<ImageButton> back;
    ScopedPointer<ImageButton> playPause;
    ScopedPointer<ImageButton> stop;
    ScopedPointer<Slider> timeSlider;
    ScopedPointer<Label> timeLabel;
    ScopedPointer<ImageButton> record;
    ScopedPointer<Slider> zoomSlider;
    ScopedPointer<Label> zoomLabel;
    ScopedPointer<TextButton> repeat;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]
