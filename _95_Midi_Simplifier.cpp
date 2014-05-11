#include <iostream>
#include <fstream>
#include <conio.h>
#include <vector>
#include "MidiManipulationClasses/MidiDecoder.h"
#include "ArduinoSerial/DataSender/BinaryToArduino.h"
#include "MidiManipulationClasses/MidiWriter.h" // for encoding int data into bytes

using namespace std;

/** Writes bytes to file.
 */
bool write(const char * path, char * bytes, int size)
{
    // these flags say:
    //    `out` - we will be writing data into the file
    //    `binary` - we will be writing binary data and not simple text
    //    `trunc` - if the file already exists, truncate (wipe out) the existing data

    ofstream fout(path, ios_base::out | ios_base::binary | ios_base::trunc);

    // couldn't open it (disk error?); fail
    if (!fout.is_open())
        return false;

    fout.write(bytes, size);
    fout.close();

    cout << "Wrote " << size << " bytes to " << path << endl;

    return true;
}

/** Returns the index of the active note, or -1 if none are active.
 */
int getActiveNote(bool notes[], int size)
{
    int index = size - 1;
    for ( ; index >= 0; index--)
        if (notes[index] == true)
            break;

    return index;
}

/** Streams a sequence of midi events from the specified channel to Arduino.
 */
int main()
{
    string path;
    string serialPort;
    DWORD baudRate;
    int track;
    int noteDivision = 16;

    cout << "Midi path: ";
    getline(cin, path);

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

    printf("Microseconds per beat: %i\n", md.microsecondsPerBeat);

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
    cout << "Simplification resolution: ";
    cin >> noteDivision;
        
    // Decode time data
    
    int microsecondsPerBeat = md.microsecondsPerBeat;
    int pulsesPerQuarterNote = md.isTimeDivisionInPPQN ? md.pulsesPerQuarterNote : 500000;
    int ticksPerInterval = pulsesPerQuarterNote * 4 / noteDivision;
    int microsecondsPerInterval = microsecondsPerBeat * 4 / noteDivision;
    
    cout << "Ticks per interval = " << ticksPerInterval << endl;
    cout << "Microseconds per interval = " << microsecondsPerInterval << endl << endl;
    
    // Encode time data
    
    vector<char> sequence;
    MidiWriter mw(".");

    char * divisionData = mw.getBytes(noteDivision);
    for (int i = 0; i < 4; i++)
        sequence.push_back(divisionData[i]);

    char * intervalData = mw.getBytes(microsecondsPerInterval);
    for (int i = 0; i < 4; i++)
        sequence.push_back(intervalData[i]);
    
    // Decode note data
    
    long tick = 0;
    int interval = 0;
    int lastInterval = 0;
    
    bool notes[128];
    
    for (int i = 0; i < 128; i++)
        notes[i] = false;
    
    int startIndex = md.trackDataStartIndexes[track];
    int endIndex = startIndex + md.trackSizes[track];
    int index = 0;
    int limit = md.deltaTimeStartIndexes.size();

    while (md.deltaTimeStartIndexes[index] < startIndex)
        index++;
        
    while (index < limit && md.deltaTimeStartIndexes[index] < endIndex)
    {
        // Get delta time

        int caret = md.deltaTimeStartIndexes[index];
        int dt = md.getVarLen(caret);
        tick += dt;
        interval = tick / ticksPerInterval;
        
        //cout << "Tick = " << tick << " ; Interval = " << interval << endl;

        // Update sequence if necessary

        if (interval > lastInterval)
        {
            for ( ; lastInterval < interval; lastInterval++)
                sequence.push_back((char)getActiveNote(notes, 128));
        }
        
        // Get event code
        
        char event = md.eventCodes[index];
        if ((event & 0xF0) == 0x90) // Note on
        {
            int dataIndex = md.eventDataStartIndexes[index];
            int note = (int)md.bytes[dataIndex] & 0xFF;
            dataIndex++;
            int velocity = (int)md.bytes[dataIndex] & 0xFF;
            if (velocity == 0)
                notes[note] = false;
            else
                notes[note] = true;
        }
        else if ((event & 0xF0) == 0x80) // Note off
        {
            int dataIndex = md.eventDataStartIndexes[index];
            int note = (int)md.bytes[dataIndex] & 0xFF;
            notes[note] = false;
        }

        // Increment index

        index++;
    }

    // Init Serial Port

    BinaryToArduino bta(serialPort);
    bta.connectSerial(baudRate);
    
    // Send sequence data
    
    write("Audio/SequenceTest.txt", &sequence[0], sequence.size());
    //bta.send(&data[0], data.size(), 0);

    // End

    cout << "Press any key to continue. . . ";
    getch();
    return 0;
}
