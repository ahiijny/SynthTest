#include <iostream>
#include <fstream>
#include <conio.h>
#include <vector>
#include "MidiManipulationClasses/MidiDecoder.h"
#include "ArduinoSerial/DataSender/BinaryToArduino.h"
#include "MidiManipulationClasses/MidiWriter.h" // for encoding int data into bytes

using namespace std;

/** Writes bytes to file. Used only for debugging purposes;
 * Writes the simplified note sequence to file.
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

int searchDepth = 1;

/** Returns the index of the active note, or -1 if none are active.
 */
int getActiveNote(bool notes[], int size)
{
    int index = size - 1;
    int numActive = 0;
    for ( ; index >= 0; index--)
    {
        if (notes[index] == true)
        {
            numActive++;
            if (numActive == searchDepth)
                break;
        }
    }
    return index;
}

/** Streams a sequence of midi events from the specified channel to Arduino.
 */
int main()
{
    // Declaration of Variables

    string path;
    string serialPort;
    DWORD baudRate;
    int piezoNum = 2;
    int track[piezoNum];
    int length;
    int noteDivision = 16;

    // Input

    cout << "Midi path: ";
    getline(cin, path);

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

    // Input Serial Port and Other Params

    cout << "Serial port: ";
    cin >> serialPort;
    cout << "Baud rate: ";
    cin >> baudRate;
    for (int i = 0; i < piezoNum; i++)
    {
        cout << "Track " << (i + 1) << ": ";
        cin >> track[i];
    
        if (track[i] < 0 || track[i] >= md.trackNum)
            track[i] = md.trackNum - 1;
    }
    cout << "Simplification resolution: ";
    cin >> noteDivision;

    // Decode time data

    int microsecondsPerBeat = md.microsecondsPerBeat;
    int pulsesPerQuarterNote = md.isTimeDivisionInPPQN ? md.pulsesPerQuarterNote : 500000;
    int ticksPerInterval = pulsesPerQuarterNote * 4 / noteDivision;
    int microsecondsPerInterval = microsecondsPerBeat * 4 / noteDivision;

    cout << "Ticks per interval = " << ticksPerInterval << endl;
    cout << "Microseconds per interval = " << microsecondsPerInterval << endl << endl;

    // Encode time data into byte sequence

    MidiWriter mw(".");
    vector<char> sequence;
    vector< vector<char> > tempTracks;
    for (int i = 0; i < piezoNum; i++)
    {
        vector<char> temp;
        tempTracks.push_back(temp);
    }

    char * divisionData = mw.getBytes(noteDivision); // Note resolution (e.g. 16 = 16th note)
    for (int i = 0; i < 4; i++)
        sequence.push_back(divisionData[i]);

    char * intervalData = mw.getBytes(microsecondsPerInterval); // Microseconds per minimum note resolution
    for (int i = 0; i < 4; i++)
        sequence.push_back(intervalData[i]);
        
    // Loop through each track

    for (int j = 0; j < piezoNum; j++)
    {
        // Decode note data

        long tick = 0;
        int interval = 0;
        int lastInterval = 0;

        bool notes[128]; // Specify if each note is on or off

        for (int i = 0; i < 128; i++) // Init each note "off"
            notes[i] = false;

        int startIndex = md.trackDataStartIndexes[track[j]];
        int endIndex = startIndex + md.trackSizes[track[j]];
        int index = 0;
        int limit = md.deltaTimeStartIndexes.size();

        // Find start of specified track

        while (md.deltaTimeStartIndexes[index] < startIndex)
            index++;

        // Loop through track events

        while (index < limit && md.deltaTimeStartIndexes[index] < endIndex)
        {
            // Get delta time

            int caret = md.deltaTimeStartIndexes[index];
            int dt = md.getVarLen(caret);
            tick += dt;
            interval = tick / ticksPerInterval;

            // Update sequence data if necessary

            if (interval > lastInterval)
            {
                for ( ; lastInterval < interval; lastInterval++)
                    tempTracks[j].push_back((char)getActiveNote(notes, 128));
            }

            // Get event code

            char event = md.eventCodes[index];
            if ((event & 0xF0) == 0x90) // Note on
            {
                int dataIndex = md.eventDataStartIndexes[index];
                int note = (int)md.bytes[dataIndex] & 0xFF; // Byte 1 : MIDI note
                dataIndex++;
                int velocity = (int)md.bytes[dataIndex] & 0xFF; // Byte 2 : velocity (0 = off)
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
        if (j < piezoNum - 1)
        {
            if (track[j] == track[j + 1])
                searchDepth++;
            else
                searchDepth = 1;
        }
    }
    
    // Combine tracks into one sequence
    
    // Find longest track sequence
    
    int maxLength = 0;
    for (int i = 0; i < piezoNum; i++)
        if (tempTracks[i].size() > maxLength)
            maxLength = tempTracks[i].size();
        
    // Pad all shorter track sequences with rests

    for (int i = 0; i < piezoNum; i++)
    {
        while (tempTracks[i].size() < maxLength)
        {
            tempTracks[i].push_back(0xFF);
        }
    }
    
    // Interleave separate tracks into one sequence
        
    for (int i = 0; i < maxLength; i++)
    {
        for (int j = 0; j < piezoNum; j++)
        {
            sequence.push_back(tempTracks[j][i]);
        }
    }

    // Init Serial Port

    BinaryToArduino bta(serialPort);
    bta.connectSerial(baudRate);

    // Send sequence data

    //write("Audio/Seq2.txt", &sequence[0], sequence.size());
    bta.send(&sequence[0], sequence.size(), 0);

    // End

    cout << "Press any key to continue. . . ";
    getch();
    return 0;
}
