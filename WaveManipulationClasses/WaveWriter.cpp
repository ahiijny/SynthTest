#include <iostream>
#include <fstream>
#include "WaveWriter.h"

using namespace std;

/** https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
 */
WaveWriter::WaveWriter(string file_path, int sample_rate, int num_channels, int bits_per_sample)
{
    // Initialize Variables
    
    path = file_path.c_str();
    sampleRate = sample_rate;
    numChannels = num_channels;
    bitsPerSample = bits_per_sample;

    // Initialize WAVE Header (PCM format)

    chunkID();
    chunkSize(true);
    format();

    subchunk1ID();
    subchunk1Size();
    audioformat();
    numChannelsSet();
    sampleRateSet();
    byteRate();
    blockAlign();
    bitsPerSampleSet();

    subchunk2ID();
    subchunk2SizeSet(true);
}

//-------------------------------
//------ Other Methods ----------
//-------------------------------

void WaveWriter::append (char datum)
{
    bytes.push_back(datum);
}

void WaveWriter::append (char * data, int length)
{
    for (int i = 0; i < length; i++)
        bytes.push_back(data[i]);
}

/** @return the number of bytes in this WAV file. This includes
 * 			the 44 bytes from the header.
 */
int WaveWriter::size()
{
    return bytes.size();
}

/** Replaces a sequence of already appended bytes.
 *
 * @param start		the index of the first byte to replace
 * @param newBytes	the replacement bytes
 * @param length    the number of bytes
 */
void WaveWriter::swap(int start, char * newBytes, int length)
{
    for (int i = 0; i < length; i++)
        bytes[start + i] = newBytes[i];
}

//-------------------------------
//------- Helper Methods---------
//-------------------------------

/** Converts a short into 2 bytes (little endian).
 */
char * WaveWriter::getBytes (short n)
{
    char * two = new char[2];

    for (int i = 0; i < 2; i++)
    {
        two[i] = (char)n;
        n >>= 8;
    }

    return two;
}

/** Converts an int into 4 bytes (little endian).
 */
char * WaveWriter::getBytes (int n)
{
    char * four = new char[4];

    for (int i = 0; i < 4; i++)
    {
        four[i] = (char)n;
        n >>= 8;
    }

    return four;
}

/** Updates the WAV header with the finalized version of the number of bytes
 * in the "DATA" chunk.
 */
void WaveWriter::finalizeChunkSizes()
{
    subchunk2Size = bytes.size() - 44;
    chunkSize(false);
    subchunk2SizeSet(false);
}

//----------------------------------
//----- Canonical Wave Header ------
//----------------------------------

void WaveWriter::chunkID()
{
    append('R');
    append('I');
    append('F');
    append('F');
}

void WaveWriter::chunkSize(bool init)
{
    int chunkSize = 36 + subchunk2Size;
    if (init)
        append(getBytes(chunkSize), 4);
    else
        swap(4, getBytes(chunkSize), 4);
}

void WaveWriter::format()
{
    append('W');
    append('A');
    append('V');
    append('E');
}

void WaveWriter::subchunk1ID()
{
    append('f');
    append('m');
    append('t');
    append(' ');
}

void WaveWriter::subchunk1Size()
{
    int chunkSize = 16;
    append(getBytes(chunkSize), 4);
}

void WaveWriter::audioformat()
{
    short af = 1;
    append(getBytes(af), 2);
}

void WaveWriter::numChannelsSet()
{
    append(getBytes(numChannels), 2);
}

void WaveWriter::sampleRateSet()
{
    append(getBytes(sampleRate), 4);
}

void WaveWriter::byteRate()
{
    int rate = sampleRate * numChannels * bitsPerSample / 8;
    append(getBytes(rate), 4);
}

void WaveWriter::blockAlign()
{
    short blockAlign = (short)(numChannels * bitsPerSample / 8);
    append(getBytes(blockAlign), 2);
}

void WaveWriter::bitsPerSampleSet()
{
    append(getBytes(bitsPerSample), 2);
}

void WaveWriter::subchunk2ID()
{
    append('d');
    append('a');
    append('t');
    append('a');
}

void WaveWriter::subchunk2SizeSet(bool init)
{
    if (init)
        append(getBytes(subchunk2Size), 4);
    else
        swap(40, getBytes(subchunk2Size), 4);
}

/** Writes bytes in buffer to file.
 */
bool WaveWriter::write()
{
    finalizeChunkSizes();

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
