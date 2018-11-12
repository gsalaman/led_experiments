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

int outer_tick_ref[NUM_OUTER];

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

/*===============================================================================
 * Function:  setup_outer_tick_ref
 */
void setup_outer_tick_ref( void )
{
    int i;

    for (i = 1; i <= NUM_OUTER; i++)
    {
      outer_tick_ref[i] = 1000 * i / 24;

      //Serial.println(outer_tick_ref[i]);
    } 
}

/*===============================================================================
 * Function:  get_outer_phase
 */
int get_outer_phase( int ms )
{
  int phase = 0;
  
  //make sure ms is 0-999
  ms = ms % 1000;

  while (outer_tick_ref[phase] <= ms)
  {
     phase++;
  }

  phase--;
  
  return phase;
  
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
    make_outer_clockwise_streak(4, blue, red);
    
    // marker for time keeping
    fill_inner(CRGB::Blue);
    leds[0] = CRGB::Red;

    setup_outer_tick_ref();

}


void loop()
{
  static int outer_phase = 0;
  int desired_phase;
  unsigned long current_time;
  int current_ms;  // ms remainder of current time.

  current_time = millis();
  current_ms = current_time % 1000;
  
  desired_phase = get_outer_phase(current_ms);
  while (outer_phase != desired_phase)
  {
    rotate_outer_clockwise();
    outer_phase++;
    outer_phase = outer_phase % NUM_OUTER; 
  }
  
  FastLED.show();
  

}
