#include <iostream>
#include <fstream>
#include <conio.h>
#include "WaveManipulationClasses/WaveDecoder.h"
#include "WaveManipulationClasses/WaveWriter.h"

using namespace std;

/** Reads the specified wave file, and writes to "DegradedMono.wav" a degraded
 * version of the inputted file. The user inputs the output's sample rate.
 * The output has only 1 channel and 8 bits per sample.
 */
int main ()
{
    // Variables

    string outputPath = "Audio/DegradedMono.wav";
    string filePath;
    int targetSampleRate;
    int length = 0;
    int counter = 0;
    int sampleSize;
    int byteStep;
    int monoLength;
    int adjustment;
    // ^ 8-bit samples are stored as unsigned bytes, ranging from 0 to 255.
    //   16-bit samples are stored as 2's-complement signed integers,
    //   ranging from -32768 to 32767.

    char * bytes;
    char * monoBytes;
    
    // Input
    
    cout << "File path: ";
    getline(cin, filePath);
    
    WaveDecoder wd(filePath);
    bytes = wd.read(length);
    
    cout << "Target sample rate: ";
    cin >> targetSampleRate;
    
    // Process
    
    if (targetSampleRate > wd.sampleRate)
        targetSampleRate = wd.sampleRate;

    adjustment = wd.bitsPerSample == 16 ? 128 : 0;

    sampleSize = wd.bitsPerSample / 8;
    byteStep = wd.numChannels * sampleSize * wd.sampleRate / targetSampleRate;
    monoLength = (length - 44) / byteStep + 1;
    monoBytes = new char[monoLength];
    
	for (int i = 44 + sampleSize - 1; i < length; i += byteStep, counter++)
	{
		monoBytes[counter] = (char)(bytes[i] + adjustment);
	}
	
	// Output mono'd audio
	
	WaveWriter ww(outputPath, targetSampleRate);
	ww.append(monoBytes, monoLength);
	ww.write();
	
	// End

    cout << endl << "Press any key to continue... ";
    getch();
    return 0;
}
