#include <iostream>
#include <fstream>
#include "WaveDecoder.h"

using namespace std;

WaveDecoder::WaveDecoder(string file_path)
{
    path = file_path.c_str();
}

/** Converts 2 consecutive bytes (little endian) into an int.
 */
int WaveDecoder::getShort(int startIndex)
{
	int value = 0;
	for (int i = 1; i >= 0; i--)
	{
		value <<= 8;
		value |= (int)bytes[startIndex + i] & 0xFF;
	}
	return value;
}

/** Converts 4 consecutive bytes (little endian) into an int.
 */
int WaveDecoder::getInt(int startIndex)
{
	int value = 0;
	for (int i = 3; i >= 0; i--)
	{
		value <<= 8;
		value |= (int)bytes[startIndex + i] & 0xFF;
	}
	return value;
}

/** Reads the specified parameters from the bytes.
 */
void WaveDecoder::decodeParams()
{
    audioFormat = getShort(20);
	numChannels = getShort(22);
	sampleRate = getInt(24);
	byteRate = getInt(28);
	blockAlign = getShort(32);
	bitsPerSample = getShort(34);

	cout << "audio format: " << audioFormat << endl;
	cout << "number channels: " << numChannels << endl;
	cout << "sample rate: " << sampleRate << endl;
	cout << "byte rate: " << byteRate << endl;
	cout << "block align: " << blockAlign << endl;
	cout << "bits per sample: " << bitsPerSample << endl;
}

/** Reads the byte data from the specified file. The length of these
 * bytes are stored in the given &length variable.
 */
char * WaveDecoder::read(int &length)
{
    ifstream file (path, ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        // Read File

        length = file.tellg();
        size = length;
        bytes = new char [size];
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
