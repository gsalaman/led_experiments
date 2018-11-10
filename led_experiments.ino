#include <SoftwareSerial.h>

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
    
    // fill the entire array with blue.
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();

    FastLED.delay(1000);

    leds[0] = CRGB::Red;
    
    FastLED.delay(1000);

}

#define LOOP_TIME 1000
void loop()
{
    static int current_led=0;
    int next_led;
    CRGB temp;

    // This loop is going to walk the an LED around the circle.  
    next_led = current_led + 1;
    if (next_led == NUM_LEDS) next_led = 0;

    temp = leds[next_led];
    leds[next_led] = leds[current_led];
    leds[current_led] = temp;

    current_led++;
    if (current_led == NUM_LEDS) current_led = 0;
    
    FastLED.show();
    FastLED.delay(LOOP_TIME);
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
