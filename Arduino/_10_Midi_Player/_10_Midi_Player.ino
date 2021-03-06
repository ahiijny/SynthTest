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

unsigned int toneDuration[128];
int offset = 0;

int timeDivision = 16;
long microsecondsPerInterval = 31250;


/** Converts 4 consecutive bytes (big endian) into a long.
 */
long getInt(int startIndex)
{
    long value = 0;
    for (int i = 0; i < 4; i++)
    {
        value <<= 8;
        value |= (long)buffer[startIndex + i] & 0xFF;
    }
    return value;
}

/** Plays the specified MIDI note for the specified duration
 *
 * @param note          the MIDI note (0-127)
 * @param microseconds  the duration in microseconds
 */
void playNote(int note, long microseconds)
{
    long numPeriods = microseconds / toneDuration[note];
    unsigned int timeHigh = toneDuration[note] / 2 + offset;
    for (long i = 0; i < numPeriods; i++)
    {
        digitalWrite(piezo1, HIGH);
        digitalWrite(piezo2, HIGH);
        delayMicroseconds(timeHigh);
        digitalWrite(piezo1, LOW);
        digitalWrite(piezo2, LOW);
        delayMicroseconds(timeHigh); 
    }
}

/** Plays the note for the specified minimum time interval if the
 * caret currently points to a valid note, or delays for the minimum
 * time interval if the caret points to a -1 (signifies a rest).
 */
void playPiezos()
{    
    // Play note or rest
    
    if (buffer[caretIndex] != 0xFF)
    {
        playNote(buffer[caretIndex], microsecondsPerInterval);
    }
    else
    {        
        delay(microsecondsPerInterval / 1000);
    }    
            
    // Increment caret; Loop back if necessary
    
    caretIndex++;
    if (caretIndex == bufferSize)
    {
        caretIndex = 0;
        bufferCyclesAhead--;
    }
}

/** Determines how ahead the buffer is relative to the caret, in number of bytes.
 */
int bufferAhead()
{
    return (bufferCyclesAhead * bufferSize + bufferIndex) - (caretIndex);
}

/** Attempts to read a byte from Serial and record it into the buffer array.
 */
boolean readData()
{
    // Only read data if buffer is less than maxDiff ahead of the caret
    
    if (bufferAhead() < maxDiff)
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
            return true;
        }          
    } 
    return false;
}

void setup()
{
    // Begin serial
    
    Serial.begin(baud);
    
    if (Serial.available() > 0)
        Serial.read();
    
    for (int i = 0; i < 9; i++)
        Serial.write(next);
        
    // Calculate Tone Durations

    for (int i = 0; i < 128; i++)
        toneDuration[i] = (unsigned int)(pow(2, -(i - 69)/ 12.0) / 440 * 1000000);
        
    pinMode(piezo1, OUTPUT);
    pinMode(piezo2, OUTPUT);
    
    while (Serial.available() < 8)
    {
        delay(1); // Wait for initializing time division and temp data
    }
    
    // Read first 8 bytes
    
    for (int i = 0; i < 8; i++)
    {
        buffer[bufferIndex] = (byte)Serial.read();
        bufferIndex++;  
    }
    
    // Decode time division and microseconds per interval
    
    timeDivision = getInt(0);
    microsecondsPerInterval = getInt(4);
    
    // Print decoded values, for debugging purposes
    
    Serial.println(timeDivision, DEC);
    Serial.print(microsecondsPerInterval, DEC);
}

void loop()
{
    readData();
    playPiezos();
}
