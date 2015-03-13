#include <Adafruit_NeoPixel.h>

#define PIN 23

#define PINS_IN_STRIP 16

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

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
  FLASH_TO_NOTICE_DEC
};

int flashToNoticeCount = 0;
int flashToNoticeBrightness = 0;
int flashToNoticeTimer = 0;

int rotateToCounter = 0;

int targetRotationPoint = 0;
int currentRotationPoint = 0;

ANIMATION_TIMING animationState = NONE;

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  Serial.begin(115200);
}



void loop() {
  
  // are we getting anything?
  while( Serial.available() > 1 )
  {
    
    int type = Serial.read();
    if(type == '1')
    {
      mode = LET_ME_TALK;
    }
    if(type == '2')
    {
      mode = REMOTE_GAZE;
      targetRotationPoint = int(Serial.read());
    }    
  }
  
  switch(animationState)
  {
    case NONE:
    break;
    
    case ROTATE_TO_GAZE:
      rotateToCounter++;

      strip.setPixelColor( currentRotationPoint - 2, 122, 122, 122 );      
      strip.setPixelColor( currentRotationPoint - 1, 255, 255, 255 );
      strip.setPixelColor( currentRotationPoint, 255, 255, 255 );
      strip.setPixelColor( currentRotationPoint + 1, 255, 255, 255 );
      strip.setPixelColor( currentRotationPoint + 2, 122, 122, 122 );
      strip.show();
      
      
      // TODO THIS NEEDS TO ACCOUNT FOR COUNTER-CLOCKWISE ROTATION
      if( rotateToCounterm > 200 && currentRotationPoint != targetRotationPoint )
      {
        
        rotateToCounterm = 0;
        
        if(currentRotationPoint < targetRotationPoint) {
         currentRotationPoint++;
        }
        if(currentRotationPoint > targetRotationPoint) {
         currentRotationPoint--;
        }
      }
      
    break;
    
    case FLASH_TO_NOTICE_INC:
      flashToNoticeCount++;
      if(flashToNoticeCount % 1000) {
        flashToNoticeBrightness++;
      }
      if(flashToNoticeBrightness > 254) {
        animationState = FLASH_TO_NOTICE_BRIGHT;
      }
      
      colorWipe( flashToNoticeBrightness );
      
    break;
    
    case FLASH_TO_NOTICE_BRIGHT:
       flashToNoticeCount++;
       if(flashToNoticeCount > 8000000) {
         animationState = FLASH_TO_NOTICE_DEC;
       }
       
       colorWipe( flashToNoticeBrightness );
       
    break;
    
    case FLASH_TO_NOTICE_DEC:
      flashToNoticeCount++;
      if(flashToNoticeCount % 1000) {
        flashToNoticeBrightness--;
      }
      if(flashToNoticeBrightness < 1) {
        animationState = FLASH_TO_NOTICE_INC;
      }
      
      colorWipe( flashToNoticeBrightness );
      
    break;
    
//    // NOT NEEDED?
//    case FLASH_TO_NOTICE_DARK:
//       flashToNoticeCount++;
//       if(flashToNoticeCount > 4000000) {
//         animationState = FLASH_TO_NOTICE_INC;
//       }
//       colorWipe( flashToNoticeBrightness );
//    break;
    
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint8_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c, c, c);
      //strip.show();
      //delay(wait);
  }
  strip.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

