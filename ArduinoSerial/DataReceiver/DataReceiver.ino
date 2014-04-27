unsigned long localStart = micros();
unsigned long localEnd = micros();

bool receiving = true;
bool processing = false;
int baud = 9600;

int serialBufferLimitLow = 16; // max Serial.available() value for resuming reading
int serialBufferLimitHigh = 48; // min Serial.available() value for stopping reading

const int bufferSize = 1024;
byte buffer[bufferSize]; // stores bytes of data that are read in

int bufferIndex = 0;
int caretIndex = 0;

int bufferCyclesAhead = 0;
int minDiff = bufferSize / 4;        // 256 : min value for how far ahead bufferIndex is
int maxDiff = bufferSize - minDiff;    // 768 : max value for how far ahead bufferIndex is

/** Reads data from serial only if the bufferIndex isn't too
 * ahead of the caretIndex, and if there exists data to be read.
 */
void readSerial()
{
    // Only read data if buffer is less than maxDiff ahead of the caret
    
    if ((bufferCyclesAhead * bufferSize + bufferIndex) - (caretIndex) < maxDiff)
    {
        // If transmission was halted, resume transmission if buffer is empty enough            
        
        if (!receiving && Serial.available() < serialBufferLimitLow)
        {
             receiving = true;
             Serial.write('G');  // Send message to resume sending. 'G' for "Go".
        }
                
        if (Serial.available() > 0)
        {                       
            // Read data           
            
            buffer[bufferIndex] = (byte)Serial.read();
            bufferIndex++;
            
            // Avoid serial buffer overflow
            
            if (Serial.available() > serialBufferLimitHigh)
            {
                if (receiving)
                {
                    receiving = false;
                    Serial.write('S'); // send message to suspend sending. 'S' for "Stop".
                }
            }
            
            // Increment buffer index
            
            if (bufferIndex == bufferSize)
            {
                bufferIndex = 0;
                bufferCyclesAhead++;
            }                          
        }
        
    } 
    else
    {
        if (receiving)
        {
            receiving = false;
            Serial.write('S'); // send message to suspend sending. 'S' for "Stop".
        }
    }
}

/** Do stuff with the data.
 */
void process()
{
    caretIndex++;
    if (caretIndex == bufferSize)
    {
        caretIndex = 0;
        bufferCyclesAhead--;
    }
}

void setup()
{
    Serial.begin(baud);
}

void loop()
{    
    readSerial();
    
    // Only do stuff with the data if the caretIndex isn't too close to catching up with the bufferIndex
    if ((bufferCyclesAhead * bufferSize + bufferIndex) - (caretIndex) > minDiff)
    {
        processing = true;
        process();                
    }
    else
        processing = false;    
}
