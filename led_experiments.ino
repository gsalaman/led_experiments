/*===========================================================
 * First test sketch.  Use leds array to set circles to blue, then
 * walk a red light around.  
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
