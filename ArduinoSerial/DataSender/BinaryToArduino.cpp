#include <iostream>
#include <fstream>
#include <vector>
#include "BinaryToArduino.h"
#include "Serial.h"

using namespace std;

BinaryToArduino::BinaryToArduino(string portName)
{
    serialPort = portName;
    next = 'N';
}

bool BinaryToArduino::connected()
{
    return mySerial->IsConnected();
}

/** Initializes the Serial Port
 */
bool BinaryToArduino::connectSerial(DWORD baud_rate = CBR_9600)
{
    baudRate = baud_rate;
    
    vector<char> temp(serialPort.begin(), serialPort.end()); // convert string to char *
    temp.push_back('\0');
    
    mySerial = new Serial(&temp[0], baudRate);

    if (mySerial->IsConnected())
    {
        cout << "Successfully connected to port " << serialPort << "." << endl;
        return true;
    }
    else
    {
        cout << "Could not connect to port " << serialPort << "." << endl;
        return false;
    }
}

void BinaryToArduino::send(char * bytes, int length, int offset)
{
    if (connected())
    {
        cout << "Sending " << length << " bytes...\n";
        
        char * msgBuffer = new char [1];
        int result;
        cout << endl;
        
        for (int i = offset; i < offset + length; i ++)
        {
            do // Check serial for any messages.
            {
                result = mySerial->ReadData(msgBuffer, 1);
            }
            while (result == -1); // Delay until next byte is requested.

            if (msgBuffer[0] != next)
                cout << msgBuffer[0];
                
            if (!mySerial->WriteData(&bytes[i], 1)) // Write a chunk
            {
                i--; // Go back a chunk if write was unsuccessful
                cout << "!";
            }        
        
        }

        printf("\nSent %u bytes.\n", length);
    }
    else
        cout << "Error: Not connected to serial port.\n";
}
