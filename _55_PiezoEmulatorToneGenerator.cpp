#include <iostream>
#include <fstream>
#include <conio.h>
#include <cmath>
#include "WaveManipulationClasses/WaveWriter.h"

/** Outputs audio to file. Implements a more precise timing mechanism.
 */

/**
 * MIDI # 		Note Name		Frequency		Period (ms)
 * 60				C			261.63			3.822
 * 61				C#			277.18			3.608
 * 62				D			293.67			3.405
 * 63				D#			311.13			3.214
 * 64				E			329.63			3.304
 * 65				F			349.23			2.863
 * 66				F#			369.99			2.703
 * 67				G			392.00			2.551
 * 68				G#			415.30			2.408
 * 69				A			440.00			2.273
 * 70				A#			466.16			2.145
 * 71				B			493.88			2.025
 * 72				C			523.25			1.910
 *
 * From http://www.phys.unsw.edu.au/jw/notes.html :
 *
 * A4 = 440 Hz
 * n = # of semitones away from A4
 * fn = 2^(n / 12) * 440
 * fmidi = 2^((m - 69)/ 12) * 440
 * high period (microseconds) = 0.5 * 2^(-(m - 69)/12.0) / 440 * 1000000
 */

//-----------------------------------------------------------
//------ whatever substitutes for Arduino ignore these ------
//-----------------------------------------------------------

int sampleRate = 44100;
WaveWriter ww("Audio/CODE06_Tone.wav", sampleRate);

const int HIGH = 192;
const int LOW = 64;
const int OUTPUT = 0; // we're not using this anyway

char state = 0; // the state of the "Piezo Element"

void pinMode(int pin, int mode)
{
	// Do nothing
}

void digitalWrite(int pin, int value)
{
	state = (char)value;
}

void analogWrite(int pin, int value)
{
	state = (char)value;
}

long micros()
{
    return (long)((ww.size() - 44) * 1000000.0 /sampleRate);
}

long millis()
{
    return (long)((ww.size() - 44) * 1000.0 /sampleRate);
}

void delayMicroseconds (int duration)
{
	int samples = int(duration / 1000000.0 * sampleRate + 0.5);
	for (int i = 0; i < samples; i++)
	{
        ww.append(state);
    }
}

void delay (int duration)
{
	int samples = int(duration / 1000.0 * sampleRate + 0.5);
	for (int i = 0; i < samples; i++)
	{
		ww.append(state);
    }
}

//---------------------------------------------------------------------
//---------------------------- IGNORE ---------------------------------
//---------------------------------------------------------------------
void setup();
void loop();

int main()
{
    setup();
    loop();
    ww.write();

    cout << "Press any key to continue...";
    getch();
    return 0;
}


//------------------------------------------------------------
//--------------------- Arduino Code -------------------------
//------------------------------------------------------------

const int amplitude = 64;
int speakerPin = 9;

int length = 1; // the number of notes
int notes[] = {60};
int beats[] = {20};

int tempo = 125;

int toneDuration[108]; // Now the duration of an entire wave cycle
int waveType = 0;
int lookupResolution = 512;
char * lookUpTable;

void playTone(int tone, int duration)
{
    long localStart = micros();
    long maxSamples = sampleRate * duration / 1000L;

    for (long i = 0; i < maxSamples; i++)
    {
        // Delay Necessary
        long expectedTime = (long)(i * 1000000.0 / sampleRate + localStart + 0.5);
        long dt = expectedTime - micros();
        if (dt > 0)
            delayMicroseconds(dt);

        // Determine position in waveform

        float percent = (1000000.0 * i / (tone * sampleRate));
        long elapsedCycles = (long)percent;
        percent -= elapsedCycles;
        int index = (int)(percent * lookupResolution);

        // Write sample

        analogWrite(speakerPin, lookUpTable[index]);
    }
}

void playNote(int note, int duration)
{
    if (note != -1)
    {
        playTone(toneDuration[note], duration);
    }
    else
    {
        delay(duration);
    }
}

void setup()
{
	pinMode(speakerPin, OUTPUT);

	// Calculate Tone Durations

	for (int i = 20; i < 108; i++)
        toneDuration[i] = (int)(pow(2, -(i - 69)/ 12.0) / 440 * 1000000);

    // Init Lookup Table

    lookUpTable = new char[lookupResolution];

    if (waveType == 0) // square
    {
        for (int i = 0; i < lookupResolution; i++)
        {
            if (i < lookupResolution / 2)
                lookUpTable[i] = 128 + amplitude;
            else
                lookUpTable[i] = 128 - amplitude;
        }
    }
    else if (waveType == 1) // sine
    {
        for (int i = 0; i < lookupResolution; i++)
        {
            lookUpTable[i] = (char)(amplitude * sin(2 * M_PI / lookupResolution * i) + 128);
        }
    }
    else if (waveType == 2) // sawtooth
    {
        for (int i = 0; i < lookupResolution; i++)
        {
            lookUpTable[i] = (char)((i - lookupResolution/2) * amplitude * 2 / lookupResolution + 128);
        }
    }
}

void loop()
{
	for (int i = 0; i < length; i++)
    {
        playNote(notes[i], beats[i] * tempo);

        // pause between notes
        delay(tempo / 2);
    }
}
