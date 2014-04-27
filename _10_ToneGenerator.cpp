#include <iostream>
#include <fstream>
#include <conio.h>
#include <cmath>
#include "WaveManipulationClasses/WaveWriter.h"

#define _USE_MATH_DEFINES

using namespace std;

int main ()
{
    // Variables
    
    string targetPath = "Audio/Tone.wav";
    const char LOW = 64;
    const char HIGH = 192;
    int sampleRate;
    int duration;
    int frequency;
    int lookupResolution;
    int waveType;
    int amplitude;
    char * lookUpTable;
    
    // Input
    
    cout << "Sample rate: ";
    cin >> sampleRate;
    cout << "Duration (ms): ";
    cin >> duration;
    cout << "Frequency (Hz): ";
    cin >> frequency;
    cout << "Wave type (0 = square, 1 = sine, 2 = sawtooth): ";
    cin >> waveType;
    cout << "Lookup table resolution: ";
    cin >> lookupResolution;
    cout << "Amplitude (0 to 127): ";
    cin >> amplitude;
    
    // Generate lookup table
    
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
    else
    {
        // Lookup table is random noise.
    }
    
    // Generate wave

    WaveWriter ww(targetPath, sampleRate);
    float wavePeriod = 1000000 / frequency;
    float sampleDuration = 1000000 / sampleRate;
    
    long maxSamples = duration * sampleRate / 1000;
    
    for (long i = 0; i < maxSamples; i++)
    {
        long elapsedCycles = frequency * i / sampleRate;
        float mod = (1.0 * frequency * i / sampleRate) - elapsedCycles;
        char value = lookUpTable[(int)(mod * lookupResolution)];
        ww.append(value);
    }
    
    ww.write();
    
    // End

    cout << endl << "Press any key to continue... ";
    getch();
    return 0;
}
