/** Outputs a constant analog value to the piezo element.
 * This is to test the effectiveness of an attached low-pass
 * filter (sim.okawa-denshi.jp/en/PWMtool.php, provideyourown.com/2011/analogwrite-convert-pwm-to-voltage/).
 * Silence is good.
 */

const int speakerPin = 9;

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

void setup()
{
    pinMode(speakerPin, OUTPUT);
        
    // Set PWM frequency
    
    setPwmFrequency(speakerPin, 8); // 31250 Hz divide 8 = 3906.25 Hz
}

void loop()
{
    analogWrite(speakerPin, 128);
}
