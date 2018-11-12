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
    fill_solid(&(leds[OUTER_START]), NUM_OUTER, color);
}

/*===============================================================================
 * Function:  fill_all
 */
void fill_all(CRGB color)
{
    fill_solid(leds, NUM_LEDS, color);
}

/*===============================================================================
 * Function:  rotate_down_helper
 *
 * Since the LEDs can be wired arbitrarily (is increasing the index going clockwise
 * or counter-clockwise), I'm gonna make these helpers just go "up" for increasing array
 * and "down" for decreasing array.  We can then map to clockwise and counter-clockwise
 * with the real rotate functions.
 */
void rotate_down_helper( CRGB *start_led, int num )
{
    CRGB roll_over_value;
    int i;
    
    // this is basically just gonna be a big shift with roll-over.
    
    // remember the "0th" value...it's gonna go into the "last" array value.
    roll_over_value = start_led[0];
    
    // now copy everything one slot "down"
    for (i=0; i< num -1; i++)
    {
        start_led[i] = start_led[i+1];
    }
    
    // Finally, store the last LED with that roll-over value.
    start_led[num - 1] = roll_over_value;
    
}  // end of rotate_down_helper

/*===============================================================================
 * Function:  rotate_up_helper
 *
 * Since the LEDs can be wired arbitrarily (is increasing the index going clockwise
 * or counter-clockwise), I'm gonna make these helpers just go "up" for increasing array
 * and "down" for decreasing array.  We can then map to clockwise and counter-clockwise
 * with the real rotate functions.
 */
void rotate_up_helper( CRGB *start_led, int num )
{
    CRGB roll_over_value;
    int i;
    
    // this is basically just gonna be a big shift with roll-over.
    
    // remember the "last" value...it's gonna go into the "first" array value.
    roll_over_value = start_led[num - 1];
    
    // now copy everything one slot "up"
    for (i = num - 1; i > 0; i--)
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
 * Function:  rotate_inner_counter_clockwise
 */
void rotate_inner_counter_clockwise( void )
{
    rotate_up_helper(leds, NUM_INNER);
    
}  // end of rotate_down_helper

/*===============================================================================
 * Function:  rotate_outer_clockwise
 */
void rotate_outer_clockwise( void )
{
    rotate_up_helper(&(leds[OUTER_START]), NUM_OUTER);
    
}  // end of rotate_down_helper
    

/*===============================================================================
 * Function:  rotate_outer_counter_clockwise
 */
void rotate_outer_counter_clockwise( void )
{
    rotate_down_helper(&(leds[OUTER_START]), NUM_OUTER);
    
}  // end of rotate_down_helper

void setup()
{

    Serial.begin(9600);

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    // fill the entire array with blue.
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();

    FastLED.delay(1000);

    leds[0] = CRGB::Red;
    leds[16]= CRGB::Red;
    
    FastLED.delay(1000);

    fill_inner(CRGB::Black);
  
}

#define LOOP_TIME 100
#define PALETTE_JUMP 5
uint8_t brightness=120;

void loop()
{
  static int led_index=OUTER_START;
  static uint8_t color_index = 0;
  static bool inner_toggle=false;
  uint8_t next_color_index;
  
  
  CRGBPalette16 current_palette;

  //current_palette = LavaColors_p;
  //current_palette = CloudColors_p;
  current_palette = ForestColors_p;
  //current_palette = RainbowColors_p;

  leds[led_index] = ColorFromPalette(current_palette, color_index, brightness, LINEARBLEND);
  FastLED.show();
  FastLED.delay(LOOP_TIME);

  led_index++;
  if (led_index == NUM_LEDS) led_index = OUTER_START;

  next_color_index = color_index + PALETTE_JUMP;
  if (next_color_index < color_index)
  {
    // we had a roll-over.  Gonna use that to toggle the inner ring.
    if (inner_toggle)
    {
      fill_inner(CRGB::Black);
    }
    else
    {
      fill_inner(CRGB::Gray);
    }
    
    inner_toggle = !inner_toggle;
  }
  
  color_index = next_color_index;  

}
