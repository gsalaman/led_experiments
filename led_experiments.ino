/*===========================================================
 * Implementing helper functions.
 */

#include <SoftwareSerial.h>
#include <FastLED.h>

#define LED_PIN    6
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    40
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          40

// a bunch of helpful defines
#define INNER_START 0
#define NUM_INNER  16
#define OUTER_START 16
#define NUM_OUTER 24
#define LAST_INNER (NUM_INNER - 1)
#define LAST_OUTER (NUM_LEDS - 1)

/*===============================================================================
 * Function:  fill_inner
 */
void fill_inner(CRGB color)
{
    fill_solid(leds, NUM_INNER, color);
}

/*===============================================================================
 * Function:  fill_outer
 */
void fill_outer(CRGB color)
{
    // Two ways of doing the pointer math.  I'm gonna try both and see which one 
    // is more efficient....
    fill_solid(&(leds[OUTER_START]), NUM_OUTER, color);
    // fill_solid(leds+OUTER_START*sizeof(CRGB), NUM_OUTER, color);
}

#ifndef ROTATE_DOWN_PTR
/*===============================================================================
 * Function:  rotate_down_helper
 *
 * Since the LEDs can be wired arbitrarily (is increasing the index going clockwise
 * or counter-clockwise), I'm gonna make these helpers just go "up" for increasing array
 * and "down" for decreasing array.  We can then map to clockwise and counter-clockwise
 * with the real rotate functions.
 */
void rotate_down_helper( CRGB *start_led, int NUM_LEDS )
{
    CRGB roll_over_value;
    int i;
    
    // this is basically just gonna be a big shift with roll-over.
    
    // remember the "0th" value...it's gonna go into the "last" array value.
    roll_over_value = start_led[0];
    
    // now copy everything one slot "down"
    for (i=0; i< NUM_LEDS -1; i++)
    {
        start_led[i] = start_led[i+1];
    }
    
    // Finally, store the last LED with that roll-over value.
    start_led[NUM_LEDS - 1] = roll_over_value;
    
}  // end of rotate_down_helper
#else
/*===============================================================================
 * Function:  rotate_down_helper
 *
 * Since the LEDs can be wired arbitrarily (is increasing the index going clockwise
 * or counter-clockwise), I'm gonna make these helpers just go "up" for increasing array
 * and "down" for decreasing array.  We can then map to clockwise and counter-clockwise
 * with the real rotate functions.
 */
void rotate_down_helper( CRGB *start_led, int NUM_LEDS )
{
    CRGB roll_over_value;
    int i;
    
    // this is basically just gonna be a big shift with roll-over.
    
    // remember the "0th" value...it's gonna go into the "last" array value.
    roll_over_value = *start_led;
    
    // now copy everything one slot "down"
    while (i < NUM_LED - 1)
    {
        *start_led = *(start_led+sizeof(CRGB));
        i++;
        start_led++;
    }
    
    // Finally, store the last LED with that roll-over value.
    *start_led = roll_over_value;
    
}  // end of rotate_down_helper
#endif

/*===============================================================================
 * Function:  rotate_up_helper
 *
 * Since the LEDs can be wired arbitrarily (is increasing the index going clockwise
 * or counter-clockwise), I'm gonna make these helpers just go "up" for increasing array
 * and "down" for decreasing array.  We can then map to clockwise and counter-clockwise
 * with the real rotate functions.
 */
void rotate_up_helper( CRGB *start_led, int NUM_LEDS )
{
    CRGB roll_over_value;
    int i;
    
    // this is basically just gonna be a big shift with roll-over.
    
    // remember the "last" value...it's gonna go into the "first" array value.
    roll_over_value = start_led[NUM_LEDS - 1];
    
    // now copy everything one slot "up"
    for (i=NUM_LEDS - 1; i > 0; i--)
    {
        start_led[i] = start_led[i-1];
    }
    
    // Finally, store the first LED with that roll-over value.
    start_led[0] = roll_over_value;
    
}  // end of rotate_down_helper

/*===============================================================================
 * Function:  rotate_inner_clockwise
 */
void rotate_inner_clockwise( void )
{
    rotate_down_helper(leds, NUM_INNER);
    
}  // end of rotate_down_helper


/*===============================================================================
 * Function:  rotate_inner_counter-clockwise
 */
void rotate_inner_counter-clockwise( void )
{
    rotate_up_helper(leds, NUM_INNER);
    
}  // end of rotate_down_helper

/*===============================================================================
 * Function:  rotate_outer_clockwise
 */
void rotate_outer_clockwise( void )
{
    rotate_down_helper(&(leds[OUTER_START]), NUM_OUTER);
    
}  // end of rotate_down_helper
    

/*===============================================================================
 * Function:  rotate_outer_counter-clockwise
 */
void rotate_outer_counter-clockwise( void )
{
    rotate_up_helper(&(leds[OUTER_START]), NUM_OUTER);
    
}  // end of rotate_down_helper

void setup()
{

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    // fill the entire array with blue.
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();

    FastLED.delay(1000);

    leds[0] = CRGB::Red;
    leds[16]= CRGB::Red;
    
    FastLED.delay(1000);

}

#define LOOP_TIME 1000
void loop()
{

    rotate_inner_clockwise();
    rotate_outer_clockwise();
    
    FastLED.show();
    FastLED.delay(LOOP_TIME);
}
