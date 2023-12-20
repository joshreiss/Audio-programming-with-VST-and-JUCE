# Summary
This repository is a collection of audio effects plugins implemented from the explanations in the book *"Audio Effects: Theory, Implementation and Application"* by Joshua D. Reiss and Andrew P. McPherson, and takes as example the code provided with the book which has contributions and implementations by Brecht De Man, and others.

The audio effects implemented are:

- [**Template Frequency Domain**](Template%20Frequency%20Domain) implements a short-time Fourier transform class. This plugin does not apply any processing to the input, it just converts the input block to the frequency domain, and back to the time domain using the overlap-add method. This plugin is used as a template project for frequency domain audio processing effects.
![Template Frequency Domain](Screenshots/Template%20Frequency%20Domain.png)

- [**Robotization/Whisperization**](Robotization-Whisperization) implements two audio effects based on the phase vocoder algorithm. This plugin is meant to be used with speech sounds. Robotization applies a constant pitch to the signal while preserving the formants, the result sounds like a robotic voice. Whisperization eliminates any sense of pitch while preserving the formants, the result should sound like someone whispering.
![Robotization/Whisperization](Screenshots/Robotization-Whisperization.png)

- [**Pitch Shift**](Pitch%20Shift) changes the pitch of the input signal without changing the duration using the phase vocoder algorithm. It is a real-time implementation that allows continuous and smooth changes of the pitch shift parameter.
![Pitch Shift](Screenshots/Pitch%20Shift.png)
