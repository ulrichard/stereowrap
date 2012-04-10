

const int leftEye   =  3;  
const int rightEye  =  4; 
int inByte = 0;

void setup()   
{       
  Serial.begin(38400);  
  // initialize the digital pin as an output:
  pinMode(leftEye, OUTPUT);     
  pinMode(rightEye, OUTPUT);      
}

void loop()                     
{
    if(Serial.available() > 0) 
    {
        // get incoming byte:
        inByte = Serial.read();
        
        if(inByte == 'l')
        {
            digitalWrite(leftEye,  HIGH);
            digitalWrite(rightEye, LOW);
        }
        if(inByte == 'r')
        {
            digitalWrite(leftEye,  LOW);
            digitalWrite(rightEye, HIGH);
        }
    }
}

