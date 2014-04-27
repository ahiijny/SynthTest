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

int speakerPin = 9;

int sampleRate = 3906;
int amplitude = 64;
int sampleResolution = 1000000 / sampleRate;

int microsecondsPerBeat = 300000; // default is 120 bpm = 500000 us / beat
int ticksPerQuarterNote = 120;
int microsecondsPerTick = microsecondsPerBeat / ticksPerQuarterNote;

int length1 = 2; // the number of notes
int length2 = 2; // the number of notes
int length3 = 2;

// Track 1

char notes1[] = {60, 60}; // 0 indicates rest

int dt1[] =   {0, 4800};

char vel1[] =   {64, 0};

// Track 2

char notes2[] = {64, 64};

int dt2[] = {0, 4800};

char vel2[] = {64, 0};

// Track 3

char notes3[] = {67, 67};

int dt3[] = {0, 4800};

char vel3[] = {64, 0};

// Other variables

int toneDuration[128]; // Now the duration of an entire wave cycle
char noteState[128]; // The velocity (loudness) of each state
int lookUpResolution = 128;
char * lookUpTable;

long previousUpdate = 0;

int waveType = 0;

long timer1 = micros();
long timer2 = micros();
long timer3 = micros();

int caret1 = 0;
int caret2 = 0;
int caret3 = 0;

/** From: playground.arduino.cc/Code/PwmFrequency
 *        forum.arduino.cc/index.php/topic,16612.0.html#4 
 */
void setPwmFrequency(int pin, int divisor) 
{
    byte mode;
    if(pin == 5 || pin == 6 || pin == 9 || pin == 10) 
    {
        switch(divisor) 
        {
            case 1: mode = 0x01; break;
            case 8: mode = 0x02; break;
            case 64: mode = 0x03; break;
            case 256: mode = 0x04; break;
            case 1024: mode = 0x05; break;
            default: return;
        }
        if(pin == 5 || pin == 6) 
        {
            TCCR0B = TCCR0B & 0b11111000 | mode;
        } 
        else 
        {
            TCCR1B = TCCR1B & 0b11111000 | mode;
        }
    } 
    else if(pin == 3 || pin == 11) 
    {
        switch(divisor) 
        {
          case 1: mode = 0x01; break;
          case 8: mode = 0x02; break;
          case 32: mode = 0x03; break;
          case 64: mode = 0x04; break;
          case 128: mode = 0x05; break;
          case 256: mode = 0x06; break;
          case 1024: mode = 0x7; break;
          default: return;
        }
        TCCR2B = TCCR2B & 0b11111000 | mode;
    }
}

void setup()
{
    pinMode(speakerPin, OUTPUT);

    // Calculate Tone Durations

    for (int i = 0; i < 128; i++)
        toneDuration[i] = (int)(pow(2, -(i - 69)/ 12.0) / 440 * 1000000);
        
    // Set PWM frequency
    
    setPwmFrequency(speakerPin, 8); // 31250 Hz divide 8 = 3906.25 Hz

    // Init Lookup Table

    lookUpTable = new char[lookUpResolution];

    if (waveType == 0) // square
    {
        for (int i = 0; i < lookUpResolution; i++)
        {
            if (i < lookUpResolution / 2)
                lookUpTable[i] = 128 + amplitude;
            else
                lookUpTable[i] = 128 - amplitude;
        }
    }
    else if (waveType == 1) // sine
    {
        for (int i = 0; i < lookUpResolution; i++)
        {
            lookUpTable[i] = (char)(amplitude * sin(2 * M_PI / lookUpResolution * i) + 128);
        }
    }
    else if (waveType == 2) // sawtooth
    {
        for (int i = 0; i < lookUpResolution; i++)
        {
            lookUpTable[i] = (char)((i - lookUpResolution/2) * amplitude * 2 / lookUpResolution + 128);
        }
    }
}

void updateState()
{
    // Calculate elapsed ticks since last event for all tracks

    previousUpdate = micros();
    long ticks1 = (previousUpdate - timer1) / microsecondsPerTick;
    long ticks2 = (previousUpdate - timer2) / microsecondsPerTick;
    long ticks3 = (previousUpdate - timer3) / microsecondsPerTick;

    // Check if there are any events that should be effected

    while (caret1 < length1 && ticks1 >= dt1[caret1])
    {
        noteState[(int)notes1[caret1]] = vel1[caret1];
        caret1++;
        ticks1 = 0;
        timer1 = previousUpdate;
    }

    while (caret2 < length2 && ticks2 >= dt2[caret2])
    {
        noteState[(int)notes2[caret2]] = vel2[caret2];
        caret2++;
        ticks2 = 0;
        timer2 = previousUpdate;
    }

    while (caret3 < length3 && ticks3 >= dt3[caret3])
    {
        noteState[(int)notes3[caret3]] = vel3[caret3];
        caret3++;
        ticks3 = 0;
        timer3 = previousUpdate;
    }

    // Sum all active waves; Subtract 128 from each so that middle is neutral

    int mixedWaveState = 0;

    for (int i = 0; i < 128; i++)
    {
        if (noteState[i] != 0)
        {
            int lookUpIndex = lookUpResolution * (previousUpdate % toneDuration[i]) / toneDuration[i];
            int sample = ((int)(lookUpTable[lookUpIndex]) & 0xFF) - 128;
            sample = sample * noteState[i] / 128;
            mixedWaveState += sample;
        }
    }
    mixedWaveState += 128; // Recenter waveform

    if (mixedWaveState > 255) // Clip waveform if necessary
        mixedWaveState = 255;
    else if (mixedWaveState < 0)
        mixedWaveState = 0;

    analogWrite(speakerPin, mixedWaveState); // Write state
}

void loop()
{
    while (!(caret1 >= length1 && caret2 >= length2 && caret3 >= length3))// && caret2 >= length2 && caret3 >= length3))
    {
        long elapsedTime = micros() - previousUpdate;
        if (elapsedTime >= sampleResolution)
            updateState();
        else
            delayMicroseconds(sampleResolution - elapsedTime);
    }
}

