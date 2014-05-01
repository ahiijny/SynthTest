int speakerPin = 9;

byte sample[2000];

byte masks[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

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

int waveType = 0;
int period = 100;
int sampleRate = 1000;
int sampleResolution = 1000000/sampleRate;


void setup()
{
    pinMode(speakerPin, OUTPUT);
    //setPwmFrequency(speakerPin, 1); // 31250 Hz divide 8 = 3906.25 Hz
    
    if (waveType == 0) // square
    {
        for (int i = 0; i < 2000; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                int num = 8 * i + j;
                
                if ((num % period) > (period / 2))
                    sample[i] |= masks[j];
            }
        }
    }
}


void loop()
{  
    long start = micros();    
    for (int i = 0; i < 2000; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            long expected = (start + 8 * i + j) * sampleResolution;
            long remaining = micros() - expected;
            if (remaining > 0)
                delayMicroseconds(remaining);
             
            if ((sample[i] & masks[j]) == masks[j])
                digitalWrite(speakerPin, HIGH);
            else
                digitalWrite(speakerPin, LOW);        
        }
    }
}
