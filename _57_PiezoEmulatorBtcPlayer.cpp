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

int sampleRate = 3906;
int sampleResolution = 1000000 / sampleRate;
WaveWriter ww("Audio/CODE06_BTC.wav", sampleRate);

const int HIGH = 192;
const int LOW = 64;
const int OUTPUT = 0; // we're not using this anyway

char * buffer;
int size;

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

char * read(string filePath)
{
    const char * path = filePath.c_str();
    ifstream file (path, ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        // Read File

        size = file.tellg();
        buffer = new char [size];
        file.seekg (0, ios::beg);
        file.read (buffer, size);
        file.close();
        cout << "Successfully read " << path << ": " << size << " bytes" << endl;
    }
    else
    {
        cout << "Error: could not read " << path << endl;
    }
    return buffer;
}

int main()
{
    string path;
    cout << "btc path: ";
    cin >> path;
    read(path);
    
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

int speakerPin = 9;

unsigned long localStart = micros();
unsigned long localEnd = micros();

bool processing = false;
int baud = 28800;

int bufferIndex = 0;
int caretIndex = 0;
int caretSubIndex = 0;

char masks[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

int previousUpdate = micros();

bool initializing = true;

/** Updates the state of the piezo element.
 */
void updateState()
{
    previousUpdate = micros();

    bool on = (buffer[caretIndex] & masks[caretSubIndex]) == masks[caretSubIndex];

    if (on)
        digitalWrite(speakerPin, HIGH);
    else
        digitalWrite(speakerPin, LOW);

    caretSubIndex++;

    if (caretSubIndex == 8)
    {
        caretSubIndex = 0;
        caretIndex++;
    }
}

/** Do stuff with the data.
 */
void process()
{
    updateState();
}

void setup()
{
    pinMode(speakerPin, OUTPUT);
    //setPwmFrequency(speakerPin, 1); // 31250 Hz divide 8 = 3906.25 Hz
}

void loop()
{
    while (caretIndex < size)
    {
        process();
        ww.append(state);
    }
}
