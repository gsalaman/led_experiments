/*===========================================================
 * Next test sketch.  Bounce two leds around each circle.    
 * Same speed, and since the arrays are set up with 
 * Inner as counter-clockwise, outer clockwise, we'll
 * use that as our default direction.  
 */

#include <SoftwareSerial.h>
#include <FastLED.h>

#define LED_PIN    6
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    40
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          40

void setup()
{

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    // fill the entire array with blue.
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();

    FastLED.delay(1000);

    leds[0] = CRGB::Red;
    leds[16] = CRGB::Red;
    
    FastLED.delay(1000);

}

#define LOOP_TIME      100
#define NUM_INNER_LEDS 16
void loop()
{
    static int current_inner_led = 0;    // Inner leds go from 0 to 15
    static int current_outer_led = NUM_INNER_LEDS;   // outer leds go from 16 to 39
    int next_inner_led;
    int next_outer_led;
    CRGB temp;

    next_inner_led = current_inner_led + 1;
    if (next_inner_led == NUM_INNER_LEDS) next_inner_led = 0;

    temp = leds[next_inner_led];
    leds[next_inner_led] = leds[current_inner_led];
    leds[current_inner_led] = temp;

    current_inner_led = next_inner_led;

 
    next_outer_led = current_outer_led + 1;
    if (next_outer_led == NUM_LEDS) next_outer_led = NUM_INNER_LEDS;

    temp = leds[next_outer_led];
    leds[next_outer_led] = leds[current_outer_led];
    leds[current_outer_led] = temp;


    current_outer_led = next_outer_led;

    
    FastLED.show();
    FastLED.delay(LOOP_TIME);
}
