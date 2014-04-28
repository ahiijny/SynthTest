#include <iostream>
#include <fstream>
#include <conio.h>
#include "MidiManipulationClasses/MidiDecoder.h"

using namespace std;
int main()
{
    string path = "Audio/piano.mid";
    cout << "Midi path: ";
    cin >> path;
    
    // Init
    
    int length;
    MidiDecoder md(path);
    md.read(length);
    
    // Print data

    printf("Format type: %i\n", md.formatType);
    printf("Number of tracks: %i\n", md.trackNum);
    
    if (md.isTimeDivisionInPPQN)
    {
        printf("Pulses per quarter note: %i\n", md.pulsesPerQuarterNote);
    }
    else
    {
        printf("Frames per second: %i\n", md.framesPerSecond);
        printf("Ticks per frame: %i\n", md.ticksPerFrame);
    }
    
    printf("Microseconds per tick: %i\n", md.microsecondsPerTick);
    
    for (int i = 0; i < md.trackNum; i++)
    {
        printf("Index %i: MTrk: ", md.trackDataStartIndexes[i]);
        printf("size = %i\n", md.trackSizes[i]);
    }
    
    // End

    cout << "Press any key to continue. . . ";
    getch();
    return 0;
}
