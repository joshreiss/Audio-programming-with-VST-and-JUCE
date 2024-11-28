#pragma once
class MainContentComponent   : public juce::AudioAppComponent, private juce::ChangeListener
{
public:
    MainContentComponent()
       : state (Stopped),
         thumbnailCache (5),                            // [4]
         thumbnail (512, formatManager, thumbnailCache) // [5]
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
        setSize (600, 400);
        formatManager.registerBasicFormats();
        transportSource.addChangeListener (this);
        thumbnail.addChangeListener (this);            // [6]
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
        if (readerSource.get() == nullptr) bufferToFill.clearActiveBufferRegion();
        else transportSource.getNextAudioBlock (bufferToFill);
    }
    void releaseResources() override
    {
        transportSource.releaseResources();
    }
    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<int> thumbnailBounds (10, 100, getWidth() - 20, getHeight() - 120);
        if (thumbnail.getNumChannels() == 0) paintIfNoFileLoaded (g, thumbnailBounds);
        else paintIfFileLoaded (g, thumbnailBounds);
    }
    void resized() override
    {
        openButton.setBounds (10, 10, getWidth() - 20, 20);
        playButton.setBounds (10, 40, getWidth() - 20, 20);
        stopButton.setBounds (10, 70, getWidth() - 20, 20);
    }
    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {
        if (source == &transportSource) transportSourceChanged();
        if (source == &thumbnail)       thumbnailChanged();
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
        changeState (transportSource.isPlaying() ? Playing : Stopped);
    }
    void thumbnailChanged()
    {
        repaint();
    }
    void paintIfNoFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
    {
        g.setColour (juce::Colours::darkgrey);
        g.fillRect (thumbnailBounds);
        g.setColour (juce::Colours::white);
        g.drawFittedText ("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
    }
    void paintIfFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
    {
        g.setColour (juce::Colours::white);
        g.fillRect (thumbnailBounds);
        g.setColour (juce::Colours::red);                               // [8]
        thumbnail.drawChannels (g,                                      // [9]
                                thumbnailBounds,
                                0.0,                                    // start time
                                thumbnail.getTotalLength(),             // end time
                                1.0f);                                  // vertical zoom
    }
    void openButtonClicked()
    {
        chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...", juce::File{}, "*.wav");
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
                    thumbnail.setSource (new juce::FileInputSource (file));                            // [7]
                    readerSource.reset (newSource.release());
                }
            }
        });
    }
    void playButtonClicked() { changeState (Starting); }
    void stopButtonClicked() { changeState (Stopping); }
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;                    // [3]
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;
    juce::AudioThumbnailCache thumbnailCache;                  // [1]
    juce::AudioThumbnail thumbnail;                            // [2]
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
