#include <iostream>
#include <fstream>
#include <conio.h>
#include <cmath>
#include <vector>
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
WaveWriter ww("Audio/Midi_Player_Test.wav", sampleRate);

const int OUTPUT = 0; // we're not using this anyway
const char HIGH = 0xC0;
const char LOW = 0x40;

vector<char> toArduinoBuffer;
vector<char> fromArduinoBuffer;

char state = 0; // the state of the "Piezo Element"

char * sequenceBytes;
int byteSize = 0;
int byteIndex = 0;

void readFile(const char * location)
{
    ifstream file (location, ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        // Read File

        byteSize = file.tellg();
        sequenceBytes = new char[byteSize];
        file.seekg (0, ios::beg);
        file.read (sequenceBytes, byteSize);
        file.close();
        cout << "Successfully read " << location << ": " << byteSize << " bytes" << endl;
    }
    else
    {
        cout << "Error: could not read " << location << endl;
    }
}

void startSending()
{
    string location;
    cout << "Path for simplified sequence data: ";
    cin >> location;
    readFile(location.c_str());
}

int Serialavailable()
{
    return toArduinoBuffer.size();
}

void Serialbegin(int baud)
{
}

void Serialwrite(char datum)
{
    if (byteIndex < byteSize)
    {
        toArduinoBuffer.insert(toArduinoBuffer.begin(), sequenceBytes[byteIndex]);
        byteIndex++;
    }
}

char Serialread()
{
    char datum = toArduinoBuffer[toArduinoBuffer.size() - 1];
    toArduinoBuffer.pop_back();
    return datum;
}

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

void delayMicroseconds (int duration)
{
	int samples = int(duration / 1000000.0 * sampleRate + 0.5);
	for (int i = 0; i < samples; i++)
		ww.append(state);
}

void delay (int duration)
{
	int samples = int(duration / 1000.0 * sampleRate + 0.5);
	for (int i = 0; i < samples; i++)
		ww.append(state);
}

long micros()
{
    return (long)((ww.size() - 44) * 1000000.0 /sampleRate);
}

long millis()
{
    return (long)((ww.size() - 44) * 1000.0 /sampleRate);
}

//---------------------------------------------------------------------
//---------------------------- IGNORE ---------------------------------
//---------------------------------------------------------------------
void setup();
void loop();

int main()
{
    startSending();

    WaveWriter temp("Audio/Midi_Player_Test.wav", sampleRate);
    ww = temp;
    setup();
    while (byteIndex < byteSize)
        loop();
        
    ww.write();

    cout << "Press any key to continue...";
    getch();
    return 0;
}


//------------------------------------------------------------
//--------------------- Arduino Code -------------------------
//------------------------------------------------------------

/**
 * @author Jiayin Huang
 * @author Tony Liao
 *
 * MIDI # 		Note Name		Frequency		Period (ms)
 * 60				C		261.63			3.822
 * 61				C#		277.18			3.608
 * 62				D		293.67			3.405
 * 63				D#		311.13			3.214
 * 64				E		329.63			3.304
 * 65				F		349.23			2.863
 * 66				F#		369.99			2.703
 * 67				G		392.00			2.551
 * 68				G#		415.30			2.408
 * 69				A		440.00			2.273
 * 70				A#		466.16			2.145
 * 71				B		493.88			2.025
 * 72				C		523.25			1.910
 *
 * From http://www.phys.unsw.edu.au/jw/notes.html :
 *
 * A4 = 440 Hz
 * n = # of semitones away from A4
 * fn = 2^(n / 12) * 440
 * fmidi = 2^((m - 69)/ 12) * 440
 * high period (microseconds) = 0.5 * 2^(-(m - 69)/12.0) / 440 * 1000000
 */

// Constants

const int piezo1 = 12;
const int piezo2 = 13;

// Serial

const char next = 'N';

bool receiving = true;
bool processing = false;
int baud = 9600;

char buffer[1024];
const int bufferSize = 1024;

int bufferIndex = 0;
int caretIndex = 8;

int bufferCyclesAhead = 0;
int minDiff = bufferSize / 4;        // 256 : min value for how far ahead bufferIndex is
int maxDiff = bufferSize - minDiff;    // 768 : max value for how far ahead bufferIndex is

// Vars

bool playPushed = false;
bool playing = false;

int toneDuration[128];
int offset = 0;

int timeDivision = 16;
int microsecondsPerInterval = 31250;

/** Converts 4 consecutive bytes (big endian) into an int.
 */
int getInt(int startIndex)
{
    int value = 0;
    for (int i = 0; i < 4; i++)
    {
        value <<= 8;
        value |= (int)buffer[startIndex + i] & 0xFF;
    }
    return value;
}

void playNote(int note, int microseconds)
{
    int numPeriods = microseconds / toneDuration[note];
    int timeHigh = toneDuration[note] / 2 + offset;
    for (int i = 0; i < numPeriods; i++)
    {
        digitalWrite(piezo1, HIGH);
        digitalWrite(piezo2, HIGH);
        delayMicroseconds(timeHigh);
        digitalWrite(piezo1, LOW);
        digitalWrite(piezo2, LOW);
        delayMicroseconds(timeHigh);
    }
}

void playPiezos()
{
    if ((buffer[caretIndex] & 0xFF) != 0xFF)
    {
        playNote(buffer[caretIndex], microsecondsPerInterval);
    }
    else
    {
        delay(microsecondsPerInterval / 1000);
    }
    caretIndex++;
    if (caretIndex == bufferSize)
    {
        caretIndex = 0;
        bufferCyclesAhead--;
    }
}

void read()
{
    // Only read data if buffer is less than maxDiff ahead of the caret

    if ((bufferCyclesAhead * bufferSize + bufferIndex) - (caretIndex) < maxDiff)
    {
        if (Serialavailable() > 0)
        {
            // Read Byte

            buffer[bufferIndex] = (char)Serialread();
            bufferIndex++;

            // Increment buffer index

            if (bufferIndex == bufferSize)
            {
                bufferIndex = 0;
                bufferCyclesAhead++;
            }

            Serialwrite(next); // Request next byte
        }
    }
}

void setup()
{
    // Begin serial

    Serialbegin(baud);

    for (int i = 0; i < 9; i++)
        Serialwrite(next);

    // Calculate Tone Durations

    for (int i = 0; i < 128; i++)
    {
        toneDuration[i] = (int)(pow(2, -(i - 69)/ 12.0) / 440 * 1000000);
        cout << i << " : " << toneDuration[i] << endl;
    }
    getch();

    pinMode(piezo1, OUTPUT);
    pinMode(piezo2, OUTPUT);

    while (Serialavailable() < 8)
    {
        delay(1); // Wait for initializing time division and temp data
    }
    
    for (int i = 0; i < 8; i++)
    {
        buffer[bufferIndex] = (char)Serialread();
        bufferIndex++;
    }

    timeDivision = getInt(0);
    microsecondsPerInterval = getInt(4);
    
    cout << "Time division : " << timeDivision << endl;
    cout << "Microseconds per interval : " << microsecondsPerInterval << endl;
    cout << "Microseconds per quarter note : " << (microsecondsPerInterval * timeDivision / 4) << endl;
}

void loop()
{
    read();
    playPiezos();
}
