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
#include "AudioPlayerPlugin.h"
#include "AudioPlayerEditor.h"

AudioPlayerPlugin::AudioPlayerPlugin() : readAheadThread("AUDIO_READER"), transportState(NoFile), AudioPluginInstance(getDefaultBusesProperties())
{
    readAheadThread.startThread(0);
    formatManager.registerBasicFormats();
    setNumFiles(1);
    addBus(false);
    startTimer(STATE_TIMER_MS);
}

AudioPlayerPlugin::~AudioPlayerPlugin()
{
    readAheadThread.stopThread(500);
    stopTimer();
    while (isTimerRunning());

    // unregister
    if (sharedData) 
        sharedData->playerKilled();
}

/**
    Set shared data class for exchange with recorder
*/
void AudioPlayerPlugin::setSharedData(AudioPlayerPluginSharedData* shared)
{
    sharedData = shared;
    // register player to get notified about recorder creation/deletion
    sharedData->setPlayer(this);
}

/**
    Change the number of files this player plays.
    This does not include the recorder file!
    It is handled automatically, if a recorder is present.
    @param num number of playback files w/o recorder
*/
bool AudioPlayerPlugin::setNumFiles(int num) {

    if (num > MAX_NUM_FILES || num < 1)
        return false;

    // stop player
    if (getTransportState() > Stopped) {
        changeState(Stopping);
        while (hasRecorder() && getRecordState() > Stopped)
            ;
    }

    // delete unused entries from file list
    {
        const ScopedLock afl(audioFileLock);
        while (audioFiles.size() > num + hasRecorderFile()) {
            audioFiles.removeLast();
            removeBus(false);
        }

        // add empty/default values for new files if list enlarged
        while (audioFiles.size() < num + hasRecorderFile()) {
            // if recorder file loaded for playback -> always insert new file second last
            audioFiles.insert(audioFiles.size() - hasRecorderFile(), new AudioFileBundle(nullptr, new String(String::empty)));
        }
    }

    numFiles = num;

    // if recorder is there, reload for playback as last file
    reloadRecorderFile();

	return true;
}

bool AudioPlayerPlugin::canAddBus(bool inputBus) const {
    // add only output buses, up to max. plus optional recorder
    return !inputBus && getBusCount(false) < MAX_NUM_FILES + hasRecorder();
}

bool AudioPlayerPlugin::canRemoveBus(bool inputBus) const {
    // remove only output buses, down to 1 plus optional recorder
    return !inputBus && getBusCount(false) > 1 + hasRecorder();
}

const AudioPlayerPlugin::BusesProperties AudioPlayerPlugin::getDefaultBusesProperties() {
    // default setup is 1 file playback = 1 stereo bus
    BusesProperties prop;
    prop.withOutput("File 1", AudioChannelSet::stereo());
    return prop;
}

bool AudioPlayerPlugin::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // only accept no input
    if (layouts.getMainInputChannels() != 0)
        return false;

    // only accept stereo output buses
    for (auto& bus : layouts.outputBuses) {
        if (bus.size() != 2)
            return false;
    }
    return true;
}

/**
    Used to change the number of files in the player and the bus names when host changes the layout.
    @extends AudioProcessor::canApplyBusCountChange
*/
bool AudioPlayerPlugin::canApplyBusCountChange(bool isInput, bool isAddingBuses, BusProperties& outNewBusProperties) {
    // no inputs allowed
    if (isInput)
        return false;

    // get current buses w/o recorder
    int requiredBuses = getBusCount(false) - hasRecorder();

    if (isAddingBuses) {
        if (!canAddBus(false))
            return false;
        
        requiredBuses++;

        // set bus name and stereo layout
        outNewBusProperties.busName = "File " + String(requiredBuses + hasRecorder());
        outNewBusProperties.defaultLayout = AudioChannelSet::stereo();
        outNewBusProperties.isActivatedByDefault = true;
    }
    else {
        // removing bus
        requiredBuses--;
    }

    // set new number of files
    setNumFiles(requiredBuses);

    return true;
}

/**
   Open one of the files of this player
   @param file the file object to open
   @param fileNo the index of the file to open
   @returns false, if opening the file fails (see debug messages for more information)
*/
bool AudioPlayerPlugin::openFile(File file, int fileNo) {

    const ScopedLock afl(audioFileLock);
    
    // invalid index (including recorder)
    if (fileNo >= getNumFiles())
        return false;

    // first reset the file bundle for defined state in case opening fails
    audioFiles.set(fileNo, new AudioFileBundle(nullptr, new String(String::empty)));

    String *filename = new String(file.getFullPathName());
      
    // also set filename in recorder, if this is the recorder file
    if (hasRecorder() && fileNo == getNumFiles(true)) {
        String *recFilename = new String(*filename);
        recorder->setFilename(recFilename);
	}

	if (file.exists()) {
		Logger::outputDebugString(String("opening file for reading: ") += *filename);
		AudioFormatReader* bufferingReader;
		try {
            // create a reader and from it, a buffering reader
			AudioFormatReader* reader = formatManager.createReaderFor(file);
			if (reader != nullptr)
				bufferingReader = new BufferingAudioReader(reader, readAheadThread, READ_AHEAD_SAMPLES);
            else {
                Logger::outputDebugString("error creating reader!");
                return false;
            }
		}
		catch (...) {
            Logger::outputDebugString("exception creating buffering reader!");
            return false;
		}

		if (bufferingReader != nullptr)
		{
            // wrap reader into a reader source
			Logger::outputDebugString("creating reader source");
			ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource(bufferingReader, true);
            if (newSource != nullptr)
            {
                newSource->prepareToPlay(getBlockSize(), getSampleRate());
                // store reader and filename in a new audio file bundle
                AudioFileBundle *fileBundle = new AudioFileBundle(newSource.release(), filename);
                // replace or add new entry to file data list
                audioFiles.set(fileNo, fileBundle);
            }
            else {
                Logger::outputDebugString("error creating reader source!");
                return false;
            }
        }
        else {
            Logger::outputDebugString("error creating buffering reader!");
            return false;
        }
        // update length to longest file
		updateTotalLength();
	}
    // file does not exist
    else {
        if (hasRecorder() && fileNo == getNumFiles(true))
            // recorder file does not yet exist --> OK
            return true;
        else {
            // playback file has to exist!
            Logger::outputDebugString("file does not exist!");
            return false;
        }
    }

	return true;
}

/**
    Reloads the recoder file, restoring its mute state.
    Will return false if no recorder is there.
    @param mute true to mute the file
    @returns false, if loading fails
    @see openFile
    */
bool AudioPlayerPlugin::reloadRecorderFile(bool mute) {
    if (!hasRecorder())
        return false;

    bool ret = reloadFile(getNumFiles(true));
    if (ret)
        setMute(mute, getNumFiles(true));
    return ret;
}

/**
    Reloads a file with certain mute state.
    @param fileNo the index of the file to load
    @returns false, if loading fails
    @see openFile
*/
bool AudioPlayerPlugin::reloadFile(int fileNo) {
    const String* name = getFullPath(fileNo);
    // no filename
    if (*name == String::empty)
        return false;
    // try loading and return result
    else {
        bool ret = openFile(File(*name), fileNo);
        // any file loaded allows playing again
        if (isReadyToPlay() && transportState < Stopped)
            changeState(Stopped);
        return ret;
    }
}

/**
    Unloads the recorder file, usually to overwrite it.
*/
void AudioPlayerPlugin::unloadRecorderFile() {
    const ScopedLock afl(audioFileLock);

    // if recorder file was loaded for playback
    if (hasRecorderFile()) {
        if (audioFiles.getUnchecked(audioFiles.size() - 1)->source)
            audioFiles.getUnchecked(audioFiles.size() - 1)->source->releaseResources();
        // delete last entry = unload recorder file
        audioFiles.removeLast();
    }
}

/**
    @returns true if a recorder is present and the recorder file was loaded (for playback).
*/
bool AudioPlayerPlugin::hasRecorderFile() {
    const ScopedLock afl(audioFileLock);

    if (hasRecorder() && audioFiles.size() == getNumFiles())
        return true;

    return false;
}

void AudioPlayerPlugin::prepareToPlay(double samples, int estimatedSamplesPerBlock)
{
    const ScopedLock afl(audioFileLock);

    // just call it for all files
	for (const auto& file : audioFiles)
		if (file->source)
            file->source->prepareToPlay(estimatedSamplesPerBlock, samples);
}

void AudioPlayerPlugin::releaseResources()
{
    const ScopedLock afl(audioFileLock);

    // just call it for all files
    for (const auto& file : audioFiles)
        if (file->source)
            file->source->releaseResources();
}

void AudioPlayerPlugin::fillInPluginDescription(PluginDescription & d) const
{
	d.name = getName();
	d.uid = d.name.hashCode();
	d.category = "Input devices";
	d.pluginFormatName = "Internal";
	d.manufacturerName = "mycompany.com";
	d.version = "1.0";
	d.isInstrument = false;
	d.numInputChannels = getTotalNumInputChannels();
	d.numOutputChannels = getTotalNumOutputChannels();
}

/**
    Mute a file playback. 
    No effect, if file with this index does not exist/is not loaded.
    @param mute true to mute
    @param fileNo index of the file starting from 0
*/
void AudioPlayerPlugin::setMute(bool mute, int fileNo) {
    const ScopedLock afl(audioFileLock);

    if (audioFiles[fileNo])
        audioFiles[fileNo]->mute = mute;
}

/**
    Set this player to have a recorder connected to it.
    Adds a bus and loads the recorder file for playing, if possible.
    @param rec pointer to the recorder object (giving nullptr is same as calling recorderKilled)
*/
void AudioPlayerPlugin::setRecorder(AudioSlaveRecorderPlugin* rec)
{
    if (rec != nullptr) {
        recorder = rec;
        addBus(false);
        reloadRecorderFile();
        changeState(Stopping);
    }
    else
        recorderKilled();
}

/**
    Disconnects the recorder from this player when deleted.
    Also unloads the recorder file from playback files.
    Recorder destructor must call this (indirectly via AudioPlayerPluginSharedData)!
*/
void AudioPlayerPlugin::recorderKilled() {
    unloadRecorderFile();
    recorder = nullptr;
    changeState(Stopping);
    removeBus(false);
}

/**
    Get current position in seconds
    @returns position in seconds
*/
double AudioPlayerPlugin::getCurrentPosition() const
{
	if (getSampleRate() > 0.0)
		return getNextReadPosition() / getSampleRate();

	return 0.0;
}

/**
    Get length of longest file in seconds
    @returns length in seconds
*/
double AudioPlayerPlugin::getLengthInSeconds() const
{
	if (getSampleRate()> 0.0)
		return getTotalLength() / getSampleRate();

	return 0.0;
}

/**
    Set position in seconds
    @param pos position in seconds
*/
void AudioPlayerPlugin::setPosition(double pos)
{
	if (getSampleRate() > 0.0)
		setReadPosition((int64)(pos * getSampleRate()));
}

/**
    Gets the next read position in samples for the longest file.
    @returns read position in samples, 0 if no file or error
*/
int64 AudioPlayerPlugin::getNextReadPosition() const {
    const ScopedLock afl(audioFileLock);

	if (audioFiles.size() > longestFile)
        if (audioFiles.getUnchecked(longestFile)->source)
            return audioFiles.getUnchecked(longestFile)->source->getNextReadPosition();
	
    return 0;
}

/**
    Get number of files, optionally including a possible recorder.
    Same time, getNumFiles(true) is also equal to the index of the recorder file (if a recorder is there).
    @param      playbackOnly    if true, get number of playback files only;
    default is false, recorder file will be counted
    - this makes sense as you can also play it
    @returns    the number of files for playback, by default including the recorder file
*/
int AudioPlayerPlugin::getNumFiles(bool playbackOnly) {
    const ScopedLock afl(audioFileLock);

    return numFiles + (!playbackOnly && hasRecorder());
}

/**
    Set position for all files in samples, at max. the end of the file
    @param pos position in samples
*/
void AudioPlayerPlugin::setAllReadPositions(int64 pos) {
    const ScopedLock afl(audioFileLock);

	for (const auto& file : audioFiles)
        if (file->source)
            file->source->setNextReadPosition(jmin(file->source->getTotalLength() -1, pos));
}

/**
    Sets all files to the selected looping mode value.
    @param shouldLoop true to turn on loop mode
*/
void AudioPlayerPlugin::setAllLooping(bool shouldLoop) {
    const ScopedLock afl(audioFileLock);

	for (const auto& file : audioFiles)
        if (file->source)
            file->source->setLooping(shouldLoop);
}

/**
    @returns true if at least one file is in the looping mode
*/
bool AudioPlayerPlugin::isLooping() {
    const ScopedLock afl(audioFileLock);

    for (const auto& file : audioFiles)
        if (file->source)
            if (file->source->isLooping())
                return true;

    return false;
}

/**
    @returns true if at least one file is ready to play.
    I.e. if it has an audio source with length > 0.
*/
bool AudioPlayerPlugin::isReadyToPlay() {
    const ScopedLock afl(audioFileLock);

    for (const auto& file : audioFiles)
        if (file->source)
            if (file->source->getTotalLength())
                return true;

    return false;
}

/**
    Update the total length to the longest file.
    Precondition for getLengthInSeconds and stopping at the end to work.
    So it must be called after each file load.
    @see getLengthInSeconds, processBlock
*/
void AudioPlayerPlugin::updateTotalLength() {
    const ScopedLock afl(audioFileLock);

	int fileNo = 0;
    totalLength = 0;
	for (const auto& file : audioFiles) {
        if (file && file->source) {
            int64 len = file->source->getTotalLength();
            if (len > totalLength) {
                totalLength = len;
                longestFile = fileNo;
            }
        }
        fileNo++;
    }
}

/**
    Get the full path of a file; also works for the recorder file.
    @param fileNo index of the file
    @returns filename including path; empty String if no file loaded
    @see getFilename
*/
const String* AudioPlayerPlugin::getFullPath(int fileNo)
{
    const ScopedLock afl(audioFileLock);

	const String* path;

	if (hasRecorder() && fileNo == getNumFiles(true) && recorder->getFilename() != nullptr)
		path = recorder->getFilename();
    else if (audioFiles.size() > fileNo)
		path = audioFiles.getUnchecked(fileNo)->filename;
	else
		return &String::empty;

    if (path == nullptr)
        return &String::empty;

	return path;
}

/**
    Get the filename (w/o path) of a file; also works for the recorder file.
    @param fileNo index of the file
    @returns filename; empty String if no file loaded
    @see getFullPath
*/
const String* AudioPlayerPlugin::getFilename(int fileNo)
{ 
	String* path = new String(*getFullPath(fileNo));

	// find filename after last slash/backslash
	int index = path->lastIndexOfChar('\\');
	if (index == -1)
		index = path->lastIndexOfChar('/');
	index++; // do not include slash; includes not found -1 -> 0

	String* filename = new String(path->substring(index));
	return filename;
}

/**
    Set next read position in samples.
    Pauses the player, changes position, then resumes previous state.
    @param pos read position in samples
*/
void AudioPlayerPlugin::setReadPosition(int64 pos) 
{
	TransportState previousState(transportState);

	// if playing, request pausing
	if (transportState > Paused) {
		changeState(Pausing);
	}

	const ScopedLock sl(stateLock);
	setAllReadPositions(pos);

	// resume previous state, if it was playing, start anew
	if (previousState == Playing) {
		changeState(Starting);
	} else if (previousState > Paused) {
		changeState(previousState);
	}
}

/**
    Timer callback to run & update the transport state machine
*/
void AudioPlayerPlugin::timerCallback() {
    // switch state requested from UI
    if (transportState == Starting) {
        const ScopedLock sl(stateLock);
        // also start recorder
        if (hasRecorder() && recorder->isReadyToRecord())
            recorder->changeState(Starting);
        changeState(Playing);
    }
    if (transportState == Stopping) {
        const ScopedLock sl(stateLock);
        // also stop recorder
        if (hasRecorder() && recorder->isReadyToRecord()) {
            recorder->changeState(Unloading);
        }
        setAllReadPositions(0);
        changeState(Stopped);
    }
    if (transportState == Pausing) {
        const ScopedLock sl(stateLock);
        // also pause recorder
        if (hasRecorder() && recorder->isReadyToRecord())
            recorder->changeState(Pausing);
        changeState(Paused);
    }
}

void AudioPlayerPlugin::processBlock (AudioSampleBuffer& buffer, MidiBuffer&)
{
    const ScopedLock afl(audioFileLock);

    // clear buffer to avoid noise when not playing or not all files present
    for (int i = 0; i < getTotalNumOutputChannels() && i < buffer.getNumChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

	// read block, if currently playing
	if (audioFiles.size() && transportState == Playing) {
		const ScopedLock sl(stateLock);

        int64 pos = getNextReadPosition();

        // stop at the end
        if (pos >= getTotalLength()) {
            changeState(Stopping);
        }
        
        int bus= 0;
		for (const auto& file : audioFiles) {
			if (file->source && file->source->getTotalLength() > pos) {
				if (!file->mute) {
                    AudioSampleBuffer busBuffer = getBusBuffer(buffer, false, bus);
                    file->source->getNextAudioBlock(AudioSourceChannelInfo(busBuffer));
                }
                else {
                    AudioSampleBuffer* dummyBuffer = new AudioSampleBuffer(2, buffer.getNumSamples());
                    file->source->getNextAudioBlock(AudioSourceChannelInfo(*dummyBuffer));
                    delete dummyBuffer;
                }
			}
			bus++;
		}
	}
}

void AudioPlayerPlugin::getStateInformation(MemoryBlock& data) {
	if (&data) {
        const ScopedLock afl(audioFileLock);

		MemoryOutputStream stream(data, true);

        // save number of files
		stream.writeInt(getNumFiles(true));

        // save filenames
		for (int i = 0; i < getNumFiles(true); i++) {
			if (audioFiles.size())
				stream.writeString(*(audioFiles.getUnchecked(i)->filename));
		}
	}
}

void AudioPlayerPlugin::setStateInformation(const void* data, int sizeInBytes) {
	if (data) {
		String filename;
		MemoryInputStream stream(data, sizeInBytes, false);

        // get number of files
		setNumFiles(stream.readInt());

        // get file names and load
		for (int i = 0; i < getNumFiles(true); i++) {
			filename = stream.readString();
			if (filename.isNotEmpty()) {
				const ScopedLock sl(stateLock);
                // cannot check for success here
				openFile(filename, i);
                // any file loaded allows playing again
                if (isReadyToPlay() && transportState < Stopped)
                    changeState(Stopped);
            }
		}
    }
}

AudioProcessorEditor* AudioPlayerPlugin::createEditor()
{
    return new AudioPlayerEditor (*this);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPlayerPlugin();
}
