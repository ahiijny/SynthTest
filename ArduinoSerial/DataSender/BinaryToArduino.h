#ifndef BINARYTOARDUINO_H_INCLUDED
#define BINARYTOARDUINO_H_INCLUDED

#include <iostream>
#include <fstream>
#include "Serial.h"

using namespace std;

class BinaryToArduino
{
    public:
        BinaryToArduino(string portName, int bytes_per_chunk);
        Serial* mySerial;
        char * bytes;
        
        DWORD baudRate;
        string serialPort;
        int bytesPerChunk;
        bool paused;
        
        char go;
        char stop;
        
        bool connectSerial(DWORD baudRate);
        bool connected();
        
        void send(char * bytes, int length, int offset);
};

#endif // BINARYTOARDUINO_H_INCLUDED
