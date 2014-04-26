SynthTest
=========

Experimentation with audio synthesis.

1. ToneGenerator.dev: Generates a wave file in the path "Audio/Tone.wav", consisting entirely of a single tone.
        Params:
            Sample rate:
            Duration (ms):              for the entire wave file
            Frequency (Hz):
            Wave type:                  0 = square, 1 = sine, 2 = sawtooth, other = random noise
            Lookup table resolution:    preferably a power of 2
            Amplitude:                  0 to 127

2. WaveDegrader.dev: Reads a wave file, converts to mono channel, and outputs to "Audio/DegradedMono.wav".
        Params:
            File path:                  for the input file
            Target sample rate:         
            
3. PiezoEmulatorExample.dev: Basically does the same thing as the CODE06 Arduino example (piezo elements),
                             except this program outputs audio that would have been generated to "Audio/CODE06_Audio.wav".
                            
4. PiezoEmulatorMoreNotes.dev: Same as 3, except replaces the "ccggaagffeeddc " etc. notes interpretation with midi note
                               values, e.g. {60, 60, 67, 67, 69, 69, 67, 65, 65, 64, 64, 62, 62, 60, -1}.
                             
5. PiezoEmulatorWaveForms.dev: Same as 4, except uses different possible wave forms, uses a waveform lookup table,
                               and sets the value of the waveform state every sample instead of every time it changes;
                               unlike in 4 and 3.
                              


