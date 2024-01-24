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
#ifndef TRANSPORTSTATE_H_INCLUDED
#define TRANSPORTSTATE_H_INCLUDED

/**
    Transport state enum for Audio players, recorders and similar
*/
enum TransportState : int
{
    // order matters, operators like > are used!
    NoFile,
    Unloading,
    Stopped,
    Stopping,
    Paused,
    Pausing,
    Starting,
    Playing,
    Recording
};
#endif  // TRANSPORTSTATE_H_INCLUDED
