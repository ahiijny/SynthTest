#include <iostream>
#include <fstream>
#include <cstdio>
#include "MidiWriter.h"

using namespace std;

/** http://www.mobilefish.com/tutorials/midi/midi_quickguide_specification.html
 * http://www.sonicspot.com/guide/midifiles.html
 */
MidiWriter::MidiWriter(string file_path)
{
    path = file_path.c_str();
    
    formatType = 0;
    trackNum = 1;
    pulsesPerQuarterNote = 128;
    isTimeDivisionInPPQN = true;
    framesPerSecond = -30;
    ticksPerFrame = 120;
    activeEventStatus = 0;
    
    setMidiHeader();
    newTrack();
}

//-------------------------------
//------ Other Methods ----------
//-------------------------------

void MidiWriter::append (char datum)
{
    bytes.push_back(datum);
}

void MidiWriter::append (char * data, int length)
{
    for (int i = 0; i < length; i++)
        bytes.push_back(data[i]);
}

/** @return the number of bytes in this WAV file. This includes
 * 			the 44 bytes from the header.
 */
int MidiWriter::size()
{
    return bytes.size();
}

/** Replaces a sequence of already appended bytes.
 *
 * @param start		the index of the first byte to replace
 * @param newBytes	the replacement bytes
 * @param length    the number of bytes
 */
void MidiWriter::swap(int start, char * newBytes, int length)
{
    for (int i = 0; i < length; i++)
        bytes[start + i] = newBytes[i];
}

//-------------------------------
//------- Helper Methods---------
//-------------------------------

/** Converts a short into 2 bytes (big endian).
 */
char * MidiWriter::getBytes(short n)
{
    char * two = new char[2];

    for (int i = 1; i >= 0; i--)
    {
        two[i] = (char)n;
        n >>= 8;
    }

    return two;
}

/** Converts an int into 4 bytes (big endian).
 */
char * MidiWriter::getBytes(int n)
{
    char * four = new char[4];

    for (int i = 3; i >= 0; i--)
    {
        four[i] = (char)n;
        n >>= 8;
    }

    return four;
}

/** Converts an int into the specified number of bytes (big endian).
 */
char * MidiWriter::getBytes(int n, int byteNum)
{
    char * var = new char[byteNum];

    for (int i = byteNum - 1; i >= 0; i--)
    {
        var[i] = (char)n;
        n >>= 8;
    }

    return var;
}

/** Convents an int to bytes (big endian, variable length format).
 */
char * MidiWriter::getVarLenBytes (int number, int &length)
{
    length = 4;
    int mask = 0x0FE00000;
    
    // Determine number of bytes required
    
    while ((mask & number) == 0 && length > 1)
    {
        mask >>= 7;
        length--;
    }
    
    // Fragment int into bytes

    char * var = new char[length];
    
    for (int i = length - 1; i >= 0; i--)
    {
        var[i] = number & 0x7F;
        var[i] |= 0x80; // 1xxxxxxx signals existance of continuing bit
        number >>= 7;
    }
    var[length - 1] &= 0x7F; // last byte has no leading zero
    
    return var;
}

//---------------------------------
//------- Encoding Methods---------
//---------------------------------

//--------- Header Chunk ------------//

char * MidiWriter::getTimeDivision(short ppqn)
{
    if ((ppqn & 0x8000) == 0x8000)
        ppqn &= 0x7FFF;
    return getBytes(ppqn);
}

char * MidiWriter::getTimeDivision(char fps, char ticksPerFrame)
{
    if ((fps & 0x80) != 0x80) // fps needs to be 1xxxxxxx
        fps *= -1;

    short code = (fps << 8) & 0xF0;
    code |= ticksPerFrame;
    
    return getBytes(code);
}

void MidiWriter::setMidiHeader()
{
    // Chunk ID
    
    append('M');
    append('T');
    append('h');
    append('d');
    
    // Chunk Size
    
    append(getBytes(6), 4);
    
    // Header data
    
    append(getBytes(formatType), 2);
    append(getBytes(trackNum), 2);
    if (isTimeDivisionInPPQN)
        append(getTimeDivision(pulsesPerQuarterNote), 2);
    else
        append(getTimeDivision(framesPerSecond, ticksPerFrame), 2);
}

void MidiWriter::updateFormatType(short format)
{
    formatType = format;
    if (size() >= 14) // already wrote header
        swap(8, getBytes(format), 2);
}

void MidiWriter::updateTrackNum(short num)
{
    trackNum = num;
    if (size() >= 14) // already wrote header
        swap(10, getBytes(num), 2);
}

void MidiWriter::updateTimeDivision(short ppqn)
{
    pulsesPerQuarterNote = ppqn;
    isTimeDivisionInPPQN = true;
    if (size() >= 14) // already wrote header
        swap(12, getTimeDivision(ppqn), 2);
}

void MidiWriter::updateTimeDivision(char fps, char ticks_per_frame)
{
    framesPerSecond = fps;
    ticksPerFrame = ticks_per_frame;
    isTimeDivisionInPPQN = false;
    if (size() >= 14) // already wrote header
        swap(12, getTimeDivision(fps, ticks_per_frame), 2);
}

//--------- Track Chunk ------------//

void MidiWriter::newTrack()
{
    append('M');
    append('T');
    append('r');
    append('k');

    append(getBytes(0), 4);
    trackStartIndex = size();
}

void MidiWriter::endTrack()
{
    char eventData[] = {0x00, 0xFF, 0x2F, 0x00};
    activeEventStatus = 0;
    appendEvent(eventData, 4);
}

void MidiWriter::updateTrackChunkSize()
{
    int length = size() - trackStartIndex;
    swap(trackStartIndex - 4, getBytes(length), 4);
}

//-----------------------------------//
//------------ Events ---------------//
//-----------------------------------//

void MidiWriter::appendEvent(char * data, int dataLength)
{
    append(data, dataLength);
    activeEventStatus = 0;
    updateTrackChunkSize();
}

/** @param length       the length of the bytes array
 */
void MidiWriter::appendEvent(int deltaTime, char * data, int dataLength)
{
    int dtSize = 0;
    append(getVarLenBytes(deltaTime, dtSize), dtSize);
    append(data, dataLength);
    activeEventStatus = 0;
    updateTrackChunkSize();
}

void MidiWriter::appendEvent(int deltaTime, char statusByte, char * data, int dataLength)
{
    int dtSize = 0;
    append(getVarLenBytes(deltaTime, dtSize), dtSize);
    if (statusByte != activeEventStatus) // If the status byte is the same as before, repetition is unnecessary
    {
        append(statusByte);
        activeEventStatus = statusByte;
    }
    append(data, dataLength);
    updateTrackChunkSize();
}


//------------ Midi Events --------------//


/** E.g., "Note On" has the status byte 1001nnnn(9n), so the command parameter
 *  in this case would be 0x90.
 */
void MidiWriter::appendMidiEvent(int deltaTime, char command, int channel, int var1, int var2)
{
    char statusByte = (command & 0xF0) | (channel & 0x0F);
    char var1Byte = 0x7F & var1;
    char var2Byte = 0x7F & var2;
    char eventData[] = {var1Byte, var2Byte};
    
    appendEvent(deltaTime, statusByte, eventData, 2);
}

void MidiWriter::appendMidiEvent(int deltaTime, char command, int channel, int var1)
{
    char statusByte = (command & 0xF0) | (channel & 0x0F);
    char var1Byte = 0x7F & var1;
    char eventData[] = {var1Byte};

    appendEvent(deltaTime, statusByte, eventData, 1);
}

void MidiWriter::noteOn(int deltaTime, int channel, int note, int velocity)
{
    appendMidiEvent(deltaTime, 0x90, channel, note, velocity);
}

void MidiWriter::noteOff(int deltaTime, int channel, int note, int velocity)
{
    appendMidiEvent(deltaTime, 0x80, channel, note, velocity);
}

void MidiWriter::aftertouch(int deltaTime, int channel, int note, int pressure)
{
    appendMidiEvent(deltaTime, 0xA0, channel, note, pressure);
}

void MidiWriter::selectInstrument(int deltaTime, int channel, int instrument)
{
    appendMidiEvent(deltaTime, 0xC0, channel, instrument);
}

//------------ Meta Events --------------//

void MidiWriter::appendMetaEvent(char typeByte, int dataLength, char * data)
{
    int lengthByteLength = 0;
    char * lengthByte = getVarLenBytes(dataLength, lengthByteLength);
    char eventData[dataLength + lengthByteLength + 1];
    
    // Copy typeByte data
    
    eventData[0] = typeByte;

    // Copy length data
    
    for (int i = 0; i < lengthByteLength; i++)
        eventData[i + 1] = lengthByte[i];
        
    // Copy data
    
    for (int i = 0; i < dataLength; i++)
        eventData[i + lengthByteLength + 1] = data[i];
        
    appendEvent(0, 0xFF, eventData, dataLength + lengthByteLength + 1);
}

void MidiWriter::setMicrosecondsPerBeat(int microSecondsPerQuarterNote)
{
    char * uspqn = getBytes(microSecondsPerQuarterNote, 3);
    appendMetaEvent(0x51, 3, uspqn);
}

void MidiWriter::appendSequenceNumber(short num)
{
    appendMetaEvent(0x00, 2, getBytes(num));
}

void MidiWriter::appendTextEvent(int textLength, char * text)
{
    appendMetaEvent(0x01, textLength, text);
}

void MidiWriter::appendCopyrightNotice(int textLength, char * text)
{
    appendMetaEvent(0x02, textLength, text);
}

void MidiWriter::appendTrackName(int textLength, char * text)
{
    appendMetaEvent(0x03, textLength, text);
}

void MidiWriter::appendInstrumentName(int textLength, char * text)
{
    appendMetaEvent(0x04, textLength, text);
}

void MidiWriter::appendLyrics(int textLength, char * text)
{
    appendMetaEvent(0x05, textLength, text);
}

void MidiWriter::appendMarker(int textLength, char * text)
{
    appendMetaEvent(0x06, textLength, text);
}

void MidiWriter::appendCuePoint(int textLength, char * text)
{
    appendMetaEvent(0x07, textLength, text);
}

void MidiWriter::appendMidiChannelPrefix(int channel)
{
    char data[] = {(char)(channel & 0x0F)};
    appendMetaEvent(0x20, 1, data);
}

/** Writes bytes in buffer to file.
 */
bool MidiWriter::write()
{
    // these flags say:
    //    `out` - we will be writing data into the file
    //    `binary` - we will be writing binary data and not simple text
    //    `trunc` - if the file already exists, truncate (wipe out) the existing data

    ofstream fout(path, ios_base::out | ios_base::binary | ios_base::trunc);

    // couldn't open it (disk error?); fail
    if (!fout.is_open())
        return false;

    fout.write(&bytes[0], bytes.size());
    fout.close();

    cout << "Wrote " << bytes.size() << " bytes to " << path << endl;

    return true;
}
