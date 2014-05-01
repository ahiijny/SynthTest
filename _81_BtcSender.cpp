#include <iostream>
#include <fstream>
#include <conio.h>
#include "ArduinoSerial/DataSender/BinaryToArduino.h"

using namespace std;

char * bytes;
int size;

/** Reads the byte data from the specified file. The length of these
 * bytes are stored in the given &length variable.
 */
char * read(string filePath)
{
    const char * path = filePath.c_str();
    ifstream file (path, ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        // Read File

        size = file.tellg();
        bytes = new char [size];
        file.seekg (0, ios::beg);
        file.read (bytes, size);
        file.close();
        cout << "Successfully read " << path << ": " << size << " bytes" << endl;
    }
    else
    {
        cout << "Error: could not read " << path << endl;
    }
    return bytes;
}

/** Converts an int into 4 bytes (little endian).
 */
char * getBytes (int n)
{
    char * four = new char[4];

    for (int i = 0; i < 4; i++)
    {
        four[i] = (char)n;
        n >>= 8;
    }

    return four;
}

int main()
{
    // Vars

    string path;
    string serialPort;
    DWORD baudRate;
    int sampleRate;

    // Input

    cout << "BTc file path: ";
    getline(cin, path);
    read(path);
    cout << "Serial port: ";
    cin >> serialPort;
    cout << "Baud rate: ";
    cin >> baudRate;
    cout << "Samples per second: ";
    cin >> sampleRate;

    // Init Serial Port

    BinaryToArduino bta(serialPort);
    bta.connectSerial(baudRate);
    
    // Send sample rate

    cout << "Sending sample rate...\n";
    bta.send(getBytes(sampleRate), 4, 0);
    
    // Send binary data
    
    cout << "Sending binary data...\n";
    bta.send(bytes, size, 0);

    cout << "Press any key to continue. . .";
    getch();
    return 0;
}
