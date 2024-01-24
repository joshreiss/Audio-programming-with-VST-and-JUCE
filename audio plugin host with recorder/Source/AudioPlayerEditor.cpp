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

//[Headers] You can add your own extra header files here...
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
//[/Headers]

#include "AudioPlayerEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
#include "ThumbnailComp.h"

/**
    A simple container class for the UI of each audio file including
    - a file open button
    - the filename (including path)
    - an audio thumbnail
    - a mute button
*/
class AudioPlayerEditor::AudioFileUiBundle
{
public:
	AudioFileUiBundle(ImageButton *openFileButton, Label *filename, ThumbnailComp *thumbnail, Button *muteButton)
		: openFileButton(openFileButton), filename(filename), thumbnail(thumbnail), muteButton(muteButton) {};

	ScopedPointer<ImageButton> openFileButton;
	ScopedPointer<Label> filename;
	ScopedPointer<ThumbnailComp> thumbnail;
	ScopedPointer<Button> muteButton;
};
//[/MiscUserDefs]

//==============================================================================
AudioPlayerEditor::AudioPlayerEditor (AudioPlayerPlugin &processor)
    : AudioProcessorEditor(processor), processor(processor)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (back = new ImageButton ("back"));
    back->setExplicitFocusOrder (1);
    back->setButtonText (String());
    back->addListener (this);

    back->setImages (false, true, true,
                     ImageCache::getFromMemory (back_png, back_pngSize), 1.000f, Colour (0x00000000),
                     Image(), 1.000f, Colour (0x00000000),
                     Image(), 1.000f, Colour (0x00000000));
    addAndMakeVisible (playPause = new ImageButton ("playPause"));
    playPause->setExplicitFocusOrder (2);
    playPause->setButtonText (String());
    playPause->addListener (this);

    playPause->setImages (false, true, true,
                          ImageCache::getFromMemory (play_png, play_pngSize), 1.000f, Colour (0x00000000),
                          Image(), 1.000f, Colour (0x00000000),
                          Image(), 1.000f, Colour (0x00000000));
    addAndMakeVisible (stop = new ImageButton ("stop"));
    stop->setExplicitFocusOrder (3);
    stop->setButtonText (String());
    stop->addListener (this);

    stop->setImages (false, true, true,
                     ImageCache::getFromMemory (stop_png, stop_pngSize), 1.000f, Colour (0x00000000),
                     Image(), 1.000f, Colour (0x00000000),
                     Image(), 1.000f, Colour (0x00000000));
    addAndMakeVisible (timeSlider = new Slider ("timeSlider"));
    timeSlider->setExplicitFocusOrder (4);
    timeSlider->setRange (0, 100, 0);
    timeSlider->setSliderStyle (Slider::LinearHorizontal);
    timeSlider->setTextBoxStyle (Slider::NoTextBox, true, 55, 20);
    timeSlider->setColour (Slider::thumbColourId, Colour (0xffa6a6b4));
    timeSlider->addListener (this);

    addAndMakeVisible (timeLabel = new Label ("timeLabel",
                                              String()));
    timeLabel->setFont (Font (Font::getDefaultSansSerifFontName(), 18.00f, Font::plain).withTypefaceStyle ("Regular"));
    timeLabel->setJustificationType (Justification::centredLeft);
    timeLabel->setEditable (false, false, false);
    timeLabel->setColour (TextEditor::textColourId, Colours::black);
    timeLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (record = new ImageButton ("record"));
    record->setButtonText (String());
    record->addListener (this);

    record->setImages (false, true, true,
                       ImageCache::getFromMemory (reclightred_png, reclightred_pngSize), 1.000f, Colour (0x00000000),
                       Image(), 1.000f, Colour (0x00000000),
                       Image(), 1.000f, Colour (0x00000000));
    addAndMakeVisible (zoomSlider = new Slider ("zoom"));
    zoomSlider->setTooltip (TRANS("zoom"));
    zoomSlider->setRange (0, 1, 0);
    zoomSlider->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    zoomSlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    zoomSlider->addListener (this);

    addAndMakeVisible (zoomLabel = new Label ("zoomLabel",
                                              TRANS("Zoom")));
    zoomLabel->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    zoomLabel->setJustificationType (Justification::centredLeft);
    zoomLabel->setEditable (false, false, false);
    zoomLabel->setColour (TextEditor::textColourId, Colours::black);
    zoomLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (repeat = new TextButton ("repeat"));
    repeat->addListener (this);
    repeat->setColour (TextButton::textColourOnId, Colours::grey);


    //[UserPreSize]
    addChildComponent(audioInputThumb = new LiveScrollingAudioDisplay());
    audioInputThumb->setVisible(false);
    //[/UserPreSize]

    setSize (750, 185);


    //[Constructor] You can add your own custom stuff here..
    repeat->setClickingTogglesState(true);

    formatManager.registerBasicFormats();
	updateUiLayout();
    updatePlayPauseButton(true);
	startTimer(UI_TIMER_MS);
    //[/Constructor]
}

AudioPlayerEditor::~AudioPlayerEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    stopTimer();
    while(isTimerRunning());

    //[/Destructor_pre]

    back = nullptr;
    playPause = nullptr;
    stop = nullptr;
    timeSlider = nullptr;
    timeLabel = nullptr;
    record = nullptr;
    zoomSlider = nullptr;
    zoomLabel = nullptr;
    repeat = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    audioFiles.clear();
    if (processor.hasRecorder()) {
        processor.recorder->removeAudioCallback(audioInputThumb);
    }
    audioInputThumb = nullptr;

    processor.editorBeingDeleted(this);
    //[/Destructor]
}

//==============================================================================
void AudioPlayerEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff5e5e5e));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AudioPlayerEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    back->setBounds (8, 8, 40, 24);
    playPause->setBounds (108, 8, 40, 24);
    stop->setBounds (58, 8, 40, 24);
    timeSlider->setBounds (10, 40, 622, 24);
    timeLabel->setBounds (648, 40, 72, 24);
    record->setBounds (158, 8, 40, 24);
    zoomSlider->setBounds (694, 7, 33, 26);
    zoomLabel->setBounds (648, 8, 53, 24);
    repeat->setBounds (208, 8, 60, 24);
    //[UserResized] Add your own custom resize handling here..
	Rectangle<int> r = getFileArea();
    int i = 0;
    // go through all file UI and resize them
    for (const auto& file : audioFiles) {
		file->openFileButton->setBounds(r.withTrimmedTop(10).withSize(40, 24));
		file->filename->setBounds(r.withTrimmedTop(10).withTrimmedLeft(50).withSize(200, 24));
		file->thumbnail->setBounds(r.withLeft(260).withHeight(getFileLineHeight() - 10));
        file->muteButton->setBounds(r.withTrimmedTop(40).withTrimmedLeft(50).withSize(60, 24));

        // index == number of playback files -> this must be the recorder, it additionally has a LiveScrollingAudioDisplay
        if (i == getNumFiles(true)) {
            // set input thumb size same as file thumbnail
            audioInputThumb->setBounds(r.withLeft(260).withHeight(getFileLineHeight() - 10));
        }
        i++;

        r.removeFromTop(getFileLineHeight());
	}
    //[/UserResized]
}

void AudioPlayerEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == back)
    {
        //[UserButtonCode_back] -- add your button handler code here..
		processor.setReadPosition(0);
		updateTime(); // to update slider and label
		back->setEnabled(false);
        //[/UserButtonCode_back]
    }
    else if (buttonThatWasClicked == playPause)
    {
        //[UserButtonCode_playPause] -- add your button handler code here..
		updateLoopState(repeat->getToggleState());
		if ((getTransportState() == Stopped) || (getTransportState() == Paused))
			changeState(Starting);
		else if (isPlaying())
			changeState(Pausing);
        //[/UserButtonCode_playPause]
    }
    else if (buttonThatWasClicked == stop)
    {
        //[UserButtonCode_stop] -- add your button handler code here..
		changeState(Stopping);
        //[/UserButtonCode_stop]
    }
    else if (buttonThatWasClicked == record)
    {
        //[UserButtonCode_record] -- add your button handler code here..

        // check if a filename is selected
        if (processor.getFilename(getNumFiles(true))->isEmpty()) {
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                "Cannot Record",
                "Please chose a filename to record first!");
        }
        // handle button press
		else if (!toggleRecordState()) {
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
				"Recording Error",
				"There was an errror trying to Record, e.g. file could not be opened for writing!");
		}
        //[/UserButtonCode_record]
    }
    else if (buttonThatWasClicked == repeat)
    {
        //[UserButtonCode_repeat] -- add your button handler code here..
        updateLoopState(repeat->getToggleState());
        //[/UserButtonCode_repeat]
    }

    //[UserbuttonClicked_Post]
	else {
		int fileNo = 0;
        // go through all file UIs
		for (const auto& file : audioFiles) {
			if (buttonThatWasClicked == file->muteButton) {
				processor.setMute(file->muteButton->getToggleState(), fileNo);
			}
			if (buttonThatWasClicked == file->openFileButton)
			{
                // fileno > max is the recording file
                bool isRecorder = fileNo == getNumFiles(true);

				String formats = formatManager.getWildcardForAllFormats();
				// MP3 not recommended, positioning not reliable
                formats = formats.replace("*.mp3;", "", true);

				// chose a file; recording file -> display different text and use save dialog
				FileChooser chooser(String::formatted(isRecorder ? "Select file to record" : "Select file %d to play...", fileNo+1),
					File::nonexistent, formats);
				if ( (!isRecorder && chooser.browseForFileToOpen()) || (isRecorder && chooser.browseForFileToSave(true)) )
				{
					// if Player is active, stop it
					if (isPlaying()) {
						changeState(Stopping);
						while (getTransportState() != Stopped);
					}
                    // no playing possible during file load
					changeState(NoFile);

                    // load the file in the procesor
					if (!processor.openFile(chooser.getResult(), fileNo)) {
						AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
							"File open Error",
							"File could not be opened! Probably the file is corrupt.");
                    }

                    // update filename & thumbnail, also after error (sets it empty)
                    updateFilename(fileNo);
                    updateThumbnail(fileNo);
                    // reset mute state and enable button
                    file->muteButton->setToggleState(false, dontSendNotification);
                    file->muteButton->setEnabled(true);

                    updateLoopState(repeat->getToggleState());

                    // any file loaded allows playing again
                    if (processor.isReadyToPlay())
                        changeState(Stopped);
                }
			}
			fileNo++;
		}
	}
    //[/UserbuttonClicked_Post]
}

void AudioPlayerEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == timeSlider)
    {
        //[UserSliderCode_timeSlider] -- add your slider handling code here..
		int64 pos = (int64)(int(timeSlider->getValue() / 100 * getTotalLength())-1);
		setReadPosition(pos);
        //[/UserSliderCode_timeSlider]
    }
    else if (sliderThatWasMoved == zoomSlider)
    {
        //[UserSliderCode_zoomSlider] -- add your slider handling code here..
		for (const auto &file: audioFiles)
			file->thumbnail->setZoomFactor(zoomSlider->getValue());
        //[/UserSliderCode_zoomSlider]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
/**
    Set position in seconds
    @param pos position in seconds
*/
void AudioPlayerEditor::setPosition(double pos) {
    processor.setPosition(pos);
    updateTime(); // to update slider and label
}

/**
    Set position in samples
    @param pos position in samples
*/
void AudioPlayerEditor::setReadPosition(int64 pos) {
    processor.setReadPosition(pos);
    updateTime(); // to update slider and label
}

/**
    Do not show the audio thumbnail of the file with given index
    @param fileNo index of the file
*/
void AudioPlayerEditor::hideThumbnail(int fileNo)
{
    if (fileNo < getNumFiles(!processor.hasRecorderFile())) {
        AudioFileUiBundle *file = audioFiles[fileNo];
        if (file) {
            file->thumbnail->setVisible(false);
            file->thumbnail->setFile("");
        }
    }
}

/**
    Update thumbnail of given file number by reloading the file and repainting
    @param fileNo index of the file
*/
void AudioPlayerEditor::updateThumbnail(int fileNo)
{
    if (fileNo < getNumFiles(!processor.hasRecorderFile())) {
        AudioFileUiBundle *file = audioFiles[fileNo];
        if (file) {
            file->thumbnail->setFile(*processor.getFullPath(fileNo));
            file->thumbnail->setVisible(true);
            file->thumbnail->repaint();
        }
    }
}

/**
    Repaint all thumbnails, e.g. after stopping.
*/
void AudioPlayerEditor::repaintThumbnails() {
    for (const auto& file : audioFiles)
        file->thumbnail->repaint();
}

/**
    Update filename label of given file number by setting the file name and repainting
    @param fileNo index of the file
*/
void AudioPlayerEditor::updateFilename(int fileNo)
{
    if (fileNo < getNumFiles()) {
        AudioFileUiBundle *file = audioFiles.getUnchecked(fileNo);
        file->filename->setText(*processor.getFilename(fileNo), dontSendNotification);
    }
}

/**
    Get number of files, optionally including a possible recorder.
    Same time, getNumFiles(true) is also equal to the index of the recorder file (if a recorder is there).
    @param      playbackOnly    if true, get number of playback files only;
                                default is false, recorder file will be counted
                                - this makes sense as you can also play it
    @returns    the number of files for playback, by default including the recorder file
    @see        AudioPlayerPlugin::getNumFiles
*/
int AudioPlayerEditor::getNumFiles(bool playbackOnly) {
    const ScopedLock lo(layoutLock);

	return processor.getNumFiles(playbackOnly);
}

/**
    Get drawing area for file UI

    @returns a rectangle of the area
*/
Rectangle<int> AudioPlayerEditor::getFileArea() {
	return Rectangle<int>(getLocalBounds().reduced(8).withTop(Y_OFFSET));
}

/**
    Get height for one line of file UI

    @returns height in pixels
*/
unsigned AudioPlayerEditor::getFileLineHeight() {
    return Y_LINE_HEIGHT;
}

/**
    Update the file UI.
    - Updates repeat/loop button status
    - Shows record button if a recorder is there
    - Reconstructs the entire UI if number of files was changed.
*/
void AudioPlayerEditor::updateUiLayout()
{
    bool hasRecorder = processor.hasRecorder();
    record->setVisible(hasRecorder);

    // restore loop status
    if (processor.isLooping())
        repeat->setToggleState(true, dontSendNotification);
    
    int numFiles = getNumFiles();

    // number of files changed
    if (isVisible() && (numFiles != oldNoOfFiles || audioFiles.size() != numFiles)) {
        Logger::outputDebugString("Updating UI layout");
        const ScopedLock lo(layoutLock);

        fileUiInitialized = false;
        audioFiles.clear();

        setSize(getLocalBounds().getWidth(), (Y_OFFSET + numFiles * getFileLineHeight()));

        // recreate entire file UIs
		for (int i = 0; i < numFiles; i++) {
			// fileindex == number of playback files -> this must be the recorder
			bool recorder = (i == (numFiles - hasRecorder));

			ImageButton* openFile;
			addAndMakeVisible(openFile = new ImageButton("openFile"));
			openFile->setButtonText(String());
			openFile->addListener(this);
			openFile->setImages(false, true, true,
				ImageCache::getFromMemory(!recorder ? folder_png : record_png, !recorder ? folder_pngSize : record_pngSize), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000));

			Label* filename;
			addAndMakeVisible(filename = new Label("filename",
				String()));
			filename->setFont(Font(15.00f, Font::plain));
			filename->setJustificationType(Justification::centredLeft);
			filename->setEditable(false, false, false);
			filename->setColour(TextEditor::textColourId, Colours::black);
			filename->setColour(TextEditor::backgroundColourId, Colour(0x00000000));

			ThumbnailComp* thumbnail;
			addAndMakeVisible(thumbnail = new ThumbnailComp(formatManager, *this, *zoomSlider.get()));
			thumbnail->setFile(*processor.getFullPath(i));

			TextButton* muteButton;
			addAndMakeVisible(muteButton = new TextButton("mute"));
            muteButton->setColour(TextButton::textColourOnId, Colours::grey);
			muteButton->addListener(this);
            muteButton->setClickingTogglesState(true);
            // restore mute status
            muteButton->setToggleState(processor.isMuted(i), dontSendNotification);

            // set file name
            const String* name = processor.getFilename(i);
            if (name->isEmpty()) {
                name = &NO_FILE;
                // mute button makes no sense
                muteButton->setEnabled(false);
            }
            filename->setText(*name, dontSendNotification);

			audioFiles.add(new AudioFileUiBundle(openFile, filename, thumbnail, muteButton));
		}
		fileUiInitialized = true;
		resized();
        repaint();
    }
	oldNoOfFiles = numFiles;
}

/**
    Set Play/Pause button to play or pause icon
    @param play true for play icon, false for pause icon
*/
void AudioPlayerEditor::updatePlayPauseButton(bool play)
{
	playPause->setImages(false, true, true,
		ImageCache::getFromMemory(play ? play_png : pause_png, play ? play_pngSize : pause_pngSize), 1.000f, Colour(0x00000000),
		Image(), 1.000f, Colour(0x00000000),
		Image(), 1.000f, Colour(0x00000000));
}

/**
    Checks if the recorder is currently recording.
    @returns false if there is no recorder
*/
bool AudioPlayerEditor::isRecording() {
    return (processor.hasRecorder() && processor.getRecordState() >= Stopped);
}

/**
    Updates the UI if status was changed between isRecording() <-> !isRecording().
    - updates the status image of the record button (pressed/not pressed)
    - updates the UI and processor for playback when recording was stopped
*/
void AudioPlayerEditor::updateRecordState()
{
	if (isRecording() != wasRecording) {
		wasRecording = isRecording();
        // update button image
		record->setImages(false, true, true,
			ImageCache::getFromMemory(isRecording() ? record_png : reclightred_png, isRecording() ? record_pngSize : reclightred_pngSize), 1.000f, Colour(0x00000000),
			Image(), 1.000f, Colour(0x00000000),
			Image(), 1.000f, Colour(0x00000000));

        // UI actions when recording was stopped
        if (!isRecording()) {
            recordingStopped();
        }
	}
}

/**
    Handling for record key pressed.
    Will prepare or stop recording.
    Recording is started with play button.
    Punch-In not yet supported.
    @returns true for success, false if there was an error (e.g. opening the file) or there is no recorder
*/
bool AudioPlayerEditor::toggleRecordState() {
    if (!processor.hasRecorder())
        return false;

    bool ret = true;

    // do nothing when not at the beginning or currently playing or starting/recording and record key pressed
    // (punch-in/out is not suppoerted)
    if (getCurrentPosition() > 0 || getTransportState() >= Paused || processor.getRecordState() >= Starting)
        ;

    // file not opened and record key pressed
    else if (processor.getRecordState() == NoFile) {
        // prepare recording
        ret = prepareRecording();
        // roll back if not successful
        if (!ret)
            recordingStopped();
    }

    // stop and unload for all other cases
    else {
        ret = processor.changeRecordState(Unloading);
        recordingStopped();
    }

    return ret;
}

/**
    Prepare UI and recorder for recording.
    - switch to LiveScrollingAudioDisplay
    - prepare UI status
    - prepare file for writing
    @returns true for success, false if there was an error (e.g. opening the file) or there is no recorder
*/
bool AudioPlayerEditor::prepareRecording() {
    if (!processor.hasRecorder())
        return false;

    // hide file thumbnail, show audio input
    hideThumbnail(getNumFiles(true));
    audioInputThumb->setVisible(true);

    // turn repeat off, makes no sense for recording
    repeat->setToggleState(false, NotificationType::sendNotification);
    repeat->setEnabled(false);

    // keep mute status for later, unmute recording channel
    wasMuted = audioFiles[getNumFiles(true)]->muteButton->getToggleState();
    audioFiles[getNumFiles(true)]->muteButton->setToggleState(false, dontSendNotification);
    audioFiles[getNumFiles(true)]->muteButton->setEnabled(false);

    // unload recorder file from player to allow overwriting
    processor.unloadRecorderFile();

    // prepare for recording (open file)
    return processor.changeRecordState(Stopping);
}

/**
    UI and processor actions to do when recording was stopped.
    - load recording file for playback
    - show audio thumbnail
*/
void AudioPlayerEditor::recordingStopped() {
    // hide audio input
    audioInputThumb->setVisible(false);

    // reload file to allow playback and restore mute state & button state, show file thumbnail
    processor.reloadRecorderFile(wasMuted);
    if (audioFiles[getNumFiles(true)]) {
        audioFiles[getNumFiles(true)]->muteButton->setToggleState(wasMuted, dontSendNotification);
        audioFiles[getNumFiles(true)]->muteButton->setEnabled(true);
        updateThumbnail(getNumFiles(true));
    }
}

/**
    Set all files to loop/not loop mode
    @param shouldLoop true to turn on loop mode
*/
void AudioPlayerEditor::updateLoopState(bool shouldLoop)
{
	processor.setAllLooping(shouldLoop);
}

/**
    Calculate and display time in min./sec./frames, if it was changed.
    Move time slider.
*/
void AudioPlayerEditor::updateTime() {
	double pos = getCurrentPosition();

	if (pos != previousPos) {
		previousPos = pos;

		double len = getLengthInSeconds();

		// don't send notification, as this would trigger sliderValueChanged -> setNextReadPosition (kinda notification endless loop)
		timeSlider->setValue(pos / len * 100, dontSendNotification);

		// calculate and display time
		const RelativeTime rPos(pos);
		const int minutes = ((int)rPos.inMinutes()) % 60;
		const int seconds = ((int)rPos.inSeconds()) % 60;
		const int frames = (int)(rPos.inMilliseconds() % 1000 * 0.075); // audio frames=1/75sec
		const String time(String::formatted("%02d:%02d:%02d", minutes, seconds, frames));

		timeLabel->setText(time, dontSendNotification);

        if (pos > 0 && !isRecording())
            back->setEnabled(true);
        else
            back->setEnabled(false);
	}
}

/**
    Timer callback to update the UI if needed, mainly
    - time display
    - transport state changed
    - recorder added/removed
    - number of files changed
*/
void AudioPlayerEditor::timerCallback() {

	TransportState state = getTransportState();

	if (isPlaying())
		updateTime();

    // transport state changed
	if (state != oldState)
	{
        const ScopedLock lo(layoutLock); 
        
        oldState = state;

        if (fileUiInitialized) {
            // update button enable status and play/pause bitmap according to transport state
            switch (state)
            {
            case NoFile:
                back->setEnabled(false);
                playPause->setEnabled(false);
                stop->setEnabled(false);
                repeat->setEnabled(false);
                timeSlider->setEnabled(false);
                break;
            case Stopped:
                updatePlayPauseButton(true);
                playPause->setEnabled(true);
                stop->setEnabled(false);
                back->setEnabled(false);
                repeat->setEnabled(true);
                timeSlider->setEnabled(true);
                updateTime();
                repaintThumbnails();
                break;
            case Playing:
                updatePlayPauseButton(false);
                stop->setEnabled(true);
                repeat->setEnabled(!isRecording());
                timeSlider->setEnabled(!isRecording());
                back->setEnabled(!isRecording());
                break;
            case Paused:
                updatePlayPauseButton(true);
                updateTime();
                break;
            default:
                break;
            }
        }
	}

	bool hasRecorder = processor.hasRecorder();

    // recorder was added or removed
	if (hasRecorder != hadRecorder) {
		hadRecorder = hasRecorder;
        if (hasRecorder) {
            processor.recorder->addAudioCallback(audioInputThumb);
            processor.reloadRecorderFile(false);
        }
        updateRecordState();
	}
    else if (hasRecorder)
		updateRecordState();

    // update UI layout if needed (checked inside method)
    updateUiLayout();
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AudioPlayerEditor" componentName=""
                 parentClasses="public AudioProcessorEditor, private Timer, public ButtonListener, public SliderListener"
                 constructorParams="AudioPlayerPlugin &amp;processor" variableInitialisers="AudioProcessorEditor(processor), processor(processor)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="750" initialHeight="185">
  <BACKGROUND backgroundColour="ff5e5e5e"/>
  <IMAGEBUTTON name="back" id="6004726561a75f83" memberName="back" virtualName=""
               explicitFocusOrder="1" pos="8 8 40 24" buttonText="" connectedEdges="0"
               needsCallback="1" radioGroupId="0" keepProportions="1" resourceNormal="back_png"
               opacityNormal="1" colourNormal="0" resourceOver="" opacityOver="1"
               colourOver="0" resourceDown="" opacityDown="1" colourDown="0"/>
  <IMAGEBUTTON name="playPause" id="30084e5ef045db45" memberName="playPause"
               virtualName="" explicitFocusOrder="2" pos="108 8 40 24" buttonText=""
               connectedEdges="0" needsCallback="1" radioGroupId="0" keepProportions="1"
               resourceNormal="play_png" opacityNormal="1" colourNormal="0"
               resourceOver="" opacityOver="1" colourOver="0" resourceDown=""
               opacityDown="1" colourDown="0"/>
  <IMAGEBUTTON name="stop" id="254c53ebeb5bf50b" memberName="stop" virtualName=""
               explicitFocusOrder="3" pos="58 8 40 24" buttonText="" connectedEdges="0"
               needsCallback="1" radioGroupId="0" keepProportions="1" resourceNormal="stop_png"
               opacityNormal="1" colourNormal="0" resourceOver="" opacityOver="1"
               colourOver="0" resourceDown="" opacityDown="1" colourDown="0"/>
  <SLIDER name="timeSlider" id="9ea57500d69f6164" memberName="timeSlider"
          virtualName="" explicitFocusOrder="4" pos="10 40 622 24" thumbcol="ffa6a6b4"
          min="0" max="100" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="55" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="timeLabel" id="906dcb6c6fd6022" memberName="timeLabel"
         virtualName="" explicitFocusOrder="0" pos="648 40 72 24" edTextCol="ff000000"
         edBkgCol="0" labelText="" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default sans-serif font" fontsize="18"
         kerning="0" bold="0" italic="0" justification="33"/>
  <IMAGEBUTTON name="record" id="65adc4db1b11afcd" memberName="record" virtualName=""
               explicitFocusOrder="0" pos="158 8 40 24" buttonText="" connectedEdges="0"
               needsCallback="1" radioGroupId="0" keepProportions="1" resourceNormal="reclightred_png"
               opacityNormal="1" colourNormal="0" resourceOver="" opacityOver="1"
               colourOver="0" resourceDown="" opacityDown="1" colourDown="0"/>
  <SLIDER name="zoom" id="158cc789a211c210" memberName="zoomSlider" virtualName=""
          explicitFocusOrder="0" pos="694 7 33 26" tooltip="zoom" min="0"
          max="1" int="0" style="RotaryHorizontalVerticalDrag" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="zoomLabel" id="8af66e196b5cf0ee" memberName="zoomLabel"
         virtualName="" explicitFocusOrder="0" pos="648 8 53 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Zoom" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         kerning="0" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="repeat" id="d0e28cd35acebf04" memberName="repeat" virtualName=""
              explicitFocusOrder="0" pos="208 8 60 24" textColOn="ff808080"
              buttonText="repeat" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: back_png, 2815, "../../../../Icons/back.png"
static const unsigned char resource_AudioPlayerEditor_back_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,131,0,0,0,112,8,2,0,0,0,178,249,84,134,0,0,0,6,116,82,78,83,0,199,0,192,0,201,70,
132,223,14,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,10,159,73,68,65,84,120,156,237,157,121,84,19,215,30,199,111,54,22,11,65,12,8,40,40,238,160,214,5,228,21,129,22,208,106,229,137,158,
231,177,62,173,122,20,183,90,139,213,227,6,86,229,180,175,174,96,17,8,4,66,8,73,8,155,200,177,62,91,247,21,21,20,20,170,207,90,81,4,4,217,247,42,178,37,100,242,254,136,5,140,40,75,102,238,157,73,248,252,
21,39,153,223,239,75,62,102,150,59,119,18,218,157,180,44,48,0,9,160,163,14,48,192,27,6,76,144,5,102,159,94,93,87,91,87,94,89,38,147,181,1,0,88,44,189,97,86,195,57,28,14,141,70,35,38,27,229,41,43,47,43,
42,42,146,203,101,116,58,221,202,202,106,148,237,104,38,243,189,111,120,207,38,148,74,229,253,7,57,233,25,233,25,153,233,85,149,149,106,207,154,155,155,187,56,187,185,185,186,57,58,56,209,233,3,159,48,
0,0,80,40,20,103,207,159,57,145,154,92,90,86,218,117,185,49,219,196,219,203,251,171,165,43,216,108,246,187,107,209,62,188,199,78,187,113,93,34,141,45,42,46,234,177,189,141,205,136,117,62,27,60,220,61,
251,24,91,219,184,151,125,151,23,197,253,192,59,102,58,216,116,231,246,221,174,46,174,106,203,223,107,66,38,147,29,13,9,188,124,249,98,159,114,184,127,230,177,219,111,175,161,129,97,159,214,210,14,74,
203,74,163,162,195,51,110,103,244,248,74,58,141,190,107,135,191,215,188,249,93,23,118,111,162,186,170,234,251,0,191,130,194,130,126,4,178,177,182,57,116,32,200,198,218,166,31,235,82,148,166,166,166,248,
196,184,147,191,164,202,219,229,189,92,133,193,96,132,29,11,159,60,105,74,199,146,110,76,52,54,54,126,187,101,99,73,201,139,126,39,179,176,180,228,135,11,76,77,135,244,187,2,85,192,48,236,194,197,115,
49,34,65,67,67,125,95,215,29,97,51,82,18,27,223,177,115,85,223,199,202,229,178,61,1,254,154,104,0,0,84,85,86,250,239,217,217,210,218,162,73,17,242,243,199,163,135,155,54,111,8,10,62,210,15,13,0,128,23,
37,197,55,111,221,232,248,167,186,9,105,66,220,31,143,30,106,20,16,0,0,64,222,179,60,97,172,64,243,58,228,164,186,186,122,255,161,31,183,108,243,125,154,247,84,147,58,55,110,93,239,120,252,150,137,138,
138,242,227,41,73,154,148,238,202,169,211,39,159,23,63,199,171,26,73,104,109,109,149,72,69,171,214,44,191,122,237,138,82,169,212,176,218,227,220,63,59,30,191,101,34,46,33,174,247,251,156,30,193,48,76,
18,23,139,87,53,228,96,24,118,230,236,175,43,86,45,147,72,69,173,109,173,184,212,172,169,169,233,208,217,121,102,215,220,220,124,245,218,37,92,26,116,112,235,214,205,250,250,186,33,67,56,248,150,133,79,
222,179,188,112,94,40,46,219,237,174,96,24,214,222,46,103,177,244,64,87,19,119,239,101,201,229,184,125,32,222,116,82,98,153,89,119,254,233,229,141,111,89,152,84,85,85,133,134,31,187,147,217,243,89,130,
134,116,110,157,50,179,238,16,209,32,51,235,54,17,101,33,208,210,210,34,16,242,87,175,93,1,65,3,232,250,153,200,47,120,70,68,131,252,130,254,156,30,162,69,169,84,94,190,124,49,70,28,93,83,83,3,173,105,
167,137,218,90,66,186,214,212,86,19,81,150,56,114,159,60,230,242,66,115,115,31,67,238,219,105,130,160,19,49,185,92,46,151,203,89,44,22,17,197,241,165,161,161,62,82,192,187,122,229,50,166,196,224,119,239,
52,129,41,136,106,175,80,40,72,110,66,46,151,37,38,39,164,164,38,183,180,32,27,23,232,219,149,34,173,36,237,198,117,161,40,90,237,90,2,124,116,218,196,243,162,194,112,94,216,239,247,115,80,7,1,64,103,
77,188,124,249,151,80,36,56,119,225,172,66,161,64,157,229,13,58,103,2,195,176,95,78,159,148,74,197,175,26,95,161,206,242,22,186,101,226,94,118,86,68,100,120,241,139,34,212,65,186,65,87,76,84,84,148,115,
121,97,112,206,150,251,135,246,155,104,106,106,18,73,132,191,254,246,95,28,135,153,137,64,155,77,96,24,118,238,252,25,137,52,182,182,174,14,117,150,158,209,90,19,143,254,124,24,193,227,62,201,123,130,
58,72,111,209,66,19,213,85,85,81,130,136,180,155,105,154,95,83,131,137,86,153,144,201,100,73,199,19,142,167,36,225,117,77,13,38,218,99,226,210,165,11,66,137,160,186,154,98,67,191,29,104,131,137,252,194,
252,8,94,216,131,255,221,71,29,68,35,168,109,226,175,134,6,158,32,2,213,56,54,190,80,213,132,92,46,79,61,153,146,116,60,225,245,235,215,168,179,224,3,37,77,100,102,222,230,241,195,75,74,75,80,7,193,19,
138,153,120,94,252,60,130,23,150,243,123,54,234,32,248,67,25,19,175,155,94,139,37,177,167,127,59,213,222,222,142,58,11,33,80,192,132,106,28,59,46,94,210,248,234,37,234,44,4,66,118,19,15,31,62,8,143,228,
62,203,207,67,29,132,112,200,107,162,162,178,130,27,17,74,230,113,108,124,33,163,137,230,230,230,132,36,233,201,83,169,109,109,109,168,179,192,131,92,38,84,147,239,4,34,126,109,109,45,234,44,176,33,145,
137,199,143,31,133,243,194,114,159,230,162,14,130,6,82,152,168,175,175,139,138,137,188,114,229,18,181,198,177,241,5,177,137,55,227,216,39,146,90,91,169,55,142,141,47,40,77,164,221,184,46,16,242,203,43,
202,16,102,32,15,104,76,20,20,22,240,162,184,36,153,124,71,18,96,155,232,199,77,228,58,2,84,19,217,57,247,142,28,61,168,131,71,168,189,1,158,137,107,105,87,15,30,254,137,60,243,80,201,6,164,239,1,122,
150,159,119,56,240,192,128,134,15,0,195,132,18,40,127,62,22,136,251,141,173,90,6,12,19,57,57,217,26,126,171,130,46,0,195,196,173,244,155,16,186,80,29,24,38,158,232,234,80,82,159,128,97,162,182,110,224,
176,181,103,6,190,67,145,44,192,48,97,198,49,131,208,133,234,192,48,97,55,193,30,66,23,170,3,195,196,167,110,159,65,232,66,117,96,152,112,116,156,49,97,252,4,8,141,40,13,12,19,52,64,219,185,221,159,228,
95,24,129,28,72,199,78,227,198,142,255,222,127,31,131,193,128,211,142,138,192,59,138,157,229,49,251,200,193,163,166,131,77,161,117,164,22,80,207,39,156,102,252,67,28,35,157,239,181,128,78,27,56,143,81,
7,246,59,50,216,212,116,215,14,255,56,113,130,243,39,46,144,91,147,28,52,255,55,109,172,71,28,57,24,20,28,20,98,59,210,22,73,0,18,130,114,43,225,232,224,36,140,150,108,246,221,250,209,71,70,8,99,144,4,
196,219,107,38,147,249,229,162,37,201,241,41,139,23,45,209,241,31,18,33,197,31,207,102,155,124,231,187,53,58,82,56,117,202,52,212,89,144,65,10,19,42,198,141,29,31,118,44,226,208,254,64,43,11,43,212,89,
16,64,34,19,42,92,102,186,74,68,9,95,175,255,102,208,160,65,168,179,64,133,116,38,0,0,250,250,250,203,151,173,148,138,146,230,204,249,66,119,126,24,140,140,38,84,152,153,153,237,245,15,136,138,16,76,156,
56,25,117,22,24,144,215,132,10,187,9,246,188,176,168,61,126,251,56,67,180,252,114,19,217,77,0,0,104,52,218,220,185,243,18,164,201,62,171,214,234,233,233,161,142,67,20,20,48,161,194,208,192,208,103,213,
218,4,201,241,57,115,190,64,157,133,16,40,99,66,197,208,161,67,247,250,7,132,252,204,29,51,122,12,234,44,56,67,49,19,42,166,79,115,136,225,139,247,248,237,211,166,49,118,74,154,0,0,208,233,244,185,115,
231,197,75,146,150,47,91,201,98,106,195,213,64,170,154,80,97,100,100,252,245,250,111,68,66,233,76,103,245,159,17,165,28,212,54,161,194,198,218,230,240,129,192,224,160,16,91,219,81,168,179,244,31,109,48,
161,194,209,193,73,200,23,111,246,221,106,100,68,201,49,118,237,49,1,254,30,99,79,146,158,160,226,24,59,197,226,246,6,54,155,253,157,239,86,177,48,222,105,198,39,168,179,244,1,45,52,161,98,228,136,145,
71,143,4,31,218,31,56,204,114,24,234,44,189,66,107,77,168,112,153,233,26,39,78,220,236,187,149,252,99,236,90,110,2,0,192,98,177,190,92,180,36,94,148,188,96,254,66,50,207,238,33,111,50,124,225,152,113,
118,108,243,139,228,9,38,79,250,24,117,150,238,209,21,19,42,236,198,219,133,135,70,254,24,176,223,194,194,2,117,22,117,116,203,4,0,128,70,163,121,184,123,198,197,38,146,109,140,93,231,76,168,48,48,48,
240,89,181,54,49,46,133,60,23,104,117,212,132,10,115,115,243,189,254,1,161,193,220,177,99,198,161,206,162,219,38,84,76,157,50,93,16,21,187,199,111,159,169,233,16,132,49,58,77,208,25,68,89,33,255,109,19,
111,198,216,197,137,203,151,173,68,117,199,77,231,187,111,104,96,72,68,3,22,139,69,149,187,137,84,99,236,98,97,188,167,187,39,252,238,157,38,44,45,44,137,104,96,105,73,177,9,125,214,195,173,127,8,216,
31,28,20,50,202,118,52,204,190,157,38,198,16,179,215,26,55,22,253,206,176,31,56,58,56,9,163,197,59,183,249,153,152,152,192,233,216,105,194,213,197,141,136,6,46,206,132,148,133,0,131,193,240,158,191,48,
94,156,188,120,209,18,8,187,186,78,19,142,14,51,12,12,12,240,173,206,100,50,157,157,103,226,91,19,50,170,49,118,30,151,79,244,84,196,78,19,250,250,250,30,238,179,241,173,238,234,226,102,164,21,119,169,
216,77,176,143,228,242,113,31,99,103,48,24,204,191,167,67,188,117,228,186,97,237,6,28,63,22,44,22,107,211,198,205,120,85,35,3,46,51,93,99,133,113,171,87,174,209,215,215,199,165,160,153,153,89,199,25,254,
91,38,56,28,179,37,139,151,226,210,3,0,176,208,251,95,4,29,143,33,196,208,192,112,141,207,58,169,40,201,211,99,150,230,213,38,218,79,234,120,172,126,54,183,102,245,58,92,102,172,76,159,230,176,105,163,
175,230,117,200,137,133,133,197,15,251,126,226,134,240,198,141,29,175,73,157,217,158,115,59,30,211,238,164,101,169,61,221,216,216,248,237,150,141,37,37,47,250,221,192,194,210,146,31,46,64,59,120,0,7,12,
195,206,95,60,43,20,197,52,52,212,247,117,221,145,35,108,197,66,105,199,204,135,110,70,56,140,141,141,121,97,252,233,211,28,250,23,206,222,126,98,100,24,95,23,52,0,0,232,116,250,124,175,5,241,146,164,
165,255,94,222,167,169,136,12,6,99,215,14,191,174,19,80,24,235,125,54,188,251,58,125,125,253,89,30,179,43,171,43,11,11,11,250,148,204,253,51,143,253,255,57,196,102,67,58,27,34,9,122,122,122,78,142,78,
179,60,63,175,172,42,239,205,175,236,209,105,244,93,219,253,93,93,62,237,186,176,155,173,83,87,158,60,205,229,11,34,123,243,147,162,83,62,158,186,105,163,175,189,221,196,30,95,169,221,220,203,190,203,
139,226,22,21,23,189,239,5,166,131,77,119,110,223,237,234,162,190,51,238,193,4,0,64,169,84,222,127,144,147,158,145,158,145,153,94,85,89,169,246,172,185,185,185,139,179,155,155,171,155,163,131,19,229,38,
123,17,132,66,161,56,123,254,204,137,212,228,210,178,210,174,203,141,217,38,222,94,222,95,45,93,193,102,179,223,93,171,103,19,93,169,171,173,43,175,44,147,201,218,0,0,44,150,222,48,171,225,28,14,135,36,
215,188,72,72,89,121,89,81,81,145,92,46,163,211,233,86,86,86,163,108,71,51,153,239,253,34,247,190,153,24,128,56,254,15,125,197,164,73,136,14,56,163,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* AudioPlayerEditor::back_png = (const char*) resource_AudioPlayerEditor_back_png;
const int AudioPlayerEditor::back_pngSize = 2815;

// JUCER_RESOURCE: pause_png, 1687, "../../../../Icons/pause.png"
static const unsigned char resource_AudioPlayerEditor_pause_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,112,0,0,0,112,8,2,0,0,0,73,130,99,28,0,0,0,6,116,82,78,83,0,199,0,192,0,201,70,
132,223,14,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,6,55,73,68,65,84,120,156,237,157,107,76,83,103,28,198,207,41,180,80,76,91,145,90,52,17,40,160,139,34,226,220,162,165,5,167,14,148,
184,139,113,204,219,116,23,227,116,91,182,44,75,220,212,185,37,40,4,166,38,91,246,105,97,67,197,107,28,106,226,28,110,51,234,16,39,20,166,128,219,156,87,20,48,155,147,84,139,109,21,74,171,237,97,31,186,
152,138,147,246,188,239,211,197,100,255,223,87,222,231,255,62,253,245,28,122,206,249,114,196,198,99,39,132,200,32,73,146,253,198,13,199,45,167,32,8,162,32,14,213,235,227,227,135,68,104,175,16,53,236,118,
135,203,17,168,161,79,72,24,50,36,33,114,219,69,195,39,186,156,14,107,163,213,218,88,215,220,210,236,245,122,131,255,164,213,104,77,38,115,142,101,178,105,146,73,29,171,134,111,125,95,13,151,179,161,209,
90,223,80,215,210,210,236,241,122,238,175,161,49,153,204,150,236,220,108,147,89,173,6,215,16,129,71,168,203,233,216,180,101,227,193,67,63,248,124,190,129,87,106,53,154,5,243,23,205,41,156,167,82,169,80,
187,223,163,187,251,246,150,109,149,213,7,246,223,245,221,253,239,107,192,132,158,108,250,121,221,134,50,135,211,17,126,36,37,217,88,188,166,212,152,98,132,20,8,208,114,170,169,108,125,233,205,155,93,
225,71,146,146,146,139,139,74,211,82,211,32,5,48,66,183,110,175,220,182,99,75,95,95,159,220,96,108,108,236,199,171,215,76,206,153,204,223,65,16,132,93,85,59,55,85,86,72,146,36,55,168,82,169,86,190,191,
58,63,111,58,127,135,168,165,139,151,113,142,216,249,245,142,202,173,155,216,178,62,159,239,167,227,181,35,211,31,75,78,74,230,172,177,123,207,174,175,54,150,51,124,169,130,32,248,253,254,250,134,58,99,
74,26,255,233,162,224,204,255,88,115,100,115,101,5,207,4,73,146,74,202,138,46,92,60,207,51,164,166,182,230,203,141,229,156,53,202,214,23,159,59,127,150,103,136,192,41,180,179,243,218,167,159,111,96,59,
40,130,241,122,189,197,37,69,253,46,9,194,199,102,179,125,134,168,113,231,206,157,226,210,162,94,79,47,207,16,46,161,229,21,95,120,60,158,208,235,194,160,211,214,89,181,119,23,91,182,98,115,185,219,237,
134,212,176,217,108,85,85,140,53,2,176,11,189,208,122,161,174,254,56,207,222,253,216,179,119,247,173,91,46,185,169,75,109,151,106,107,143,2,107,236,221,183,219,233,144,113,173,210,15,118,161,223,86,127,
195,127,150,5,211,211,211,125,184,230,176,220,212,129,234,253,82,159,236,159,245,1,112,187,221,135,142,28,100,142,51,10,245,251,253,245,13,245,204,187,62,12,171,181,78,214,122,73,146,234,26,228,69,194,
129,231,163,49,10,189,120,241,252,109,249,167,103,72,126,63,115,186,187,167,59,252,245,173,151,91,29,142,155,240,26,103,207,157,97,248,231,19,128,85,232,229,86,182,224,192,248,124,190,142,142,246,240,
215,95,190,20,145,26,146,36,181,181,183,177,101,25,133,222,184,126,157,45,24,146,235,114,38,203,90,44,11,230,15,200,40,180,23,116,153,242,32,61,238,158,240,23,187,61,17,171,209,43,163,70,48,172,63,74,
242,239,151,195,157,236,15,241,164,42,24,201,31,169,26,33,31,152,61,12,222,91,79,162,31,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,
67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,
66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,
193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,67,66,193,144,80,48,36,20,12,9,5,195,40,84,161,16,177,61,130,38,203,168,244,136,212,184,47,200,22,139,141,216,155,141,227,212,131,30,133,26,
131,6,201,168,17,12,163,208,196,196,68,182,32,118,178,193,16,177,26,67,25,39,51,10,77,79,77,103,11,14,140,66,161,48,166,164,134,191,126,100,90,100,106,136,138,84,35,227,171,146,25,133,102,102,102,233,
116,58,182,236,0,140,203,204,210,106,181,225,175,31,61,58,35,18,47,139,31,147,49,118,112,124,60,91,150,249,71,73,97,50,153,217,178,3,96,49,231,200,173,97,158,20,137,26,22,230,44,251,101,83,65,126,1,115,
246,95,81,169,84,211,166,228,201,77,205,152,1,174,161,140,86,62,61,53,159,57,206,46,244,201,39,38,102,155,216,191,201,7,153,55,103,129,193,96,144,155,26,159,53,33,199,146,11,172,81,88,56,119,248,176,225,
204,113,174,11,251,215,151,44,139,138,138,226,153,112,15,157,78,55,127,238,75,108,217,165,75,222,136,142,142,134,212,208,106,52,11,231,47,228,153,192,37,116,84,250,168,15,150,175,228,153,16,64,169,84,
150,149,172,211,104,52,108,241,84,99,218,138,229,171,0,53,162,149,165,37,235,117,186,193,60,67,120,111,61,103,22,60,59,235,185,217,156,67,222,125,231,189,204,177,89,60,19,10,102,204,44,156,61,135,179,
198,91,111,190,157,53,110,60,231,144,168,165,139,151,113,142,200,54,153,69,81,252,237,244,175,12,217,152,152,152,143,62,44,154,89,240,12,103,7,65,16,38,77,52,197,196,196,156,250,165,133,33,171,140,86,
174,88,190,106,214,243,188,71,134,0,17,42,138,226,227,227,39,232,19,244,77,45,39,37,57,239,251,212,233,116,165,107,63,145,123,169,52,64,141,113,153,89,137,134,196,166,230,147,126,191,63,252,160,86,163,
45,89,91,54,229,169,105,152,26,141,199,78,64,6,9,130,208,101,239,218,186,99,243,247,7,191,11,169,53,46,46,238,229,133,175,190,48,251,69,117,4,110,198,93,46,231,246,157,219,246,87,239,11,169,85,173,86,
191,178,232,53,108,13,164,208,0,109,237,109,181,199,106,172,13,245,29,87,250,191,54,90,33,42,198,100,140,205,181,228,230,77,205,55,68,236,105,64,128,142,43,237,71,107,107,172,13,245,237,29,253,95,27,173,
16,21,163,199,100,228,88,114,242,166,77,31,150,56,12,187,47,94,232,61,236,118,251,213,191,254,116,58,157,130,32,8,162,104,208,235,71,140,72,150,117,103,9,161,203,222,117,245,218,31,14,199,63,53,134,38,
36,36,37,37,107,181,248,251,230,0,17,20,250,255,132,158,216,131,33,161,96,72,40,152,191,1,107,78,224,127,227,223,49,82,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* AudioPlayerEditor::pause_png = (const char*) resource_AudioPlayerEditor_pause_png;
const int AudioPlayerEditor::pause_pngSize = 1687;

// JUCER_RESOURCE: play_png, 2138, "../../../../Icons/play.png"
static const unsigned char resource_AudioPlayerEditor_play_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,112,0,0,0,112,8,2,0,0,0,73,130,99,28,0,0,0,6,116,82,78,83,0,199,0,192,0,201,70,
132,223,14,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,7,250,73,68,65,84,120,156,237,156,125,76,19,103,28,199,175,47,199,139,34,82,65,153,3,45,138,48,28,138,226,116,138,36,83,51,124,65,
112,19,21,121,41,148,182,180,64,219,107,203,96,128,76,76,102,112,47,52,76,98,166,137,49,134,49,54,231,140,46,38,42,115,47,217,144,45,12,247,190,225,16,69,133,161,110,85,193,65,69,197,74,123,221,31,55,
103,173,8,247,70,239,174,125,62,255,221,245,158,231,247,229,3,180,215,223,221,61,188,150,83,223,67,0,250,16,146,30,105,179,217,58,187,46,153,76,38,20,69,97,216,75,44,22,135,134,132,210,152,140,163,144,
17,58,48,48,112,240,208,129,134,147,199,205,102,179,227,254,208,144,208,205,169,233,107,215,36,11,133,228,127,79,92,135,71,244,95,190,249,187,230,234,154,170,190,190,127,158,116,128,120,122,24,162,209,
45,90,184,152,114,54,78,66,76,232,167,39,79,84,239,52,162,118,116,212,35,227,150,196,107,10,144,105,161,211,40,100,227,36,4,132,254,209,214,106,40,210,217,108,54,156,199,195,66,120,67,202,198,236,44,153,
223,120,63,178,241,184,7,94,161,40,138,202,114,179,47,95,233,38,90,64,20,32,202,85,168,214,174,73,230,243,249,196,227,113,15,188,63,228,55,223,54,145,176,9,65,80,95,127,95,245,78,99,190,70,217,218,250,
27,137,225,156,3,175,208,166,111,27,169,148,185,112,177,67,95,132,108,175,220,118,237,250,53,42,243,176,31,188,66,207,182,183,81,47,214,216,212,152,163,144,212,190,183,127,240,222,32,245,217,216,9,46,
161,118,187,189,183,183,151,150,122,22,139,165,254,64,221,230,204,77,71,142,30,70,209,209,207,22,56,7,46,161,40,138,226,255,112,199,195,192,45,243,238,61,187,180,6,117,251,185,179,52,78,203,6,152,252,
228,109,111,111,83,35,121,229,21,101,166,107,38,6,99,208,11,243,167,50,45,167,155,101,185,89,251,246,239,189,123,247,46,211,89,104,128,121,161,16,4,89,44,150,143,62,254,80,42,207,56,209,112,140,235,111,
172,172,16,138,209,123,243,102,117,141,81,131,228,181,181,157,97,58,11,121,88,36,20,227,92,199,57,164,80,179,189,114,219,141,235,215,153,206,66,6,214,9,133,32,200,110,183,55,54,53,74,21,146,186,250,218,
251,247,239,51,29,135,24,108,20,138,113,207,114,175,174,190,54,75,150,254,197,23,159,49,157,133,0,236,21,138,113,227,198,141,55,141,59,10,139,117,23,59,47,50,157,5,23,108,23,138,241,219,239,191,230,229,
43,222,168,170,236,239,235,99,58,203,40,112,67,40,4,65,168,29,253,242,203,207,179,228,25,31,125,252,225,208,16,123,223,88,57,35,20,227,246,237,219,251,246,239,85,168,114,90,78,55,51,157,101,120,56,38,
20,227,202,213,43,229,21,101,197,165,133,93,221,93,76,103,113,134,147,66,49,126,254,229,39,85,190,252,157,26,163,217,220,207,116,150,135,112,88,40,4,65,86,171,245,120,195,177,108,89,38,123,154,129,220,
22,138,113,107,224,214,238,61,187,228,74,233,15,63,158,102,58,139,91,8,197,232,190,252,103,105,249,171,229,21,101,38,211,223,12,198,112,31,161,24,45,167,155,165,114,201,187,123,118,221,185,115,135,145,
0,238,38,20,130,160,33,235,208,39,71,15,75,229,18,70,154,129,110,40,20,227,230,63,189,213,53,70,53,162,58,243,71,171,43,235,186,173,80,140,243,29,231,245,175,104,183,87,110,187,238,170,102,160,155,11,
133,30,52,3,115,20,146,186,250,90,139,197,50,214,229,220,95,40,134,99,51,208,110,183,143,93,33,79,17,138,209,211,211,243,166,113,135,70,159,63,118,151,175,61,75,40,70,123,251,89,173,174,224,141,170,202,
17,238,114,37,141,39,10,133,30,52,3,51,165,105,117,245,181,244,54,3,61,84,40,198,224,224,96,93,125,173,92,41,61,213,68,233,86,56,71,60,90,40,198,213,191,174,190,94,185,173,168,196,208,217,213,73,125,54,
32,244,63,126,249,245,103,85,129,252,157,26,163,185,159,210,85,22,32,244,33,54,155,237,120,195,177,108,185,228,200,209,195,164,111,142,3,66,157,249,191,25,248,253,15,45,36,134,3,161,195,115,249,74,119,
217,107,37,229,21,101,127,155,254,34,52,16,8,29,9,18,205,64,32,116,20,172,86,43,161,102,32,16,138,11,172,25,88,160,85,182,158,249,125,228,35,129,80,2,116,92,232,40,44,210,85,85,191,53,194,119,86,32,148,
24,168,29,61,249,89,131,92,41,253,241,167,225,31,152,3,66,201,208,111,238,223,178,181,244,171,198,175,30,127,9,8,37,137,205,102,123,219,184,227,194,197,14,167,253,64,40,121,134,134,134,170,119,86,57,181,
171,129,80,74,156,239,56,239,244,102,10,132,82,165,169,233,148,227,38,16,74,149,182,115,143,60,5,11,132,82,165,167,167,199,113,19,8,165,10,143,247,200,38,16,74,149,160,160,201,142,155,64,40,85,162,163,
162,29,55,129,80,170,188,248,98,130,227,38,16,74,137,168,200,168,5,177,11,29,247,0,161,228,129,97,184,184,168,212,105,39,16,74,18,129,64,176,165,180,34,98,86,164,211,126,207,93,164,142,10,65,129,129,101,
37,91,23,45,124,254,241,151,128,80,98,140,186,92,26,16,138,23,30,143,151,144,176,42,79,81,48,121,242,228,17,14,3,66,113,49,43,60,66,167,213,207,139,137,29,245,72,32,116,20,68,162,73,106,149,38,33,97,21,
206,181,251,128,208,39,2,195,112,234,198,180,204,116,137,159,223,4,252,163,128,208,225,137,91,18,175,85,235,72,44,130,12,132,58,19,22,54,67,167,209,63,183,96,17,185,225,64,232,67,252,252,252,100,57,185,
235,215,165,80,89,67,26,8,133,32,8,226,243,249,41,47,111,204,201,150,251,251,251,83,156,10,8,133,230,197,204,71,180,134,136,240,8,90,102,243,104,161,83,131,167,234,144,194,165,113,241,52,206,233,161,66,
199,141,27,167,144,171,94,78,94,15,195,48,189,51,123,156,80,62,143,159,180,54,89,150,157,27,24,20,56,22,243,123,150,208,232,232,185,58,141,62,234,153,217,99,87,194,83,132,6,7,7,171,243,144,101,47,44,231,
57,93,165,164,27,247,23,234,229,229,149,153,158,149,182,57,195,215,199,215,5,229,220,92,232,202,149,171,85,242,252,41,83,166,184,172,162,219,10,13,159,25,142,104,12,177,243,23,184,184,174,27,10,21,5,136,
212,121,90,252,13,55,122,113,43,161,176,16,78,221,148,150,145,38,153,48,129,64,195,141,94,220,71,232,146,197,75,181,106,100,90,232,116,102,99,184,131,208,48,113,152,78,107,32,221,112,163,23,110,11,29,
63,222,79,46,163,218,112,163,23,182,228,32,202,131,134,155,204,223,127,34,211,89,30,129,147,66,99,230,206,67,52,134,200,8,231,187,54,216,0,199,132,62,21,60,85,79,119,195,141,94,56,35,212,215,215,55,91,
146,179,49,37,213,219,219,155,233,44,35,193,1,161,216,45,27,249,185,234,160,160,32,166,179,140,14,219,133,206,158,29,173,215,26,102,71,61,203,116,16,188,176,87,232,164,73,129,5,74,245,202,149,171,199,
186,225,70,47,108,20,10,195,176,36,35,219,101,13,55,122,97,157,208,21,203,86,40,115,11,66,158,14,97,58,8,73,88,36,116,230,140,112,157,70,31,27,251,28,211,65,40,193,10,161,1,19,3,148,138,188,196,53,73,
2,129,128,233,44,84,97,88,168,64,32,88,255,210,6,153,84,193,96,195,141,94,152,20,58,39,122,46,162,53,68,69,70,49,152,129,118,112,9,21,8,4,66,161,208,106,181,210,85,245,233,169,33,136,70,207,230,111,144,
164,193,251,23,26,24,24,72,203,186,208,222,222,222,233,155,51,51,210,36,62,62,62,212,103,99,33,120,133,70,63,59,135,162,80,30,143,183,124,217,10,181,74,59,37,56,152,202,60,44,7,175,208,21,203,19,190,30,
110,17,24,156,68,204,138,212,105,13,49,115,231,145,158,129,43,224,21,26,31,23,47,158,30,214,125,249,79,162,5,68,1,34,101,110,94,226,234,36,70,174,65,186,30,188,63,36,159,207,47,41,46,37,116,158,8,11,225,
180,212,244,15,222,63,152,148,184,206,67,108,66,132,158,245,156,19,29,83,92,88,194,231,225,26,18,183,36,190,118,127,189,58,31,121,210,19,103,238,10,177,243,208,181,137,201,19,39,138,170,107,170,70,88,
252,77,60,61,12,209,232,22,45,92,76,57,27,39,225,181,156,26,126,13,183,17,24,24,24,56,120,232,64,195,201,227,102,179,217,113,127,104,72,104,234,166,180,164,196,117,236,185,6,233,122,200,8,197,176,217,
108,157,93,151,76,38,19,138,162,48,236,37,22,139,73,60,213,227,126,144,23,10,24,22,79,249,240,117,25,64,40,205,252,11,87,56,147,109,242,210,148,0,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* AudioPlayerEditor::play_png = (const char*) resource_AudioPlayerEditor_play_png;
const int AudioPlayerEditor::play_pngSize = 2138;

// JUCER_RESOURCE: stop_png, 1516, "../../../../Icons/stop.png"
static const unsigned char resource_AudioPlayerEditor_stop_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,115,0,0,0,112,8,2,0,0,0,162,181,216,31,0,0,0,6,116,82,78,83,0,199,0,192,0,201,70,
132,223,14,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,5,140,73,68,65,84,120,156,237,221,109,76,19,119,28,192,113,174,20,40,189,23,128,144,52,194,228,49,60,84,50,69,158,4,89,156,60,101,
241,97,111,22,167,99,67,167,243,33,217,18,221,100,46,186,108,209,109,108,46,232,8,38,155,178,105,220,22,31,152,217,166,16,162,60,45,147,226,124,66,81,24,160,160,2,11,176,0,166,12,180,216,22,108,185,219,
11,222,244,110,215,163,45,247,187,209,63,191,207,187,123,232,159,95,190,144,114,215,123,81,234,186,174,209,3,1,80,74,181,16,195,48,195,195,195,38,179,201,100,50,154,205,38,169,150,149,135,90,77,211,52,
173,246,165,3,3,3,41,138,146,100,205,25,149,213,235,245,183,154,26,239,52,55,221,127,248,96,104,112,208,98,181,72,50,211,255,200,219,219,59,36,248,185,184,56,109,210,146,228,148,228,20,63,63,127,151,151,
162,92,120,55,152,156,156,188,164,251,189,186,230,98,75,75,51,195,50,46,255,236,89,206,75,233,149,154,186,116,245,202,151,211,211,150,185,240,135,236,92,89,134,97,170,170,47,156,41,59,53,248,104,208,217,
159,228,190,34,35,162,54,110,216,188,98,249,10,167,94,229,68,217,214,214,150,195,95,151,244,252,213,237,244,104,68,88,188,40,225,221,29,5,145,17,145,14,158,239,80,89,171,213,90,250,221,145,243,21,191,
178,44,59,179,241,220,155,167,167,231,150,205,219,94,127,45,223,161,147,183,110,218,38,126,198,196,196,196,167,159,239,171,253,173,70,138,217,220,27,203,178,183,239,52,61,126,60,154,154,146,54,237,59,
175,66,252,176,209,104,252,96,111,193,213,107,87,164,27,207,237,85,84,150,23,126,177,223,106,181,138,159,38,86,214,60,110,222,189,119,87,107,219,159,146,14,70,130,250,134,250,253,159,125,204,48,98,215,
69,98,101,15,126,245,101,71,199,61,169,167,34,196,213,107,87,78,124,127,92,228,4,187,101,207,254,92,86,175,187,4,48,18,57,206,156,61,165,187,172,179,119,84,184,108,95,127,175,248,47,4,77,41,62,124,200,
96,120,34,120,72,160,44,203,178,135,138,139,8,184,85,149,193,152,225,73,233,177,163,130,135,4,202,222,188,117,163,173,189,21,120,36,114,212,214,85,247,246,245,254,119,191,64,217,31,79,254,0,63,15,57,24,
134,57,121,90,160,24,191,108,71,231,189,142,78,188,30,112,142,174,161,126,120,120,152,183,147,95,246,98,213,5,185,230,33,199,228,228,100,77,93,21,111,39,167,44,195,48,151,255,168,151,113,36,114,232,26,
248,221,56,101,239,63,232,52,140,141,201,56,15,57,186,123,186,70,71,71,108,247,112,202,226,37,129,203,88,150,109,191,219,110,187,135,83,182,171,167,75,222,121,136,210,205,173,199,41,219,223,223,39,239,
48,68,233,255,155,83,143,83,118,100,228,31,121,135,33,10,239,194,139,83,214,108,114,179,167,217,179,138,209,104,180,221,228,150,29,31,151,119,24,162,140,143,155,109,55,185,119,10,115,250,41,215,76,241,
158,17,78,243,180,6,185,12,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,
11,5,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,
80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,11,5,203,66,193,178,80,176,44,20,44,11,5,203,74,134,247,21,65,156,178,62,42,31,89,103,33,139,143,74,101,187,201,
41,235,235,235,43,239,48,68,161,213,180,237,38,167,236,188,128,121,242,14,67,148,128,128,0,219,77,78,217,208,208,112,89,103,33,75,120,104,132,237,38,167,108,88,88,184,172,179,144,37,148,91,143,83,54,49,
33,81,214,89,8,66,81,84,18,183,30,167,108,76,116,172,90,173,150,119,36,66,132,133,134,249,139,188,207,42,149,202,23,151,103,202,59,18,33,178,178,114,120,123,248,119,10,185,217,185,114,13,67,14,138,162,
178,51,249,221,248,101,19,151,36,199,198,196,202,53,18,33,50,150,189,16,18,28,194,219,41,112,119,187,110,109,158,44,243,144,67,240,187,133,5,202,102,101,102,107,181,241,240,243,16,34,59,43,103,161,80,
46,129,178,20,69,237,120,103,167,130,194,15,107,166,167,242,81,109,223,242,182,224,33,225,124,11,181,241,27,242,223,132,28,137,16,187,11,246,104,52,26,193,67,118,255,48,55,230,111,90,188,40,1,108,36,18,
228,230,190,148,99,255,82,202,110,89,79,79,207,3,133,69,49,209,49,48,83,185,189,165,169,233,123,222,255,80,228,4,177,55,83,154,166,15,20,30,92,176,32,84,234,169,220,94,124,252,243,251,62,250,68,169,84,
138,156,51,205,191,169,160,160,160,111,74,142,104,99,181,146,14,230,222,210,211,50,138,139,74,104,154,22,63,141,186,174,107,156,118,45,139,229,217,183,199,74,207,149,255,34,209,108,238,74,161,80,108,125,
107,123,222,250,55,40,222,147,25,33,14,149,157,82,87,87,83,122,252,40,239,123,28,231,142,224,249,33,239,237,220,149,154,146,230,224,249,78,148,245,240,240,120,106,124,90,246,211,233,138,202,243,166,185,
244,29,109,254,126,254,235,215,229,173,125,229,85,47,47,111,199,95,229,92,217,41,6,131,161,188,242,92,109,77,245,192,208,128,179,175,117,47,145,17,81,171,86,173,89,189,114,141,175,202,233,39,132,174,148,
157,194,178,108,251,221,182,198,155,55,110,55,55,117,117,61,180,88,44,174,173,51,219,168,124,84,113,113,218,196,196,164,244,180,140,232,168,104,151,215,113,189,172,45,134,97,244,122,253,208,163,1,147,
201,52,49,241,108,230,11,202,79,165,82,169,213,234,224,249,33,65,65,65,146,44,40,118,69,230,56,133,66,161,209,104,236,221,231,205,77,248,177,11,20,44,11,229,95,104,231,134,224,245,170,36,25,0,0,0,0,73,
69,78,68,174,66,96,130,0,0};

const char* AudioPlayerEditor::stop_png = (const char*) resource_AudioPlayerEditor_stop_png;
const int AudioPlayerEditor::stop_pngSize = 1516;

// JUCER_RESOURCE: folder_png, 2647, "../../../../Icons/folder.png"
static const unsigned char resource_AudioPlayerEditor_folder_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,128,8,2,0,0,0,76,92,246,156,0,0,0,6,116,82,78,83,0,199,0,192,0,201,
70,132,223,14,0,0,10,12,73,68,65,84,120,156,237,157,121,108,20,215,29,199,223,190,153,93,219,216,216,216,198,7,241,149,168,128,124,112,216,248,194,222,93,155,208,54,45,106,104,73,213,86,145,170,182,161,
141,81,21,41,74,162,208,168,85,84,169,168,170,82,154,20,66,211,156,38,37,9,40,52,173,132,154,164,69,42,193,198,242,181,62,2,248,0,26,27,132,177,3,177,57,124,196,248,158,153,183,253,99,214,179,111,103,
119,103,119,231,205,100,176,231,125,254,241,188,217,217,247,126,222,239,252,222,241,123,191,157,181,184,26,218,1,197,56,160,209,6,152,29,42,128,193,80,1,12,134,10,96,48,84,0,131,161,2,24,12,21,192,96,
168,0,6,67,5,48,24,42,128,193,80,1,12,134,10,96,48,84,0,131,97,213,189,109,124,98,188,238,204,233,158,11,61,99,99,163,130,32,68,208,30,107,77,75,77,45,220,92,180,205,185,45,46,110,165,186,214,151,19,150,
72,195,209,130,32,28,126,167,246,248,7,239,115,60,71,210,112,236,138,216,154,159,239,249,225,247,127,68,82,201,50,128,121,252,177,154,136,222,240,251,23,246,157,248,240,4,66,136,176,97,142,227,218,58,
218,230,231,231,75,139,203,8,171,90,210,68,54,6,156,174,255,228,84,221,41,13,155,63,254,193,251,93,61,93,26,86,184,228,136,64,128,201,187,147,7,95,57,160,109,243,200,141,254,116,224,143,132,189,217,146,
38,2,1,94,121,237,208,196,151,19,154,91,48,56,52,248,238,209,119,52,175,118,169,16,174,0,157,103,59,79,254,247,164,78,70,28,59,126,244,218,224,128,78,149,223,227,4,152,5,9,130,208,226,106,110,113,53,95,
189,54,48,61,61,133,144,27,0,48,58,122,103,102,118,70,63,59,146,18,147,82,82,82,9,43,97,24,134,176,134,40,155,45,61,45,189,180,184,172,202,81,29,19,19,67,88,91,56,200,5,104,235,104,251,243,161,151,190,
24,190,241,21,180,125,47,147,16,159,240,248,238,154,93,59,31,129,80,223,181,170,143,0,71,143,191,247,214,225,55,145,155,116,138,185,108,216,190,237,235,191,123,126,31,185,99,41,224,149,247,163,255,124,
244,70,237,235,244,211,199,169,111,168,123,241,224,126,93,155,240,8,48,60,50,124,232,175,7,117,109,105,137,242,241,201,143,93,237,173,250,213,239,17,224,221,99,71,230,230,231,244,107,102,73,243,214,219,
111,234,87,57,4,0,112,60,87,223,80,175,95,27,75,157,254,43,253,3,186,205,146,33,0,160,191,191,111,122,102,90,167,6,150,7,61,189,221,58,213,12,1,0,195,35,195,58,213,190,108,24,25,25,209,169,102,8,0,152,
155,159,215,169,246,101,195,194,130,94,31,17,221,17,51,24,22,0,224,38,14,238,139,48,12,147,157,149,147,158,150,206,178,172,219,237,22,4,129,231,57,94,16,4,158,231,5,65,192,15,4,129,231,57,241,4,47,8,8,
9,226,133,38,92,133,168,220,146,148,81,180,185,104,215,206,71,42,182,86,198,174,136,37,169,7,33,228,209,7,9,60,199,9,130,128,11,198,11,2,18,4,94,144,189,36,138,232,123,37,207,11,72,188,86,224,57,158,227,
185,197,90,5,222,115,27,120,46,231,120,158,231,121,169,217,133,5,78,122,9,33,196,47,222,40,241,9,9,154,124,80,254,144,10,144,147,157,243,236,83,123,139,139,74,52,177,6,66,8,33,180,90,173,154,212,182,36,
96,1,0,170,29,127,199,67,59,246,62,243,92,116,84,180,166,38,153,11,245,30,240,232,15,30,125,242,137,167,130,189,138,16,26,27,31,155,157,157,177,88,32,195,48,12,195,88,173,86,241,128,129,139,127,117,14,
52,46,9,84,10,240,96,245,246,128,159,254,220,252,220,39,167,79,157,105,172,239,238,237,158,155,11,17,219,128,22,200,90,89,134,97,32,100,88,175,66,44,35,253,97,89,233,128,101,24,150,181,50,158,171,89,241,
34,214,106,21,229,100,25,6,50,140,149,245,104,108,129,22,143,234,139,103,240,106,89,214,202,48,144,97,88,171,149,181,217,162,164,183,123,155,144,174,102,216,197,166,244,186,93,88,0,128,219,237,142,232,
61,201,73,201,191,126,246,55,178,147,8,161,15,255,253,175,195,71,106,195,223,182,68,110,180,176,176,16,81,211,6,2,23,93,153,181,178,204,226,61,128,11,6,33,148,28,93,146,141,101,88,177,176,98,69,236,253,
57,247,219,43,236,247,173,201,192,171,85,227,1,53,187,247,196,197,197,225,103,198,39,198,127,187,239,249,243,221,231,137,254,197,123,27,228,70,136,71,28,207,1,130,53,217,95,94,61,244,224,182,237,123,159,
254,85,252,202,120,241,76,196,110,21,191,50,254,219,223,218,129,159,25,30,25,174,121,226,23,203,251,211,215,10,228,70,117,103,78,255,242,201,61,147,119,39,197,51,16,0,16,81,150,85,149,163,202,202,122,
167,137,83,83,119,159,121,238,105,26,77,138,136,193,161,193,151,94,126,81,60,142,216,3,156,246,42,188,184,255,192,254,207,175,15,105,99,151,153,56,211,80,127,253,198,117,16,169,7,68,71,69,151,20,151,74,
197,243,93,231,234,27,234,244,176,111,217,131,220,168,227,211,118,16,169,7,148,108,41,193,151,93,199,254,126,76,99,187,204,196,13,201,3,194,199,233,240,246,63,147,119,39,69,13,41,36,64,16,246,58,0,90,
160,189,194,33,21,93,109,173,228,57,210,102,38,61,125,13,136,200,3,242,243,242,19,87,37,74,197,198,150,70,237,141,50,19,5,249,27,64,68,2,224,253,15,199,115,29,157,180,255,81,79,82,98,82,238,250,92,224,
153,5,133,215,5,57,43,189,2,156,239,58,167,107,170,232,178,199,94,233,16,131,75,225,122,64,86,70,86,78,118,142,84,108,106,105,210,197,46,211,80,237,168,22,15,194,21,192,97,119,226,197,38,58,0,16,16,19,
19,35,109,97,65,16,222,158,112,21,182,0,238,187,220,119,251,206,109,157,140,51,3,101,197,101,54,155,77,60,14,203,3,86,37,172,218,80,176,81,42,182,184,154,117,177,203,52,84,59,183,73,199,97,9,80,89,97,
199,183,35,154,233,0,64,0,195,48,21,91,43,165,162,56,11,10,209,5,57,43,189,3,192,173,219,183,250,46,247,233,100,156,25,216,188,177,80,218,12,0,225,120,64,116,84,116,89,105,185,84,108,110,165,183,63,17,
85,14,159,112,114,104,1,74,138,75,241,0,92,99,51,157,255,16,225,168,244,153,79,138,179,32,165,133,24,30,255,153,158,153,238,162,59,95,4,172,253,218,186,53,233,107,240,51,33,60,0,90,160,189,194,46,21,59,
58,219,205,252,165,106,114,156,190,183,63,8,41,64,65,126,65,114,82,178,84,164,1,56,66,170,157,213,178,51,33,102,65,248,2,88,16,132,182,118,151,78,150,153,129,212,148,180,117,107,215,203,78,134,240,0,60,
0,215,123,177,71,218,203,167,168,192,105,151,247,63,64,89,128,172,204,108,159,0,28,157,255,144,225,223,255,0,229,89,144,163,210,129,23,155,105,4,130,128,248,149,241,133,155,138,252,207,43,121,0,30,128,
187,54,56,32,166,81,80,212,177,181,188,34,224,55,238,131,14,194,9,190,1,56,122,251,19,82,229,155,79,37,17,212,3,236,190,1,56,58,0,144,96,179,217,182,150,85,4,124,41,168,0,248,146,97,124,98,252,210,255,
46,105,111,151,105,216,82,88,28,236,225,55,129,7,97,155,205,134,7,224,90,92,205,38,252,250,156,134,200,2,112,56,129,61,160,212,55,0,215,220,74,7,0,245,64,11,116,248,69,32,188,175,6,60,235,192,214,95,11,
11,11,157,159,118,104,111,151,105,200,203,203,199,195,57,50,2,8,32,11,192,117,158,237,160,15,82,33,65,161,255,1,162,0,178,103,15,203,20,107,162,59,48,100,224,225,28,127,2,120,0,62,255,65,8,181,186,90,
180,55,202,52,100,103,249,132,115,252,9,32,0,30,1,253,172,255,179,209,177,81,237,237,50,13,202,183,63,240,95,9,103,102,100,62,144,243,128,84,164,235,47,66,156,138,3,0,240,247,0,71,133,44,0,71,7,0,245,
36,37,38,21,228,21,40,95,35,95,136,225,138,125,49,124,227,234,192,85,157,140,51,3,82,6,174,2,62,47,39,36,172,218,88,176,73,42,182,208,225,151,12,41,3,87,1,31,1,42,125,67,166,52,3,133,4,60,3,87,1,159,65,
216,129,133,76,167,166,166,186,122,104,6,138,122,240,12,92,5,188,30,96,179,217,202,75,188,1,184,182,14,23,253,10,24,9,120,6,174,2,94,1,138,139,74,240,144,41,93,0,147,32,203,192,85,192,59,11,194,247,236,
57,158,115,209,12,20,2,100,25,184,10,120,60,64,22,50,237,234,238,154,158,158,210,197,52,115,160,28,128,195,241,8,144,155,155,231,19,128,163,25,112,100,40,108,0,200,240,204,130,100,91,198,173,109,116,5,
160,30,255,12,92,5,60,30,128,7,224,174,92,189,66,31,63,67,130,127,6,174,2,16,0,144,113,95,6,30,128,163,95,193,32,36,96,6,92,48,32,0,160,28,219,127,7,116,1,76,70,192,12,92,5,32,0,0,127,140,220,157,209,
59,151,47,247,107,111,151,105,8,152,129,171,128,60,86,71,51,80,8,137,168,255,1,254,2,208,254,135,132,96,25,184,10,248,8,48,59,59,123,174,235,172,166,38,153,139,96,25,184,10,248,8,208,121,182,99,9,61,73,
245,30,36,88,6,174,2,62,2,208,0,28,9,10,25,184,10,120,5,64,8,185,218,116,252,193,172,101,143,66,6,174,2,94,1,46,92,236,29,159,24,215,212,36,115,17,126,0,14,7,123,4,7,205,192,37,64,57,3,87,233,141,210,
81,83,43,157,128,170,71,57,3,87,1,143,0,67,215,135,134,62,167,79,32,86,143,186,254,7,72,2,208,0,28,33,33,83,16,131,177,40,64,11,29,0,212,19,50,3,87,1,8,0,152,248,114,226,194,165,94,77,77,50,23,170,111,
127,32,10,112,241,210,69,217,87,4,40,17,17,50,3,87,1,8,0,184,117,251,166,118,198,152,142,244,180,244,144,25,184,10,64,0,128,213,26,58,129,139,18,140,159,254,248,103,36,63,176,4,1,0,170,7,16,202,134,252,
13,15,239,216,73,82,3,4,0,228,231,170,92,68,152,156,212,148,212,63,236,123,33,210,248,179,12,8,0,96,24,166,102,247,30,141,172,50,11,153,25,153,175,190,252,218,234,228,213,132,245,120,58,175,157,223,249,
238,247,30,222,69,108,149,89,248,198,246,111,190,253,250,223,100,63,201,166,14,139,171,193,251,51,0,255,60,241,143,218,35,181,52,41,49,24,208,2,139,183,148,60,246,147,221,133,155,10,181,170,211,71,0,0,
192,244,204,116,99,115,99,119,111,215,205,155,55,117,122,64,34,180,64,8,45,122,212,76,216,29,7,195,102,179,173,78,78,89,183,118,125,121,105,121,90,106,154,182,149,203,5,160,124,197,208,31,148,53,24,42,
128,193,80,1,12,134,10,96,48,84,0,131,161,2,24,12,21,192,96,168,0,6,67,5,48,24,42,128,193,80,1,12,134,10,96,48,84,0,131,249,63,103,176,183,75,227,82,57,214,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* AudioPlayerEditor::folder_png = (const char*) resource_AudioPlayerEditor_folder_png;
const int AudioPlayerEditor::folder_pngSize = 2647;

// JUCER_RESOURCE: reclightred_png, 1158, "../../../Icons/rec-lightred.png"
static const unsigned char resource_AudioPlayerEditor_reclightred_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,128,8,2,0,0,0,76,92,246,156,0,0,0,6,116,82,78,83,0,255,0,255,0,
255,55,88,27,125,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,4,38,73,68,65,84,120,156,237,157,221,82,211,64,24,134,131,227,25,180,220,129,142,82,81,111,75,103,128,241,8,127,170,180,98,
58,77,145,226,68,29,142,20,100,244,182,116,252,189,6,44,231,120,16,166,83,243,215,205,238,183,251,38,217,247,57,44,52,217,125,159,124,95,187,105,218,172,92,94,94,6,4,199,53,244,0,124,135,2,192,80,0,24,
10,0,115,29,61,0,37,194,238,157,232,213,86,165,167,140,38,103,227,191,223,45,141,71,144,149,154,190,11,138,99,249,109,246,251,242,219,52,166,70,2,6,157,222,52,220,118,176,163,97,116,122,56,251,233,96,
71,42,224,5,244,59,27,113,184,3,217,245,139,232,228,205,236,23,100,215,115,160,2,108,244,25,61,112,221,9,36,160,62,209,47,130,208,224,92,64,61,163,95,196,173,6,135,2,234,31,253,34,174,52,56,17,208,172,
232,23,177,175,193,254,74,184,185,233,7,65,16,199,91,171,55,172,238,193,102,5,52,58,250,20,214,74,193,90,5,180,41,253,192,226,116,228,5,140,186,155,109,75,63,33,142,39,221,187,226,91,149,110,65,173,140,
62,133,104,59,18,173,0,31,210,15,132,167,41,39,192,147,244,19,228,38,43,36,192,171,244,19,132,166,44,33,192,195,244,19,36,38,110,44,192,219,244,19,140,167,111,38,192,243,244,19,204,66,48,16,192,244,231,
24,68,161,43,128,233,167,208,13,68,71,192,168,187,169,183,179,118,163,183,78,214,90,9,243,240,47,162,250,34,185,122,5,48,253,18,170,135,83,81,0,211,95,74,197,136,120,105,162,60,91,171,55,213,255,185,202,
107,0,15,127,117,148,95,12,148,43,128,233,87,66,57,46,182,32,48,106,2,120,248,107,160,22,26,43,0,140,130,0,30,254,218,40,68,199,10,0,179,76,0,15,127,67,150,5,200,10,0,83,42,128,135,191,8,165,49,178,2,
192,20,10,120,188,118,219,229,56,218,205,243,206,70,209,159,138,207,5,177,255,200,82,112,118,136,45,8,76,190,128,126,113,201,16,61,134,157,94,238,227,5,45,136,253,199,6,121,93,136,45,8,12,5,128,201,17,
112,180,126,223,253,56,188,32,175,177,231,8,216,219,127,96,127,44,228,10,182,32,48,20,0,134,2,192,164,5,12,10,214,11,68,132,48,115,89,109,102,33,198,37,152,109,254,95,142,177,5,129,161,0,48,20,0,134,2,
192,80,0,24,10,0,67,1,96,40,0,12,5,128,161,0,48,20,0,134,2,192,80,0,24,10,0,67,1,96,50,31,200,68,167,144,113,120,66,56,57,75,61,146,22,48,173,205,173,37,90,73,148,185,173,13,91,16,24,10,0,67,1,96,114,
4,76,15,62,187,31,135,183,228,8,24,156,127,115,63,14,47,224,229,233,53,132,2,192,228,11,120,22,125,116,60,142,214,83,180,194,205,23,240,118,246,219,230,96,124,164,104,133,203,22,4,166,80,192,238,248,131,
203,113,180,155,126,116,82,244,167,66,1,199,23,127,236,12,198,71,226,226,91,134,178,5,129,41,21,80,203,59,32,55,143,210,24,89,1,96,150,9,96,17,24,178,44,64,86,0,24,5,1,44,2,109,20,162,99,5,128,81,19,192,
34,208,64,45,52,86,0,24,101,1,44,130,74,200,255,124,125,149,141,250,78,149,160,216,130,192,84,20,192,34,88,74,197,136,170,87,0,29,148,224,226,54,86,65,240,50,250,164,241,172,214,51,202,92,247,169,130,
142,128,215,179,31,26,207,106,61,227,204,117,159,42,232,190,8,179,17,165,208,13,196,224,93,16,29,204,49,136,194,236,109,40,29,4,166,33,24,175,3,60,119,96,60,125,137,133,152,183,14,36,38,46,180,18,246,
208,129,208,148,229,78,69,120,229,64,110,178,162,231,130,60,113,32,58,77,173,59,106,151,50,236,244,14,195,109,217,109,214,132,112,114,150,253,150,157,33,242,2,174,104,223,207,95,218,169,111,107,167,163,
91,214,142,172,77,199,90,5,204,105,122,41,88,62,146,236,127,32,211,232,82,176,63,120,251,21,48,167,89,165,224,234,184,113,40,32,161,254,26,220,150,172,115,1,9,245,212,128,232,150,32,1,9,245,209,128,123,
161,130,10,8,130,32,8,118,215,110,29,143,30,65,118,253,100,252,241,253,5,248,235,136,120,1,115,158,118,54,222,133,59,14,118,212,143,78,74,190,51,228,152,26,9,88,100,186,126,111,176,255,80,112,131,71,7,
95,246,206,191,10,110,80,138,154,10,72,49,232,244,166,21,207,47,13,163,211,195,38,252,248,84,51,4,180,24,94,154,8,134,2,192,80,0,24,10,0,243,15,223,166,33,216,179,70,255,92,0,0,0,0,73,69,78,68,174,66,
96,130,0,0};

const char* AudioPlayerEditor::reclightred_png = (const char*) resource_AudioPlayerEditor_reclightred_png;
const int AudioPlayerEditor::reclightred_pngSize = 1158;

// JUCER_RESOURCE: record_png, 1202, "../../../Icons/record.png"
static const unsigned char resource_AudioPlayerEditor_record_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,128,8,2,0,0,0,76,92,246,156,0,0,0,6,116,82,78,83,0,134,0,180,0,185,
32,234,31,71,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,4,82,73,68,65,84,120,156,237,157,75,79,19,81,24,134,91,195,2,195,148,24,141,113,43,32,209,127,194,134,53,20,214,26,5,241,18,35,
209,98,132,200,130,26,141,241,18,239,63,0,203,158,13,255,196,59,198,157,198,96,210,130,176,48,209,197,104,3,211,153,246,204,153,239,156,119,230,156,247,89,66,103,230,156,247,153,239,163,167,204,116,202,
43,235,27,37,130,227,16,122,0,190,67,1,96,40,0,12,5,128,233,67,15,64,137,221,169,139,203,173,143,169,54,89,170,140,246,191,121,106,104,60,130,148,243,249,46,168,54,62,38,190,207,122,46,103,154,163,10,
104,86,103,235,219,159,204,237,191,45,117,33,24,169,52,158,155,59,80,42,240,21,176,85,157,185,191,253,25,114,232,27,193,240,145,198,11,200,161,219,32,5,152,232,51,122,0,187,19,166,5,229,39,250,144,112,
60,16,13,182,5,228,45,250,253,64,52,216,19,144,231,232,247,99,89,131,13,1,69,137,126,63,214,52,24,95,9,23,49,253,54,181,241,177,205,137,179,70,15,97,240,93,80,161,163,143,96,174,20,76,85,128,75,233,151,
76,78,71,94,192,222,212,156,99,233,135,212,198,199,126,79,93,18,223,173,112,11,114,50,250,8,178,237,72,178,2,124,72,191,36,61,77,49,1,158,164,31,34,56,89,25,1,94,165,31,34,53,101,1,1,30,166,31,34,50,241,
172,2,188,77,63,36,251,244,51,9,240,60,253,144,140,33,232,11,96,250,109,178,68,161,41,128,233,71,208,14,68,71,192,222,212,156,222,193,220,70,111,157,172,35,224,78,235,131,198,86,206,115,187,245,94,99,
171,212,2,216,124,186,160,17,78,58,1,76,191,39,105,35,226,165,137,242,108,78,156,83,127,113,10,1,60,253,21,121,253,235,171,250,139,85,5,48,253,84,168,199,197,22,4,70,73,0,79,127,13,20,67,99,5,128,233,
45,128,167,191,54,42,209,177,2,192,244,16,192,211,63,35,61,3,100,5,128,233,38,128,167,191,8,221,99,100,5,128,73,20,240,125,242,130,205,113,184,205,207,234,76,210,175,18,5,60,220,217,52,51,24,31,185,151,
124,19,28,91,16,152,120,1,91,201,37,67,244,104,85,103,99,127,30,47,0,117,223,168,195,172,36,220,2,205,22,4,134,2,192,196,8,40,79,95,177,63,14,31,136,93,145,197,8,184,217,124,107,126,48,228,31,108,65,96,
40,0,12,5,128,137,10,104,38,172,23,136,8,187,29,151,213,70,5,24,253,202,36,178,220,113,89,45,91,16,24,10,0,67,1,96,40,0,12,5,128,161,0,48,20,0,134,2,192,80,0,24,10,0,67,1,96,40,0,12,5,128,161,0,48,20,
0,38,42,160,22,140,64,198,225,9,139,149,209,200,79,162,2,6,115,243,104,9,39,57,220,241,88,27,182,32,48,20,0,134,2,192,196,8,168,15,158,182,63,14,111,137,171,128,213,39,214,135,225,5,177,95,58,205,22,4,
134,2,192,196,11,184,30,12,89,30,135,243,36,173,112,227,5,28,107,188,52,57,24,31,73,90,225,178,5,129,73,20,112,117,224,164,197,97,56,206,124,48,156,244,171,68,1,39,214,94,153,25,140,143,28,77,126,100,
40,91,16,152,110,2,242,249,4,228,194,209,61,70,86,0,152,30,2,88,4,25,233,25,32,43,0,76,111,1,44,2,109,84,162,99,5,128,81,18,192,34,208,64,49,52,86,0,24,85,1,44,130,84,168,199,149,162,2,232,64,145,84,65,
177,5,129,73,39,128,69,208,147,180,17,165,174,0,58,232,130,70,56,58,45,232,86,112,74,99,43,231,89,234,184,238,83,5,29,1,65,227,153,198,86,206,211,223,113,221,167,10,154,127,132,217,136,34,104,7,162,255,
46,136,14,218,100,137,34,211,219,80,58,40,101,14,33,235,58,192,115,7,217,167,47,176,16,243,214,129,200,196,101,86,194,30,58,144,154,178,216,71,17,94,57,16,156,172,228,103,65,158,56,144,157,166,240,135,
113,245,245,141,5,119,239,179,92,172,140,138,159,100,229,21,51,167,173,123,79,96,50,84,223,166,62,142,118,172,29,153,155,78,159,161,253,150,254,15,186,232,165,96,250,76,50,254,15,153,66,151,130,133,193,
27,172,128,54,69,44,5,107,231,141,13,1,33,69,209,96,185,100,237,9,8,201,179,6,72,183,180,45,32,36,111,26,128,127,168,48,2,66,194,105,127,155,60,255,104,231,11,100,0,215,6,134,142,175,129,111,71,52,181,
16,211,224,71,117,230,129,149,7,200,205,7,195,93,238,25,178,76,142,4,28,96,250,114,173,249,78,112,127,119,7,207,252,89,125,44,184,67,41,242,42,224,32,205,234,108,218,71,123,44,4,35,149,34,124,249,84,49,
4,56,12,47,77,4,67,1,96,40,0,12,5,128,249,11,212,5,46,243,89,197,239,153,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* AudioPlayerEditor::record_png = (const char*) resource_AudioPlayerEditor_record_png;
const int AudioPlayerEditor::record_pngSize = 1202;


//[EndFile] You can add extra defines here...
//[/EndFile]
