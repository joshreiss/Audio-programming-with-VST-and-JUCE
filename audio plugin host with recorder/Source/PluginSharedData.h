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
#ifndef PLUGINSHAREDDATA_H_INCLUDED
#define PLUGINSHAREDDATA_H_INCLUDED

/**
    A class to exchange data between the two plugins AudioPlayerPlugin and 
    AudioSlaveRecorderPlugin via a SharedResourcePointer.
    Used to let the player always know if it has a recorder or not.
    
    Notify every player&recorder creation and deletion here!
*/
class AudioPlayerPluginSharedData {
    
protected:
    friend class AudioPlayerPlugin;
    friend class AudioSlaveRecorderPlugin;

    void setRecorder(AudioSlaveRecorderPlugin* );
    void recorderKilled();
    bool hasRecorder();
    void setPlayer(AudioPlayerPlugin*);
    void playerKilled();
    bool hasPlayer();

private:
    AudioPlayerPlugin* player = nullptr;
    AudioSlaveRecorderPlugin* recorder = nullptr;

    void notifyPlayer();
};

#endif  // PLUGINSHAREDDATA_H_INCLUDED
