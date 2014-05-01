#include <iostream>
#include <fstream>
#include <conio.h>
#include <vector>
#include "MidiManipulationClasses/MidiDecoder.h"
#include "ArduinoSerial/DataSender/BinaryToArduino.h"

using namespace std;

/** Streams a sequence of midi events from the specified channel to Arduino.
 */
int main()
{
    string path;
    string serialPort;
    DWORD baudRate;
    int track;
    cout << "Midi path: ";
    cin >> path;
    
    // Input

    int length;
    MidiDecoder md(path);
    md.read(length);
    
    // Print Midi data
    
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
    
    // Serial Port
    
    cout << "Serial port: ";
    cin >> serialPort;
    cout << "Baud rate: ";
    cin >> baudRate;
    cout << "Track: ";
    cin >> track;
    if (track < 0 || track >= md.trackNum)
        track = md.trackNum - 1;

    // Init Serial Port

    BinaryToArduino bta(serialPort);
    bta.connectSerial(baudRate);
    
    // Pick and choose event data

    int index = 14;
    int ticksPerBeat = 1;
    vector<char> events;
    
    // Start event stream with ticks per beat:

    events.push_back(md.bytes[12]);
    events.push_back(md.bytes[13]);
    
    // Code in the rest of the data:
    
    int startIndex = md.trackDataStartIndexes[track];
    int endIndex = startIndex + md.trackSizes[track];
    index = 0;
    
    while (md.deltaTimeStartIndexes[index] <= startIndex)
        index++;
        
    while (md.deltaTimeStartIndexes[index] < endIndex)
    {
        // Add delta time
        
        int caret = md.deltaTimeStartIndexes[index];
        bool hasContinuation = true;
        while (hasContinuation)
        {
            hasContinuation = ((md.bytes[caret] & 0x80) == 0x80); // byte is in the form 1xxxxxxx
            events.push_back(md.bytes[caret]);
            caret++;
        }
        
        // Add event code
        
        events.push_back(md.eventCodes[index]);
        
        // Add event data
        
        caret = md.eventDataStartIndexes[index];
        int size = md.eventSizes[index];
        
        for (int i = 0; i < size; i++)
            events.push_back(md.bytes[caret + i]);

        // Increment index
        
        index++;
    }
    
    // Send data
    
    bta.send(&events[0], events.size(), 0);

    // End

    cout << "Press any key to continue. . . ";
    getch();
    return 0;
}
