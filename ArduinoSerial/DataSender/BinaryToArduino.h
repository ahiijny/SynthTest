#ifndef BINARYTOARDUINO_H_INCLUDED
#define BINARYTOARDUINO_H_INCLUDED

#include <iostream>
#include <fstream>
#include "Serial.h"

using namespace std;

class BinaryToArduino
{
    public:
        BinaryToArduino(string portName);
        Serial* mySerial;
        char * bytes;
        
        DWORD baudRate;
        string serialPort;
        bool paused;
        
        char next;
        
        bool connectSerial(DWORD baudRate);
        bool connected();
        
        void send(char * bytes, int length, int offset);
};

#endif // BINARYTOARDUINO_H_INCLUDED
