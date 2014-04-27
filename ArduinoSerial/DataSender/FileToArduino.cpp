#include <iostream>
#include <fstream>
#include <vector>
#include <conio.h>
#include "Serial.h"

using namespace std;

// Variables

const char stop = 'S';
const char go = 'G';

string filePath = "Test.txt";
string serialPort = "NNN";
DWORD baudRate = CBR_9600;
int chunkSize = 1;

Serial* mySerial;
bool paused = false;

char * bytes;
int size;

vector<char> feedback;

/** Reads the file at the specified path and stores the file's 
 * byte data in the "bytes"
 *
 */
void readFile(string path)
{
    ifstream file (path.c_str(), ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        bytes = new char[size];
        file.seekg (0, ios::beg);
        file.read (bytes, size);
        file.close();
        cout << "Successfully read " << filePath << ".\n";
    }
    else
    {
        cout << "Error reading " << filePath << ".\n";
    }
}

/** Initializes the Serial Port
 */
bool initSerial(string portName, DWORD baudRate)
{
    vector<char> temp(portName.begin(), portName.end()); // convert string to char *
    temp.push_back('\0');
    
    mySerial = new Serial(&temp[0], baudRate);

    if (mySerial->IsConnected())
    {
        cout << "Successfully connected to port " << portName << "." << endl;
        return true;
    }
    else
    {
        cout << "Could not connect to port " << portName << "." << endl;
        return false;
    }
}

void send (char * bytes, int length)
{
    char * msgBuffer = new char [1];
    cout << endl;
    for (int i = 0; i < length; i += chunkSize)
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
            if (!mySerial->WriteData(&bytes[i], chunkSize)) // Write a chunk
            {
                i -= chunkSize; // Go back a chunk if write was unsuccessful
                cout << "!";
            }
        }
    }
    
    printf("\nSent %u bytes.\n", size);
}

int main()
{
    cout << "File path: ";
    cin >> filePath;
    cout << "Serial port: ";
    cin >> serialPort;
    cout << "Baud rate: ";
    cin >> baudRate;
    cout << endl;
    
    readFile(filePath);
    cout << endl;
    
    bool serialWorks = initSerial(serialPort, baudRate);
    if (serialWorks)
        send(bytes, size);

    cout << endl << "Press any key to continue... ";
    getch();
    return 0;
}
