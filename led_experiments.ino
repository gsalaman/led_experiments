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

#define BRIGHTNESS  30

// a bunch of helpful defines
#define INNER_START 0
#define NUM_INNER  16
#define OUTER_START 16
#define NUM_OUTER 24
#define LAST_INNER (NUM_INNER - 1)
#define LAST_OUTER (NUM_LEDS - 1)

//CRGBPalette16 my_palette =
const TProgmemPalette16 my_palette PROGMEM =
{
  CRGB::Blue,
  CRGB::Red,
  CRGB::Yellow,
  CRGB::Blue,

  CRGB::Blue,
  CRGB::Red,
  CRGB::Yellow,
  CRGB::Blue,

  CRGB::Blue,
  CRGB::Red,
  CRGB::Yellow,
  CRGB::Blue,
  
  CRGB::Blue,
  CRGB::Red,
  CRGB::Yellow,
  CRGB::Blue,

};


/*===============================================================================
 * Function:  fill_all
 */
void fill_all(CRGB color)
{
    fill_solid(leds, NUM_LEDS, color);
}

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

/*===============================================================================
 * Function:  fill_with_palette
 * 
 * Fills, using palette, from start_led to stop_led.
 */
void fill_with_palette( int start_led, int stop_led, uint8_t brightness, CRGBPalette16 palette, int palette_start, int palette_step )
{
  int i;
  int palette_index;

  palette_index = palette_start;

  for (i = start_led; i <= stop_led; i++)
  {
    leds[i] = ColorFromPalette(palette, palette_index, brightness, LINEARBLEND);
    palette_index = palette_index + palette_step;
  }

}

/*===============================================================================
 * Function:  make_bump
 * 
 * bump size is the number of leds on either side of the bump led...so bump size of 2
 *   gives a TOTAL LED size of 5...one in the center, and 2 on either side.
 * 
 */
void make_bump(int center_led, int bump_size, CRGB background, CRGB bump)
{
  int start_led_index;
  
  // first cut here will treat rollovers as errors...do nothing.  
  // next cut can do modulo math to make the right thing happen.
  // note I'm also not dealing with the inner/outer loop rollover.
  if (bump_size < 1) return;
  if (center_led - bump_size < 0) return;
  if (center_led + bump_size > NUM_LEDS) return;

  start_led_index = center_led - bump_size;
  fill_gradient_RGB(&(leds[start_led_index]), bump_size + 1, background, bump);
  fill_gradient_RGB(&(leds[center_led]), bump_size+1, bump, background);
  
}

/*===============================================================================
 * Function:  make_inner_bump
 */
void make_inner_bump(int bump_size, CRGB background, CRGB bump)
{
  if (bump_size < 1) bump_size = 1;
  if (bump_size > 7) bump_size = 7;
  
  fill_inner(background);
  make_bump(8, bump_size, background, bump);
}


/*===============================================================================
 * Function:  make_outer_bump
 */
void make_outer_bump(int bump_size, CRGB background, CRGB bump)
{
  if (bump_size < 1) bump_size = 1;
  if (bump_size > 11) bump_size = 11;
  
  fill_outer(background);
  make_bump(28, bump_size, background, bump);
}

/*===============================================================================
 * Function:  make_inner_clockwise_streak
 */
void make_inner_clockwise_streak(int streak_size, CRGB background, CRGB head)
{
  if (streak_size > NUM_INNER) streak_size = NUM_INNER;

  fill_inner(background);

  // inner indexes go counter-clockwise.  
  // we're gonna put the head at index 0, and then fade as array indexes increase
  fill_gradient_RGB(leds, streak_size, head, background); 
}

/*===============================================================================
 * Function:  make_inner_counter_clockwise_streak
 */
void make_inner_counter_clockwise_streak(int streak_size, CRGB background, CRGB head)
{
  if (streak_size > NUM_INNER) streak_size = NUM_INNER;

  fill_inner(background);

  // since inner indexes go counter-clockwise, we need to start at the tail, and build to the head
  fill_gradient_RGB(leds, streak_size, background, head); 
}

/*===============================================================================
 * Function:  make_outer_clockwise_streak
 */
void make_outer_clockwise_streak(int streak_size, CRGB background, CRGB head)
{
  if (streak_size > NUM_OUTER) streak_size = NUM_OUTER;

  fill_outer(background);
  
  // since outer indexes go counter-clockwise, we need to start at the tail, and build to the head
  fill_gradient_RGB(&(leds[OUTER_START]), streak_size, background, head); 

}

/*===============================================================================
 * Function:  make_outer_counter_clockwise_streak
 */
void make_outer_counter_clockwise_streak(int streak_size, CRGB background, CRGB head)
{
  if (streak_size > NUM_OUTER) streak_size = NUM_OUTER;

  fill_outer(background);
  
  // since outer indexes go counter-clockwise, we need to start at the head, and build to the tail
  fill_gradient_RGB(&(leds[OUTER_START]), streak_size, head, background); 
}

void setup()
{

    Serial.begin(9600);
    
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    // clear the array, just in case.
    fill_all(CRGB::Black);
    FastLED.show();

    FastLED.delay(1000);
    
    // fill_with_palette(OUTER_START, LAST_OUTER, 100, my_palette, 0, 2);
    // FastLED.show();
    // FastLed.delay(1000);

    CRGB red = CRGB::Red;
    CRGB blue = CRGB::Blue;
    make_outer_clockwise_streak(8, blue, red);
    make_inner_bump(3, blue, red);
}


// indexed by inner position...gives outer index where the leds are
// considered "touching".  Note the 3/2 ratio, so we've got some rounding.
int align_pos[] = 
{
  0,  // I 0
  2,  // I 1
  3,  // I 2
  5,  // I 3,
  6,  // I 4
  8,  // I 5
  9,  // I 6
  11, // I 7
  12, // I 8
  14, // I 9
  15, // I 10
  17, // I 11
  18, // I 12
  20, // I 13
  21, // I 14
  23  // I 15
};

bool touching(int inner, int outer)
{
  if (align_pos[inner] == outer) return true;
  else return false;    
}

#define LOOP_TIME   50
#define TOUCH_DELAY 3
void loop()
{
    static int outer_pos=7;
    static int inner_pos=8;
    static int touch_delay=0;
    
    rotate_outer_clockwise();
    outer_pos++;
    outer_pos = outer_pos % NUM_OUTER;
 
    if (touch_delay == 0)
    {
      
      if (touching(inner_pos, outer_pos))
      {
        rotate_inner_clockwise();
        inner_pos++;
        inner_pos = inner_pos % NUM_INNER;
        touch_delay = 1;
      }
    }
    else
    {
      touch_delay++;
      if (touch_delay == TOUCH_DELAY) touch_delay = 0;
    }


    FastLED.show();

    //while (!Serial.available());               //wait for character...
    //while (Serial.available()) Serial.read();  // and clear the buffer and move on...
    FastLED.delay(LOOP_TIME);

}
