SynthTest
=========

Experimentation with audio synthesis.


======= C++: ========
=====================
1.0. ToneGenerator.dev:    
        Generates a wave file in the path "Audio/Tone.wav", consisting entirely of a single tone.
        Params:
            Sample rate:
            Duration (ms):              for the entire wave file
            Frequency (Hz):
            Wave type:                  0 = square, 1 = sine, 2 = sawtooth, other = random noise
            Lookup table resolution:    preferably a power of 2
            Amplitude:                  0 to 127

2.0. WaveDegrader.dev: 
        Reads a wave file, converts to mono channel, and outputs to "Audio/DegradedMono.wav".
        Params:
            File path:                  for the input file
            Target sample rate:         

3.0. PiezoEmulatorExample.dev: (ARDUINO)
        Basically does the same thing as the CODE06 Arduino example (piezo elements),
        except this program outputs audio that would have been generated to "Audio/CODE06_Audio.wav".

4.0. PiezoEmulatorMoreNotes.dev: (ARDUINO)
        Same as [3], except replaces the "ccggaagffeeddc " etc. notes interpretation with midi note
        values, e.g. {60, 60, 67, 67, 69, 69, 67, 65, 65, 64, 64, 62, 62, 60, -1}.

5.0. PiezoEmulatorWaveForms.dev: (ARDUINO)
        Same as [4], except uses different possible wave forms, uses a waveform lookup table,
        and sets the value of the waveform state every sample instead of every time it changes;
        unlike in [4] and [3].

5.5. PiezoEmulatorToneGenerator.dev: (ARDUINO)
        Identical to [5] except that it only plays one note for an extended period of time.

6.0 PiezoEmulatorConcurrentTone.dev: (ARDUINO)
        Uses MIDI-like encoding with notes, delta time, and velocity to allow multiple notes
        to be played simultaneously. It has 3 channels. It plays a C, E, and G simultaneously.

6.5. PiezoEmulatorConcurrentNotes.dev: (ARDUINO)
        Identical to [6] except that it plays a sequence of notes instead of just a chord.
        


======= Arduino: =======
========================

1. [^3.0] CODE_06_Piezo_Elements.ino

2. [^4.0] More_Notes.ino

3. [----] Low_Pass_Filter_Tester.ino: Applies a constant 128 pulse width modulation so that a low-pass
         filter can be tested, to convert the PWM to an analog voltage (0-5V). Silence = successful.

4. [^5.0] Wave_Forms.ino

5. [^6.0] Concurrent_Tone.ino


