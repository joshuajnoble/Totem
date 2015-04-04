#include <Adafruit_NeoPixel.h>


#define PIN 23
#define PINS_IN_STRIP 45

volatile long milliTimeout;
volatile int firstPin;

volatile int lastPin;
const int TIMEOUT = 50000;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PINS_IN_STRIP, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define LET_ME_TALK 1
#define REMOTE_GAZE 2

enum ANIMATION_TIMING {
  NONE,
  ROTATE_TO_GAZE,
  FLASH_TO_NOTICE_INC,
  FLASH_TO_NOTICE_BRIGHT,
  FLASH_TO_NOTICE_DEC,
  FLASH_TO_NOTICE_DARK
};

int flashToNoticeCount = 0;
int flashToNoticeBrightness = 0;
int flashToNoticeCycles = 0;

int rotateToCounter = 0;
int showRotationCounter = 0;
int targetRotationPoint = 0;
int currentRotationPoint = 0;

int mode = 0;
int scrollDirection = 1;


ANIMATION_TIMING animationState = NONE;

void setup() {
  Serial.begin(115200);
  Serial.println(" OK " );

  pinMode(2, OUTPUT);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  //interrupts for mic directional
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);

  attachInterrupt(2, two, RISING);
  attachInterrupt(3, three, RISING);
  attachInterrupt(4, four, RISING);
  attachInterrupt(5, five, RISING);
  attachInterrupt(6, six, RISING);
  attachInterrupt(7, seven, RISING);
}



void loop() {

  // are we getting anything?
  while( Serial.available() > 1 )
  {

    int type = Serial.read();
    
    // show let me talk
    if(type == '1')
    {
      mode = LET_ME_TALK;
      animationState = FLASH_TO_NOTICE_INC;
      flashToNoticeCycles = 0;
    }
    
    // set the rotation
    if(type == '2')
    {
      mode = REMOTE_GAZE;
      animationState = ROTATE_TO_GAZE;
      int tmpRotationPoint = int(Serial.parseInt());
      
      if(tmpRotationPoint < 45 && tmpRotationPoint > 0)
      {
        targetRotationPoint = tmpRotationPoint;
        showRotationCounter = 0;
        //t 10, c 45
        if(targetRotationPoint - currentRotationPoint > 0)
        {
          if(currentRotationPoint + (PINS_IN_STRIP - targetRotationPoint) > (targetRotationPoint - currentRotationPoint)) {
            scrollDirection = 1;
          } else {
            scrollDirection = -1;
          }
        }
        else
        {
          // (45 - 10) + targetRotationPoint > abs(10-45)
          if(PINS_IN_STRIP - currentRotationPoint + targetRotationPoint < (currentRotationPoint - targetRotationPoint)) {
            scrollDirection = 1;
          } else {
            scrollDirection = -1;
          }
        }
      }
      //end
    }
  }

  switch(animationState)
  {
  case NONE:
    break;

  case ROTATE_TO_GAZE:
    rotateToCounter++;

    if( currentRotationPoint == targetRotationPoint && showRotationCounter < 1000 )
    {
      Serial.println("showRotationCounter");
      showRotationCounter+=1;
      
      int fadeout = (1000 - showRotationCounter)/4;
      
      strip.setPixelColor( currentRotationPoint - 3, 0, 0, 0 );
      strip.setPixelColor( currentRotationPoint - 2, fadeout/10, fadeout/10, fadeout/10 );      
      strip.setPixelColor( currentRotationPoint - 1, fadeout/2, fadeout/2, fadeout/2 );
      strip.setPixelColor( currentRotationPoint, fadeout, fadeout, fadeout );
      strip.setPixelColor( currentRotationPoint + 1, fadeout/2, fadeout/2, fadeout/2 );
      strip.setPixelColor( currentRotationPoint + 2, fadeout/10, fadeout/10, fadeout/1020 );
      strip.setPixelColor( currentRotationPoint + 3, 0, 0, 0 );

//      setPixelsAround(currentRotationPoint, fadeout, 5);
     
      strip.show();  
      
      if(showRotationCounter == 1000) {
        colorWipe(0);
        showRotationCounter = -1;
        animationState = NONE;
      }
    }
    else
    {
      strip.setPixelColor( currentRotationPoint - 3, 0, 0, 0);     
      strip.setPixelColor( currentRotationPoint - 2, 20, 20, 20 );      
      strip.setPixelColor( currentRotationPoint - 1, 80, 80, 80 );
      strip.setPixelColor( currentRotationPoint, 255, 255, 255 );
      strip.setPixelColor( currentRotationPoint + 1, 80, 80, 80 );
      strip.setPixelColor( currentRotationPoint + 2, 20, 20, 20 );
      strip.setPixelColor( currentRotationPoint + 3, 0, 0, 0 );
//      setPixelsAround(currentRotationPoint, 255, 5 );
      strip.show();
    }

    if(rotateToCounter > 6 && currentRotationPoint != targetRotationPoint )
    {

      rotateToCounter = 0;

      if(currentRotationPoint != targetRotationPoint) {
        currentRotationPoint += scrollDirection;
        
        if(currentRotationPoint < 0) {
          strip.setPixelColor( 0, 0, 0, 0 );
          strip.setPixelColor( 1, 0, 0, 0 );
          strip.setPixelColor( 2, 0, 0, 0 );
          currentRotationPoint = PINS_IN_STRIP;
        }
        
        if(currentRotationPoint > PINS_IN_STRIP) {
          strip.setPixelColor( PINS_IN_STRIP-2, 0, 0, 0 );
          strip.setPixelColor( PINS_IN_STRIP-1, 0, 0, 0 );
          strip.setPixelColor( PINS_IN_STRIP, 0, 0, 0 );
          currentRotationPoint = 0;
        }
        
      }
    }
    
    break;

  case FLASH_TO_NOTICE_INC:
    //Serial.print( " inc " );
    //digitalWrite(2, LOW);
    flashToNoticeCount++;
    if(flashToNoticeCount > 9) {
      flashToNoticeCount = 0;
      flashToNoticeBrightness+=1;
    }
    if(flashToNoticeBrightness > 100) {
      flashToNoticeCount = 0;
      animationState = FLASH_TO_NOTICE_BRIGHT;
    }

    colorWipe( flashToNoticeBrightness );

    break;

  case FLASH_TO_NOTICE_BRIGHT:

    digitalWrite(2, HIGH);
    //Serial.print( " bright " );
    flashToNoticeCount++;
    if(flashToNoticeCount > 2) {
      flashToNoticeCount = 0;
      animationState = FLASH_TO_NOTICE_DEC;
    }

    colorWipe( flashToNoticeBrightness );

    break;

  case FLASH_TO_NOTICE_DEC:
    digitalWrite(2, LOW);
    //Serial.print( " dec " );
    flashToNoticeCount++;
    if(flashToNoticeCount > 4) {
      flashToNoticeCount = 0;
      flashToNoticeBrightness -= 1;
    }
    if(flashToNoticeBrightness < 10) {
      flashToNoticeCount = 0;
      animationState = FLASH_TO_NOTICE_DARK;
    }

    colorWipe( flashToNoticeBrightness );

    break;

    // NOT NEEDED?
  case FLASH_TO_NOTICE_DARK:
    flashToNoticeCount++;
    if(flashToNoticeCount > 300) {
      flashToNoticeCount = 0;
      animationState = FLASH_TO_NOTICE_INC;
      
      flashToNoticeCycles++;
      
      if(flashToNoticeCycles > 3)
      {
        animationState = NONE;
      }
    }
    colorWipe( 0);
    break;

  }
  
  //digitalWrite(pin, state);
  if(milliTimeout > 0)
  {
    milliTimeout -= 1;
  }

  if(firstPin != lastPin && firstPin != 0)
  {
    Serial.println (firstPin);
    lastPin = firstPin; // cache for next time so we don't send again
    firstPin = 0;
  }
  
}

// Fill the dots one after the other with a color
void colorWipe(uint8_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c, c, c);
    ///strip.show();
    //delay(wait);
  }
  strip.show();
}

void setPixelsAround(int currentRotationPoint, int fadeout, int lights)
{

  strip.setPixelColor( currentRotationPoint + lights + 1, 0, 0, 0 );
  strip.setPixelColor( currentRotationPoint - lights - 1, 0, 0, 0 );
  
  for( int i = currentRotationPoint - lights; i < (currentRotationPoint + lights); i++ )
  {
    if( i < 0)
    {
      strip.setPixelColor( 45 - i, fadeout, fadeout, fadeout );
    } 
    else if( i > 45 )
    {
      strip.setPixelColor( i - 45, fadeout, fadeout, fadeout );
    } 
    else 
    {
      strip.setPixelColor( i, fadeout, fadeout, fadeout );
    }
  }
  
}

///////////////////////////////////////////////////////////////////////////////
// interrupts for the directional mic
///////////////////////////////////////////////////////////////////////////////

void seven()
{
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;

      firstPin = 7;

  }
}

void two()
{ 
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
      firstPin = 2;
  }
}

void three()
{ 
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
      firstPin = 3;
  }
}

void four()
{  
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
    firstPin = 4;
  }
}

void five()
{
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
      firstPin = 5;
  }
}

void six()
{  
  if(milliTimeout < 1)
  {
    milliTimeout = TIMEOUT;
      firstPin = 6;
  }
}


