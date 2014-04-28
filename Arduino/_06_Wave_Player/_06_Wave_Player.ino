int speakerPin = 9;

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

unsigned long sampleRate = 1;
unsigned long sampleResolution = 1;
unsigned long previousUpdate = micros();

bool initializingWaveHeader = true;

/** From: playground.arduino.cc/Code/PwmFrequency
 *        forum.arduino.cc/index.php/topic,16612.0.html#4 
 */
void setPwmFrequency(int pin, int divisor) 
{
    byte mode;
    if(pin == 5 || pin == 6 || pin == 9 || pin == 10) 
    {
        switch(divisor) 
        {
            case 1: mode = 0x01; break;
            case 8: mode = 0x02; break;
            case 64: mode = 0x03; break;
            case 256: mode = 0x04; break;
            case 1024: mode = 0x05; break;
            default: return;
        }
        if(pin == 5 || pin == 6) 
        {
            TCCR0B = TCCR0B & 0b11111000 | mode;
        } 
        else 
        {
            TCCR1B = TCCR1B & 0b11111000 | mode;
        }
    } 
    else if(pin == 3 || pin == 11) 
    {
        switch(divisor) 
        {
          case 1: mode = 0x01; break;
          case 8: mode = 0x02; break;
          case 32: mode = 0x03; break;
          case 64: mode = 0x04; break;
          case 128: mode = 0x05; break;
          case 256: mode = 0x06; break;
          case 1024: mode = 0x7; break;
          default: return;
        }
        TCCR2B = TCCR2B & 0b11111000 | mode;
    }
}

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

/** Converts 2 consecutive bytes (little endian) into an int.
 */
unsigned long getShort(int startIndex)
{
    unsigned long value = 0;
    for (int i = 1; i >= 0; i--)
    {
        value <<= 8;
        value |= (unsigned long)buffer[startIndex + i] & 0xFF;
    }
    return value;
}

/** Converts 4 consecutive bytes (little endian) into an int.
 */
unsigned long getInt(int startIndex)
{
    unsigned long value = 0;
    for (int i = 3; i >= 0; i--)
    {
        value <<= 8;
        value |= (unsigned long)buffer[startIndex + i] & 0xFF;
    }
    return value;
}

/** Updates the state of the piezo element.
 */
void updateState()
{
    previousUpdate = micros();
    
    analogWrite(speakerPin, buffer[caretIndex]);
    
    caretIndex++;
    if (caretIndex == bufferSize)
    {
        caretIndex = 0;
        bufferCyclesAhead--;
    }
}

/** Do stuff with the data.
 */
void process()
{
    if (initializingWaveHeader)
    {
        if (caretIndex >= 44)
        {
            sampleRate = getInt(24);
            sampleResolution = 1000000 / sampleRate;
            initializingWaveHeader = false;
        }
    }
    else
    {
        // Update state if enough time has passed
        
        unsigned long elapsedTime = micros() - previousUpdate;
        if (elapsedTime >= sampleResolution)
            updateState();
    }    
}

void setup()
{
    Serial.begin(baud);
    pinMode(speakerPin, OUTPUT);
    setPwmFrequency(speakerPin, 8); // 31250 Hz divide 8 = 3906.25 Hz
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
