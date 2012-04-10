

int leftEye   =  3;  
int rightEye  =  4; 

void setup()   
{       
  Serial.begin(19200);  
  // initialize the digital pin as an output:
  pinMode(leftEye, OUTPUT);     
  pinMode(rightEye, OUTPUT);      
}

// the loop() method runs over and over again,
// as long as the Arduino has power

int delaytime = 100;
int cnt = 0;
void loop()                     
{

  digitalWrite(leftEye, HIGH);
  digitalWrite(rightEye, LOW);
  delay(delaytime);
  digitalWrite(leftEye, LOW);
  digitalWrite(rightEye, HIGH);
  delay(delaytime);
}

