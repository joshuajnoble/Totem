


int pin = 13;
//volatile int state = LOW;
volatile long milliTimeout;
volatile int firstPin;

volatile int lastPin;
const int TIMEOUT = 30000;

void setup()
{

  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);

  //pinMode(pin, OUTPUT);
  Serial.begin(57600);
  attachInterrupt(2, two, RISING);
  attachInterrupt(3, three, RISING);
  attachInterrupt(4, four, RISING);
  attachInterrupt(5, five, RISING);
  attachInterrupt(6, six, RISING);
  attachInterrupt(7, seven, RISING);
}

void loop()
{
  //digitalWrite(pin, state);
  if(milliTimeout > 0)
  {
    milliTimeout -= 1;
  }

  if(firstPin != 0)
  {
    Serial.println (firstPin);
    firstPin = 0;
  }

}

void seven()
{
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
    if(lastPin != 7)
    {
      firstPin = 7;
      lastPin = 7;
    }
  }
}

void two()
{ 
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
    if(lastPin != 2)
    {
      firstPin = 2;
      lastPin = 2;
    }
  }
}

void three()
{ 
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
    if(lastPin != 3)
    {
      firstPin = 3;
      lastPin = 3;
    }
  }
}

void four()
{  
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
    if(lastPin != 4)
    {
      firstPin = 4;
      lastPin = 4;
    }
  }
}

void five()
{
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
    if(lastPin != 5)
    {
      firstPin = 5;
      lastPin = 5;
    }
  }
}

void six()
{  
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
    if(lastPin != 6)
    {
      firstPin = 6;
      lastPin = 6;
    }
  }
}



