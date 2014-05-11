#include <iostream>
#include <fstream>
#include <cstdio>
#include "MidiDecoder.h"

using namespace std;

/** http://www.mobilefish.com/tutorials/midi/midi_quickguide_specification.html
 * http://www.sonicspot.com/guide/midifiles.html
 */
MidiDecoder::MidiDecoder(string file_path)
{
    path = file_path.c_str();
    microsecondsPerBeat = 500000; // default is 120 bpm
    isTimeDivisionInPPQN = true;
}

/** Converts 2 consecutive bytes (big endian) into an int.
 */
int MidiDecoder::getShort(int startIndex)
{
	int value = 0;
	for (int i = 0; i < 2; i++)
	{
		value <<= 8;
		value |= (int)bytes[startIndex + i] & 0xFF;
	}
	return value;
}

/** Converts 4 consecutive bytes (big endian) into an int.
 */
int MidiDecoder::getInt(int startIndex)
{
	int value = 0;
	for (int i = 0; i < 4; i++)
	{
		value <<= 8;
		value |= (int)bytes[startIndex + i] & 0xFF;
	}
	return value;
}

/** Converts byteNum consecutive bytes (big endian) into an int.
 */
int MidiDecoder::getInt(int startIndex, int byteNum)
{
	int value = 0;
	for (int i = 0; i < byteNum; i++)
	{
		value <<= 8;
		value |= (int)bytes[startIndex + i] & 0xFF;
	}
	return value;
}

/** Converts a variable number of bytes (big endian) into an int.
 * Delta time values are normally stored in a variable-length format
 */
int MidiDecoder::getVarLen(int &index)
{
    bool hasContinuation = true;
    int value = 0;
    while (hasContinuation)
    {
        hasContinuation = ((bytes[index] & 0x80) == 0x80); // byte is in the form 1xxxxxxx
        value <<= 7;
        value |= (int)bytes[index] & 0x7F;
        index++;
    }
    return value;
}

/** Reads the specified parameters from the bytes.
 */
void MidiDecoder::decodeParams()
{
    decodeFormat();
    decodeNumTracks();
    decodeTimeDivision();
    decodeChunks();
}

void MidiDecoder::decodeFormat()
{
    formatType = getShort(8);
}

void MidiDecoder::decodeNumTracks()
{
    trackNum = getShort(10);
}

/** Reads the time division.
 */
void MidiDecoder::decodeTimeDivision()
{
    char one = bytes[12];
    char two = bytes[13];
    
    if ((one & 0x08) == 0x00) // First byte is 0xxxxxxx ; Division is pulses per quarter note (ppqn)
    {
        isTimeDivisionInPPQN = true;
        pulsesPerQuarterNote = getShort(12);
    }
    else // First byte is 1xxxxxxx ; Division is number of frames per second SMPTE time and the number of beats (or ticks) per frame.
    {
        isTimeDivisionInPPQN = false;
        framesPerSecond = (int)one * -1;
        ticksPerFrame = (int)two & 0xFF;
    }
}

void MidiDecoder::decodeChunks()
{
    int index = 14;
    while (index < size)
    {
        if (bytes[index] == 'M')
        {
            if (bytes[index + 1] == 'T')
                if (bytes[index + 2] == 'r')
                    if (bytes[index + 3] == 'k')
                        decodeTrackChunk(index);
        }
        else
        {
            index++;
        }
    }
}

void MidiDecoder::decodeTrackChunk(int &index)
{
    int dataStartIndex;
    char runningEventStatus = 0;
    int chunkSize;
    int chunkEnd;

    trackDataStartIndexes.push_back(index + 8);
    index += 4;

    chunkSize = getInt(index);
    trackSizes.push_back(chunkSize);
    index += 4;
    
    chunkEnd = index + chunkSize;
    
    while (index < chunkEnd)
    {
        deltaTimeStartIndexes.push_back(index);
        int dt = getVarLen(index);
        char statusByte = bytes[index]; // Event type is the first 4 bits: EEEExxxx
        
        //printf("dt = %4i ", dt);

        if ((statusByte & 0x80) == 0x80) // Event byte
        {   // Only increment index and set event status if this is an event status byte.
            // If this is not an event status byte, use the previous event status value.
            
            runningEventStatus = statusByte;
            index++;
        }

        decodeEvent(runningEventStatus, index);
    }
}

/** The index should point to the first data byte.
 */
void MidiDecoder::decodeEvent(char statusByte, int &index)
{
    eventDataStartIndexes.push_back(index);
    eventCodes.push_back(statusByte);
    
    if ((statusByte & 0xF0) == 0xF0) // Status byte is 1111xxxx
    {
        if ((statusByte & 0xFF) == 0xFF) // Status byte is 11111111
            decodeMetaEvent(index);                         // Meta Event
        else
            decodeSystemExclusiveEvent(statusByte, index);  // System Exclusive Event
    }
    else
    {
        decodeMIDIEvent(statusByte, index);                 // MIDI Control Event
    }
}

void MidiDecoder::decodeMIDIEvent(char statusByte, int &index)
{
    char command = statusByte & 0xF0;
    int channel = (int)(statusByte & 0x0F);
    //printf("MIDI %i channel %i %i %i\n", command, channel, bytes[index], bytes[index + 1]);
    
    if (((command & 0xFF) == 0xC0) || ((command & 0xFF) == 0xD0)) // 1100nnnn, 1101nnnn
    {
        // These commands do not have a second data byte
        index++;
        eventSizes.push_back(1); // record event size
    }
    else
    {
        index += 2;
        eventSizes.push_back(2); // record event size
    }
    
}

/** Doesn't need status byte since we already know that it is 11111111.
 */
void MidiDecoder::decodeMetaEvent(int &index)
{
    int before = index;
    int after;
    
    char type = bytes[index];
    index++;
    int length = getVarLen(index);
    /*printf("Meta %x len %i ", type, length);
    for (int i = 0; i < length; i++)
        printf("%4i ", bytes[index + i]);
    cout << endl;*/

    if (type == 0x51)
        microsecondsPerBeat = getInt(index, 3);

    index += length;
    
    // Record event size

    after = index;
    eventSizes.push_back(after - before);
}

void MidiDecoder::decodeSystemExclusiveEvent(char statusByte, int &index)
{
    int before = index;
    int after;
    
    char type = statusByte & 0x0F;
    int length = getVarLen(index);
    /*printf("Syst %x len %1\n", type, length);
    for (int i = 0; i < length; i++)
        printf("%4i ", bytes[index + i]);
    cout << endl;*/
    index += length;
    
    // Record event size
    
    after = index;
    eventSizes.push_back(after - before);
}

/** Reads the byte data from the specified file. The length of these
 * bytes are stored in the given &length variable.
 */
char * MidiDecoder::read(int &length)
{
    ifstream file (path, ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        // Read File

        length = file.tellg();
        size = length;
        bytes = new char[size];
        file.seekg (0, ios::beg);
        file.read (bytes, size);
        file.close();
        cout << "Successfully read " << path << ": " << size << " bytes" << endl;

        // Decode

        decodeParams();
    }
    else
    {
        cout << "Error: could not read " << path << endl;
    }
    return bytes;
}
