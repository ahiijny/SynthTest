#include <iostream>
#include <fstream>
#include <vector>
#include "BinaryToArduino.h"
#include "Serial.h"

using namespace std;

BinaryToArduino::BinaryToArduino(string portName, int bytes_per_chunk = 1)
{
    serialPort = portName;
    bytesPerChunk = bytes_per_chunk;
    
    go = 'G';
    stop = 'S';
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
        cout << endl;
        for (int i = offset; i < offset + length; i += bytesPerChunk)
        {
            // Check serial for any messages.

            int result = mySerial->ReadData(msgBuffer, 1);

            // If a byte was read, interpret the byte.
            if (result != -1)
            {
                if (msgBuffer[0] == go) // Resume transmission if 'G' is transmitted
                    paused = false;

                else if (msgBuffer[0] == stop) // Stop transmission if 'S' is transmitted
                    paused = true;

                cout << msgBuffer[0];
            }

            // Write a chunk of data if not paused

            if (!paused)
            {
                if (!mySerial->WriteData(&bytes[i], bytesPerChunk)) // Write a chunk
                {
                    i -= bytesPerChunk; // Go back a chunk if write was unsuccessful
                    cout << "!";
                }
            }
        }

        printf("\nSent %u bytes.\n", length);
    }
    else
        cout << "Error: Not connected to serial port.\n";
}
