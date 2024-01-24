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
#ifndef AUDIOPLAYERPLUGIN_H_INCLUDED
#define AUDIOPLAYERPLUGIN_H_INCLUDED

#include "JuceHeader.h"
#include "AudioSlaveRecorderPlugin.h"
#include "TransportState.h"
#include "AudioLiveScrollingDisplay.h"
#include "PluginSharedData.h"

/**
    An audio player plugin which is used as InternalFilter for the Plugin Host.
    It can play multiple audio files in parallel.
    It controls an optional AudioSlaveRecorderPlugin via a AudioPlayerPluginSharedData.

    It is controlled by the UI in the AudioPlayerEditor.
    File number is configuerd via JUCE multibus configuration.

    @see AudioPlayerEditor, AudioSlaveRecorderPlugin, AudioPlayerPluginSharedData
*/
class AudioPlayerPlugin : public AudioPluginInstance, private Timer
{
public:
    AudioPlayerPlugin();
    ~AudioPlayerPlugin();

    /**
        A container class for the data of each audio file including
        - an AudioFormatReaderSource 
        - the filename (including path)
        - the mute state
    */
	class AudioFileBundle {
	public:
		AudioFileBundle(AudioFormatReaderSource* source, String* filename, bool mute = false) : source(source), filename(filename), mute(mute) {};
        ~AudioFileBundle() {};

	protected:
		friend class AudioPlayerPlugin;
		ScopedPointer<AudioFormatReaderSource> source;
		ScopedPointer<String> filename;
		bool mute = false;
	};

    void setSharedData(AudioPlayerPluginSharedData*);

	TransportState getTransportState() { return transportState; }
	bool isPlaying() { return transportState == Playing; }

	AudioFormatReaderSource* getAudioSource(int fileNo) { return ((audioFiles.size()>fileNo) ? audioFiles.getUnchecked(fileNo)->source : nullptr); }
	const String* getFullPath(int);
	const String* getFilename(int);

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const String getName() const override {	return "Audiofile Player"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; } // some hosts don't cope well with 0 programs
    int getCurrentProgram() override { return 0; }
	void setCurrentProgram(int) override {}
    const String getProgramName (int) override { return String(); }
	void changeProgramName(int, const String&) override {}

	void getStateInformation(MemoryBlock&) override;
	void setStateInformation(const void*, int) override;

	double getCurrentPosition() const;
	double getLengthInSeconds() const;
	int64 getNextReadPosition() const;
	int64 getTotalLength() const { return totalLength; }
    int getNumFiles(bool = false);
	int getLongestFile() { return longestFile; }

	virtual void fillInPluginDescription(PluginDescription &) const override;

    void setMute(bool, int);
    bool isMuted(int fileNo) { if (audioFiles.size() > fileNo) return audioFiles.getUnchecked(fileNo)->mute; else return false; }
    void setRecorder(AudioSlaveRecorderPlugin *);
	TransportState getRecordState() { return recorder->getState(); }
	void recorderKilled();

protected:
	friend class AudioPlayerEditor;
	void changeState(TransportState newState) { const ScopedLock sl(stateLock); transportState = newState; }
    bool changeRecordState(TransportState newState) { return recorder->changeState(newState); }

    bool openFile(File, int);
    bool reloadRecorderFile(bool=false);
    bool reloadFile(int);
    void unloadRecorderFile();
    bool hasRecorderFile();

    void setAllLooping(bool);
    bool isLooping();
    bool isReadyToPlay();
	bool hasRecorder() const { return (recorder != nullptr); }
	bool setNumFiles(int);
	void setPosition(double);
	void setReadPosition(int64);
    LiveScrollingAudioDisplay liveAudioScroller;

private:
    const unsigned STATE_TIMER_MS = 250; // milliseconds to update transport state
    const unsigned READ_AHEAD_SAMPLES = 65536; // bytes to read ahead - avoids distortions by HDD load
    const int MAX_NUM_FILES = 10; // max. number of files that makes sense from UI and performance perspective

    TimeSliceThread readAheadThread;
    
	int numFiles = 0;
	int longestFile = 0;
	int64 totalLength;
	TransportState transportState;
	OwnedArray<AudioFileBundle> audioFiles;
	AudioFormatManager formatManager;
	AudioSlaveRecorderPlugin* recorder = nullptr;
	CriticalSection stateLock, audioFileLock;
    AudioPlayerPluginSharedData* sharedData = nullptr;

    const BusesProperties getDefaultBusesProperties();
    bool canAddBus(bool) const override;
    bool canRemoveBus(bool) const override;
    void setAllReadPositions(int64);
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    bool canApplyBusCountChange(bool, bool, BusProperties&) override;
    virtual void prepareToPlay(double, int) override;
	virtual void releaseResources() override;
	void updateTotalLength();
    void timerCallback() override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPlayerPlugin)
};


#endif  // AUDIOPLAYERPLUGIN_H_INCLUDED
