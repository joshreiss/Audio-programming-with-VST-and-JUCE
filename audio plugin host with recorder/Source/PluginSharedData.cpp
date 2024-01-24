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
#include "PluginSharedData.h"
#include "AudioPlayerPlugin.h"
#include "AudioSlaveRecorderPlugin.h"

/**
Call this if the recorder is created.
*/
void AudioPlayerPluginSharedData::setRecorder(AudioSlaveRecorderPlugin* rec) {
    recorder = rec;
    notifyPlayer();
}

/**
Call this if the recorder is deleted.
*/
void AudioPlayerPluginSharedData::recorderKilled() {
    setRecorder(nullptr);
}

/**
Check if a recorder is registered
@returns true if there is a recorder
*/
bool AudioPlayerPluginSharedData::hasRecorder() {
    return recorder != nullptr;
}

/**
Call this if the player is created.
*/
void AudioPlayerPluginSharedData::setPlayer(AudioPlayerPlugin* pl) {
    // new player, but possibly existing recorder?
    if (pl != nullptr) {
        player = pl;
        // notify, if recorder exists
        notifyPlayer();
    }
    else
        player = nullptr;
}

/**
Call this if the player is deleted.
*/
void AudioPlayerPluginSharedData::playerKilled() {
    setPlayer(nullptr);
    if (hasRecorder())
        setRecorder(nullptr);
}

/**
Check if a player is registered
@returns true if there is a player
*/
bool AudioPlayerPluginSharedData::hasPlayer() {
    return player != nullptr;
}

void AudioPlayerPluginSharedData::notifyPlayer() {
    if (player != nullptr) {
        if (recorder != nullptr)
            player->setRecorder(recorder);
        else;
            //player->recorderKilled();
    }
}
