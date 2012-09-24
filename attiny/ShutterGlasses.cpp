#include <avr/io.h>     
#include <util/delay.h>
#define ARDUINO_MAIN
#include "pins_arduino.h"
#include </usr/share/arduino/libraries/SoftwareSerial/SoftwareSerial.h>

const int leftEyeOutPin   =  3; // PB4
SoftwareSerial mySerial(5, 6);
int inByte = 0;

int main(void)
{
  // Set Port B pins for 3 and 4 as outputs
  // PORTB bit 3 = physical pin #2 on the ATTINY45
  // PORTB bit 4 = physical pin #3 on the ATTINY45

  DDRB = 0x18;  // In binary this is 0001 1000 (note that is bit 3 and 4)
  // AVR-GCC also would accept 0b00011000, by the way.
	
  mySerial.begin(38400);  

  // Set up a forever loop using your favorite C-style 'for' loop
  while(true)  // loop while 1 equals 1
  {

	if(mySerial.available() > 0) 
    {
        // get incoming byte:
        inByte = mySerial.read();
        
        if(inByte == 'l')
        {
            // Set Port B pins for 3 and 4 as HIGH (i.e. turn the LEDs on)
    		PORTB = 0x18;   // If we wanted only PB4 on, it'd be PORTB=0x10
        }
        if(inByte == 'r')
        {
            // Set PORTB to be all LOWs (i.e. turn the LEDs off)
    		PORTB = 0x00;
        }
    }
  }

  return 1;
}

// http://stackoverflow.com/questions/920500/what-is-the-purpose-of-cxa-pure-virtual
extern "C" void __cxa_pure_virtual() { while (1); }

