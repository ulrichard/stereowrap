
#include <SoftwareSerial.h>

SoftwareSerial mySerial(5, 6);

const int leftEye   =  3;  
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

