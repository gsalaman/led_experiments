#include <SoftwareSerial.h>
SoftwareSerial XBee(2, 3); // Arduino RX, TX (XBee Dout, Din)

#include <FastLED.h>

#define LED_PIN    6
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    40
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          40

int UPDATES_PER_SECOND = 100;
int colorMode;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

const int ledTestPin1 = 13;
const int ledTestPin2 = 12;
const int ledTestPin3 = 11;
const int ledTestPin4 = 10;

void setup()
{
  // Initialize XBee Software Serial port. Make sure the baud
  // rate matches your XBee setting (9600 is default).
  XBee.begin(9600); 
//  printMenu(); // Print a helpful menu:

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    pinMode(ledTestPin1, OUTPUT);  
    pinMode(ledTestPin2, OUTPUT); 
    pinMode(ledTestPin3, OUTPUT); 
    pinMode(ledTestPin4, OUTPUT); 
    digitalWrite(ledTestPin1, HIGH);
    digitalWrite(ledTestPin2, LOW);
    digitalWrite(ledTestPin3, LOW);
    digitalWrite(ledTestPin4, LOW);      
    colorMode1();  
}

void loop()
{
  // In loop() we continously check to see if a command has been
  //  received.
  if (XBee.available())
  {
    char c = XBee.read();

    if (c == '1') {
      digitalWrite(ledTestPin1, HIGH);
      digitalWrite(ledTestPin2, LOW); 
      digitalWrite(ledTestPin3, LOW); 
      digitalWrite(ledTestPin4, LOW); 
      colorMode1(); // Write analog pin
    
    }
    
    else if (c == '2'){
      digitalWrite(ledTestPin2, HIGH);
      digitalWrite(ledTestPin1, LOW); 
      digitalWrite(ledTestPin3, LOW); 
      digitalWrite(ledTestPin4, LOW); 
      colorMode2(); // Write digital pin
     
    }
 
    else if (c == '3'){
      digitalWrite(ledTestPin3, HIGH);
      digitalWrite(ledTestPin1, LOW); 
      digitalWrite(ledTestPin2, LOW); 
      digitalWrite(ledTestPin4, LOW); 
      colorMode3();  // Read digital pin
     
    }

    else if (c == '4'){
      digitalWrite(ledTestPin4, HIGH);
      digitalWrite(ledTestPin1, LOW); 
      digitalWrite(ledTestPin2, LOW); 
      digitalWrite(ledTestPin3, LOW); 
      colorMode4();  // Read analog pin
    
    }

    else if (c == '5'){
      digitalWrite(ledTestPin1, HIGH); 
      digitalWrite(ledTestPin2, HIGH); 
      digitalWrite(ledTestPin3, HIGH); 
      digitalWrite(ledTestPin4, HIGH);
      colorMode5();  // Read analog pin
    }

    else if (c == '0'){
      digitalWrite(ledTestPin4, LOW);
      digitalWrite(ledTestPin1, LOW); 
      digitalWrite(ledTestPin2, LOW); 
      digitalWrite(ledTestPin3, LOW); 
      BlackPalette();  // Read analog pin
    }  

    else if (c == 'A')
      UPDATES_PER_SECOND = 10;
     else if (c == 'B')
      UPDATES_PER_SECOND = 20; 
    else if (c == 'C')
      UPDATES_PER_SECOND = 30;
     else if (c == 'D')
      UPDATES_PER_SECOND = 40; 
    else if (c == 'E')
      UPDATES_PER_SECOND = 50;
     else if (c == 'F')
      UPDATES_PER_SECOND = 60; 
    else if (c == 'G')
      UPDATES_PER_SECOND = 70;
     else if (c == 'H')
      UPDATES_PER_SECOND = 80; 
    else if (c == 'I')
      UPDATES_PER_SECOND = 90;
     else if (c == 'J')
      UPDATES_PER_SECOND = 100;                   
  }
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);

}

void colorMode1() {
   currentPalette = LavaColors_p;
   currentBlending = LINEARBLEND;
}

void colorMode2() {
   currentPalette = CloudColors_p;
   currentBlending = LINEARBLEND;
}

void colorMode3() {
   currentPalette = ForestColors_p;
   currentBlending = LINEARBLEND; 
}

void colorMode4() {
   currentPalette = RainbowColors_p;
   currentBlending = LINEARBLEND;
}

void colorMode5() {
  SetupTotallyRandomPalette();
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.



// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

void BlackPalette(){
     fill_solid( currentPalette, 16, CRGB::Black); 
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};


