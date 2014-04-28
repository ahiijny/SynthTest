#include <iostream>
#include <fstream>
#include <conio.h>
#include "ArduinoSerial/DataSender/BinaryToArduino.h"
#include "WaveManipulationClasses/WaveDecoder.h"
#include "WaveManipulationClasses/WaveWriter.h"

using namespace std;

char * bytes;
int size;

int main()
{
    // Vars
    
    string path;
    string serialPort;
    DWORD baudRate;
    int bytesPerChunk = 1;
    int targetSampleRate = 3906;

    int length = 0; // From wave degrader
    int counter = 0;
    int sampleSize;
    int stepQuanta;
    int byteStep;
    int monoLength;
    int adjustment;
    char * monoBytes;
    
    // Input
    
    cout << "Wave file path: ";
    getline(cin, path);
    WaveDecoder wd(path);
    bytes = wd.read(length);
    cout << "Serial port: ";
    cin >> serialPort;
    cout << "Baud rate: ";
    cin >> baudRate;
    cout << "Bytes per chunk (sending to Arduino): ";
    cin >> bytesPerChunk;
    cout << "Target samples per second: ";
    cin >> targetSampleRate;
    
    // Init Serial Port
    
    BinaryToArduino bta(serialPort, 1);
    bta.connectSerial(baudRate);
    
    // From Wave Degrader:

    // Process

    if (targetSampleRate > wd.sampleRate)
        targetSampleRate = wd.sampleRate;

    adjustment = wd.bitsPerSample == 16 ? 128 : 0;
    sampleSize = wd.bitsPerSample / 8;
    stepQuanta = wd.numChannels * sampleSize;
    monoLength = int((length - 44.0) / stepQuanta * targetSampleRate / wd.sampleRate);
    monoBytes = new char[monoLength];

    for (int i = 0; i < monoLength; i++)
    {
        int index = (44 + sampleSize - 1) + stepQuanta * int(1.0 * i * wd.sampleRate / targetSampleRate);
        monoBytes[i] = (char)(bytes[index] + adjustment);
    }

    // Send data
    
    WaveWriter ww("Audio/SentToArduino.wav", targetSampleRate, 1, 8);
    ww.append(monoBytes, monoLength);
    ww.finalizeChunkSizes();
    bta.send(&ww.bytes[0], ww.size(), 0);
    
    cout << "Press any key to continue. . .";
    getch();
    return 0;
}
