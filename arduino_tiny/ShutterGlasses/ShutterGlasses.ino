
#include <SoftwareSerial.h>

// ATMEL ATTINY45 / ARDUINO
//
//                  +-\/-+
// Ain0 (D 5) PB5  1|    |8  Vcc
// Ain3 (D 3) PB3  2|    |7  PB2 (D 2)  Ain1
// Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
//            GND  4|    |5  PB0 (D 0) pwm0
//                  +----+

SoftwareSerial mySerial(0, 1); // receivePin D0 pin5, transmitPin D1 pin6

const int leftEye   =  4; // D4 pin3  
int inByte = 0;

void setup()   
{       
  mySerial.begin(38400);  
  // initialize the digital pin as an output:
  pinMode(leftEye, OUTPUT);    
}

void loop()                     
{
    if(mySerial.available() > 0) 
    {
        // get incoming byte:
        inByte = mySerial.read();
        
        if(inByte == 'l')
        {
            digitalWrite(leftEye,  HIGH);
        }
        if(inByte == 'r')
        {
            digitalWrite(leftEye,  LOW);
        }
    }
}

