/*
==============================================================================

This file is part of the JUCE library.
Copyright (c) 2015 - ROLI Ltd.

Permission is granted to use this software under the terms of either:
a) the GPL v2 (or any later version)
b) the Affero GPL v3

Details of these licenses can be found at: www.gnu.org/licenses

JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

------------------------------------------------------------------------------

To release a closed-source product which uses JUCE, commercial licenses are
available: visit www.juce.com for more information.

==============================================================================
*/

#ifndef THUMBNAILCOMP_H_INCLUDED
#define THUMBNAILCOMP_H_INCLUDED

#include "JuceHeader.h"

/**
    Audio Thumbnail class for the AudioPlayerEditor.
    Derived from the JUCE Demo, DemoThumbnailComp
    @see AudioPlayerEditor
*/
class AudioPlayerEditor::ThumbnailComp : public Component,
    public ChangeListener,
    public ChangeBroadcaster,
    private ScrollBar::Listener,
    private Timer
{
public:
    /**
        Create an audio thumbnail coupled to an AudioPlayerEditor
        @param formatManager the audio format manager that is used to open the file (for AudioThumbnail)
        @param editor_ the audio player that uses this thumbnail - for status and position exchange
        @param slider slider used for zooming - will be updated when zooming on the thumbnail via mouse wheel
    */
    ThumbnailComp(AudioFormatManager& formatManager,
        AudioPlayerEditor& editor_,
        Slider& slider)
        : editor(editor_),
        zoomSlider(slider),
        scrollbar(false),
        thumbnailCache(5),
        thumbnail(512, formatManager, thumbnailCache)
    {
        thumbnail.addChangeListener(this);

        addAndMakeVisible(scrollbar);
        scrollbar.setRangeLimits(visibleRange);
        scrollbar.setAutoHide(false);
        scrollbar.addListener(this);

        currentPositionMarker.setFill(Colours::white.withAlpha(0.85f));
        addAndMakeVisible(currentPositionMarker);
    }

    ~ThumbnailComp()
    {
        stopTimer();
        while (isTimerRunning());

        scrollbar.removeListener(this);
        thumbnail.removeChangeListener(this);
    }

    /**
        Set a file for this thumbnail and load it, set range accordingly.
        @param filepath filename and path
    */
    void setFile(const String filepath)
    {
        thumbnail.setSource(nullptr);
        thumbnailCache.clear();
        File file(filepath);
        if (!file.isDirectory())
        {
            thumbnail.setSource(new FileInputSource(file));
            // taking length from editor scales to longest of all files
            const Range<double> newRange(0.0, editor.getLengthInSeconds());
            scrollbar.setRangeLimits(newRange);
            setRange(newRange);

            startTimerHz(20);
        }
    }

    /**
        Set the zoom factor
        @param amount from 0 to see entire waveform up to 0.99 for max zoom in
    */
    void setZoomFactor(double amount)
    {
        // taking length from editor scales to longest of all files
        if (editor.getLengthInSeconds() > 0)
        {
            // calculate zoom factor
            double newScale = jmax(0.001, editor.getLengthInSeconds() * (1.0 - jlimit(0.0, 0.99, amount)));

            // center current position
            double timeAtCentre = editor.getCurrentPosition();

            // out of range position is fixed in set range
            setRange(Range<double>(timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5));
        }
    }

    /**
        Set new range for the thumbnail
        @param newRange double range in seconds
    */
    void setRange(Range<double> newRange)
    {
        // move position min. to 0
        if (newRange.getStart() < 0)
            newRange = newRange.movedToStartAt(0);
        // move position max. to end of longest file
        if (newRange.getEnd() > editor.getLengthInSeconds())
            newRange = newRange.movedToEndAt(editor.getLengthInSeconds());

        visibleRange = newRange;
        scrollbar.setCurrentRange(visibleRange, dontSendNotification);
        updateCursorPosition();
        repaint();
    }

    void paint(Graphics& g) override
    {
        g.fillAll(Colours::darkgrey);

        // draw only if thumbnail data filled and current position is in range of longest file
        if (editor.getLengthInSeconds() > 0.0 && visibleRange.getStart() < editor.getLengthInSeconds())
        {
            g.setColour(Colours::lightblue);
            Rectangle<int> thumbArea(getLocalBounds());
            thumbArea.removeFromBottom(scrollbar.getHeight() + 4);
            thumbnail.drawChannels(g, thumbArea.reduced(2), visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
        }
    }

    void resized() override
    {
        scrollbar.setBounds(getLocalBounds().removeFromBottom(14).reduced(2));
    }

    void changeListenerCallback(ChangeBroadcaster*) override
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

    void mouseDown(const MouseEvent& e) override
    {
        mouseDrag(e);
    }

    void mouseDrag(const MouseEvent& e) override
    {
        // do not move while recording
        if (!editor.isRecording()) {
            // move, but min. 0 (max to the end of the file is handled in processor)
            editor.setPosition(jmax(0.0, xToTime((float)e.x)));
            updateCursorPosition();
        }
    }

    void mouseUp(const MouseEvent&) override
    {
    }

    void mouseWheelMove(const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        // taking length from editor scales to longest of all files
        if (editor.getLengthInSeconds() > 0.0)
        {
            // horizontal scroll via mouse wheel
            double newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 20.0;
            newStart = jlimit(0.0, jmax(0.0, editor.getLengthInSeconds() - (visibleRange.getLength())), newStart);
            setRange(Range<double>(newStart, newStart + visibleRange.getLength()));

            // zoom via mouse wheel - update editor's zoom slider which in turn zooms this component
            if (wheel.deltaY != 0.0f)
                zoomSlider.setValue(zoomSlider.getValue() + wheel.deltaY / 10);

            repaint();
        }
    }


private:
    AudioPlayerEditor& editor;
    Slider& zoomSlider;
    ScrollBar scrollbar;

    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    Range<double> visibleRange;
    File lastFileDropped;

    // some vars to cache internal state
    bool wasPlaying = false;
    double previousPos = -1;

    DrawableRectangle currentPositionMarker;

    /**
        get x position from time in seconds
        @param time time in seconds
        @returns position
    */
    float timeToX(const double time) const
    {
        return getWidth() * (float)((time - visibleRange.getStart()) / (visibleRange.getLength()));
    }

    /**
        get time in seconds from x position
        @param x position
        @returns time in seconds
    */
    double xToTime(const float x) const
    {
        return (x / getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
    }

    /**
        Follow scrollbar
    */
    void scrollBarMoved(ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
    {
        if (scrollBarThatHasMoved == &scrollbar)
            setRange(visibleRange.movedToStartAt(newRangeStart));
    }

    /**
        Update cursor and follow player
    */
    void timerCallback() override
    {
        double pos = editor.getCurrentPosition();

        // position changed?
        if (pos != previousPos) {
            // follow player: update thumbnail scroll position to visible area if outside (page flip)
            if (editor.getCurrentPosition() > visibleRange.getEnd() || editor.getCurrentPosition() < visibleRange.getStart()) {
                setRange(visibleRange.movedToStartAt(editor.getCurrentPosition()));
            }
            updateCursorPosition();

            previousPos = pos;
        }
    }

    /**
        Update cursor position if in range and show cursor.
    */
    void updateCursorPosition()
    {
        bool validPos = editor.getCurrentPosition() <= editor.getLengthInSeconds();

        currentPositionMarker.setVisible(validPos);
        if (validPos) {
            currentPositionMarker.setRectangle(Rectangle<float>(timeToX(editor.getCurrentPosition()) - 0.75f, 0,
                1.5f, (float)(getHeight() - scrollbar.getHeight())));
        }
    }

};

#endif  // THUMBNAILCOMP_H_INCLUDED
