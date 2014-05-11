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

boolean receiving = true;
boolean processing = false;
int baud = 9600;

byte buffer[1024];
const int bufferSize = 1024;

int bufferIndex = 0;
int caretIndex = 8;

int bufferCyclesAhead = 0;
int minDiff = bufferSize / 4;        // 256 : min value for how far ahead bufferIndex is
int maxDiff = bufferSize - minDiff;    // 768 : max value for how far ahead bufferIndex is

// Vars

boolean playPushed = false;
boolean playing = false;

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
    int timeHigh = toneDuration[note] + offset;
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
    if (buffer[caretIndex] != 0xFF)
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

void readData()
{
    // Only read data if buffer is less than maxDiff ahead of the caret
    
    if ((bufferCyclesAhead * bufferSize + bufferIndex) - (caretIndex) < maxDiff)
    {
        if (Serial.available() > 0)
        {
            // Read Byte
            
            buffer[bufferIndex] = (byte)Serial.read();
            bufferIndex++;  
            
            // Increment buffer index
            
            if (bufferIndex == bufferSize)
            {
                bufferIndex = 0;
                bufferCyclesAhead++;
            }     
            
            Serial.write(next); // Request next byte
        }   
    } 
}

void setup()
{
    // Begin serial
    
    Serial.begin(baud);
    
    for (int i = 0; i < 9; i++)
        Serial.write(next);
        
    // Calculate Tone Durations

    for (int i = 0; i < 128; i++)
        toneDuration[i] = (int)(pow(2, -(i - 69)/ 12.0) / 440 * 1000000);
        
    pinMode(piezo1, OUTPUT);
    pinMode(piezo2, OUTPUT);        
    
    while (Serial.available() < 8)
    {
        delay(1); // Wait for initializing time division and temp data
    }
    
    for (int i = 0; i < 8; i++)
    {
        buffer[bufferIndex] = (byte)Serial.read();
        bufferIndex++;  
    }
    
    timeDivision = getInt(0);
    microsecondsPerInterval = getInt(4);
}

void loop()
{
    readData();
    playPiezos();
}
