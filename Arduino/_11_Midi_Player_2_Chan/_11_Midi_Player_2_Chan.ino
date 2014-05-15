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

const byte next = 'N';

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

/** Plays the specified notes for the specified duration.
 *
 * @param note1         MIDI note number of piezo1 note(0-127)
 * @param note2         MIDI note number of piezo2 note(0-127)
 * @param microseconds  duration in microseconds
 */
void playNotes(int note1, int note2, unsigned long microseconds)
{
    unsigned long timeHigh1 = microseconds;
    unsigned long timeHigh2 = microseconds;

    if ((note1 & 0xFF) != 0xFF)
        timeHigh1 = toneDuration[note1];

    if ((note2 & 0xFF) != 0xFF)
        timeHigh2 = toneDuration[note2];

    unsigned long elapsed = 0;
    unsigned long next1 = timeHigh1;
    unsigned long next2 = timeHigh2;

    byte states[3] = {LOW, 0, HIGH};
    int set1 = -1;
    int set2 = -1;

    while (elapsed < microseconds)
    {
        if (elapsed == next1)
        {
            digitalWrite(piezo1, states[set1 + 1]);
            set1 *= -1;
            next1 += timeHigh1;
        }
        if (elapsed == next2)
        {
            digitalWrite(piezo2, states[set2 + 1]);
            set2 *= -1;
            next2 += timeHigh2;
        }

        unsigned long wait1 = next1 - elapsed;
        unsigned long wait2 = next2 - elapsed;

        unsigned long wait = min(wait1, wait2);
        delayMicroseconds(wait);
        elapsed += wait;
    }
}

/** Increments caret; Loops back if necessary
 */
void incrementCaretIndex()
{
    caretIndex++;
    if (caretIndex == bufferSize)
    {
        caretIndex = 0;
        bufferCyclesAhead--;
    }
}

/** Plays the note for the specified minimum time interval if the
 * caret currently points to a valid note, or delays for the minimum
 * time interval if the caret points to a -1 (signifies a rest).
 */
void playPiezos()
{
    // Retrieve notes from two channels
    
    byte note1 = buffer[caretIndex];
    incrementCaretIndex();
    byte note2 = buffer[caretIndex];
    incrementCaretIndex();
    
    // Play notes
    
    playNotes(note1, note2, microsecondsPerInterval);
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
    readData();
    if (bufferAhead() >= 2)
        playPiezos();
}
