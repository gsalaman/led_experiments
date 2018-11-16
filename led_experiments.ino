/*===========================================================
 * A random collection of LED expirements.
 * 
 * Layering:
 * At the top level, we've defined a collection of pre-set patterns.
 * The user is able to specify which pattern to display, and at which speed.
 * This can be done either via the serial port or via Jeff's cool XBee remote.
 * 
 * One level down from that are the pattern definitions themselves.  Each pattern
 * has an associated init_ and move_ function.  The init_ function sets up the led array;
 * the move_ function is used to manipulate that array to cause "motion".  Note that our main
 * loop will call the appropriate move_ function for the selected pattern. 
 * 
 * The next level down are the user pattern definition functions.  These allow the user to fill
 * a given ring with a given color, create streaks, and create bumps.  There are also motion helpers
 * to rotate leds in either direction.
 * 
 * The next level down are "helper" functions...these do array math to do the associated rotates.
 * 
 * Finally, this is all built on the FastLED library that does the actual LED manipulations.
 */

#include <SoftwareSerial.h>
#include <FastLED.h>


// Hardware definitions for our LED strip.
#define LED_PIN    6
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

#define BRIGHTNESS  30

// Our LED Matrix:
// 16 LEDs in the inner loop, going couter-clockwise.
// 24 LEDs in the outer loop, going clockwise.
// These are helpful defines for where the loops start and end.
#define NUM_LEDS    40
#define INNER_START 0
#define NUM_INNER  16
#define OUTER_START 16
#define NUM_OUTER 24
#define LAST_INNER (NUM_INNER - 1)
#define LAST_OUTER (NUM_LEDS - 1)

// Matrix of LED values, used by FastLED to do the displays.
CRGB leds[NUM_LEDS];

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

// We're using loop_delay to time our patterns...the bigger the delay, the slower the pattern.
#define DEFAULT_LOOP_TIME 60
#define MIN_LOOP_DELAY 10
#define MAX_LOOP_DELAY 150
int loop_delay=DEFAULT_LOOP_TIME;

// These are the pre-defined patterns.  
typedef enum
{
  PATTERN_BLACK,
  PATTERN_TICK,
  PATTERN_SYNC_CLOCKWISE,
  PATTERN_SYNC_COUNTER,
  PATTERN_PULSE,
  PATTERN_OPPOSITES, 
  PATTERN_TEST
} pattern_type;

pattern_type current_pattern;

/*================ HELPER FUNCTIONS =============================================*/

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
 * Function:  draw_streak_helper
 *
 * In some instances, we want to draw a streak spanning the roll-over point.
 * This helper function does that.
 * Note this is heavier weight than the simple "make clockwise/counter-clockwise streak"
 * functions...and it doesn't fill in the background...it *JUST* updates the streak pixels.
 */
void draw_streak_helper( CRGB *ring_start, int ring_size, int streak_start_index, int streak_size, CRGB start_color, CRGB end_color)
{
   CRGB temp_led[NUM_OUTER];
   int  copy_index;
   int  num_copied=0;

   // a given streak may span our roll-over point.  
   // Exammple:  an inner streak of length 4, starting at led index 14 should light up
   // leds 14, 15, 0, and 1...but if we just use fill gradient from 14, it lights up
   // 14, 15, 16, and 17...two from the inner loop and two from the outer.
   //
   // One way to deal with this is to break the streak into two separate gradients..but then you need
   // to do color interpolation.   Instead, I'm going to have a temporary led array that *doesn't* 
   // roll over to do the fill_gradient (which will do the full interpolation for me), and then
   // copy the leds over to the right spots in our ring, dealing with the roll-over.

   // Start with some error checks...
   if (streak_size > NUM_OUTER) streak_size = NUM_OUTER;

   // make our gradient in the temp array.
   fill_gradient_RGB(temp_led, streak_size, start_color, end_color);

   // start by copying led up to the roll-over point
   copy_index = streak_start_index;
   while ((copy_index < ring_size) && (num_copied < streak_size))
   {
      ring_start[copy_index] = temp_led[num_copied];
      num_copied++;
      copy_index++;
   }

   // from here, we just rolled over...so start from the "zeroth" led.
   copy_index = 0;   

   // ...and copy the rest of the streak
   while (num_copied < streak_size)
   {
      ring_start[copy_index] = temp_led[num_copied];
      num_copied++;
      copy_index++;
   }
  
}

/*================= USER PATTERN DEFINITION FUNCTIONS ======================*/

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
 * Function:  rotate_inner_clockwise
 */
void rotate_inner_clockwise( void )
{
    rotate_down_helper(leds, NUM_INNER);
}  


/*===============================================================================
 * Function:  rotate_inner_counter_clockwise
 */
void rotate_inner_counter_clockwise( void )
{
    rotate_up_helper(leds, NUM_INNER);
}  

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

  // want the bump centered in the inner array, hence the 8
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

  // want the bump centered in the outer array, hence the 28
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
/*===================================================================================
 * Function: draw_inner_clockwise_streak
 * The draw_ functions are different from the make_ functions;
 *   - They don't fill in the background
 *   - You specify the starting location
 *   - They are more processor and memory intensive.
 *  What does this mean?  Use the make_ functions if you can.  If not, use draw_.
 *  
 */
void draw_inner_clockwise_streak(int start_index, int streak_size, CRGB head, CRGB tail)
{
   if (start_index < 0) start_index = 0;
   if (start_index > LAST_INNER) start_index = LAST_INNER;
   if (streak_size > NUM_INNER) streak_size = NUM_INNER;

   draw_streak_helper(leds, NUM_INNER, start_index, streak_size, head, tail);
}

/****=======================  PRE-DEFINED PATTERNS ============================******/
//  These have an init_ function to set up the desired pattern, 
//  and a move_function that will be called in the main loop.

/*********************************************
 * Pattern:  PATTERN_SYNC_CLOCKWISE
 * An inner and outer streak that move clockwise, synchronized.
 */
void init_sync_clockwise( void )
{
  int i;
  make_outer_clockwise_streak(10, CRGB::Green, CRGB::Red);
  make_inner_clockwise_streak(6, CRGB::Green, CRGB::Red); 

  // for those streak sizes, the inner needs to rotate 6 spots to be aligned with the outer.
  for (i=0;i<6;i++) rotate_inner_clockwise();
  
  current_pattern = PATTERN_SYNC_CLOCKWISE; 
}

void move_sync_clockwise( void )
{
  static int phase=0;

  // In order to sync the inner and outer lanes, we need to preserve the 3:2 ratio. 
  // common denominator stuff...that means 6 phases.  
  if (phase % 2 == 0) rotate_outer_clockwise();
  if (phase % 3 == 0) rotate_inner_clockwise();

  phase = phase + 1;
  phase = phase % 6;
}

/*********************************************
 * Pattern:  PATTERN_SYNC_COUNTER
 * An inner and outer streak that move counter-clockwise, synchronized.
 */
void init_sync_counter( void )
{
  int i;
  make_outer_counter_clockwise_streak(12, CRGB::Red, CRGB::Yellow);
  make_inner_counter_clockwise_streak(8, CRGB::Red, CRGB::Yellow); 

  // rotate the inner streak to line up with the outer streak
  for (i=0;i<8;i++) rotate_inner_counter_clockwise();
  
  current_pattern = PATTERN_SYNC_COUNTER;
  
}

void move_sync_counter( void )
{
  static int phase=0;

  // In order to sync the inner and outer lanes, we need to preserve the 3:2 ratio. 
  // common denominator stuff...that means 6 phases.  
  if (phase % 2 == 0) rotate_outer_counter_clockwise();
  if (phase % 3 == 0) rotate_inner_counter_clockwise();

  phase = phase + 1;
  phase = phase % 6;
  
}

/*********************************************
 * Pattern:  PATTERN_BLACK
 * All LEDs go off.  
 * Note that we don't need a move function for this...there's nothing to move.  :)
 */
void blackout( void )
{
  fill_all(CRGB::Black);
  current_pattern = PATTERN_BLACK;
}

/*********************************************
 * Pattern:  PATTERN_PULSE
 * This pattern has all LEDs with the same color, but pulses
 * that color through our pre-defined palette.
 */

void init_pulse( void )
{
  CRGB color;

  color = ColorFromPalette(my_palette, 0);
  fill_all(color);
  current_pattern = PATTERN_PULSE;
}

void move_pulse( void )
{
  static uint8_t index=0;
  CRGB color;
  
  color = ColorFromPalette(my_palette, index);

  fill_all(color);

  index++;
}

/*********************************************
 * Pattern:  PATTERN_OPPOSITES
 * This pattern creates a bump on the outside and a streak on the inside...both 
 * of which move in opposite directions.
 */
void init_opposites( void )
{
    current_pattern = PATTERN_OPPOSITES;
    make_outer_bump(6, CRGB::Blue, CRGB::Yellow);
    make_inner_clockwise_streak(4, CRGB::Blue, CRGB::Yellow);
}

void move_opposites( void )
{
    rotate_inner_clockwise();
    rotate_outer_counter_clockwise();  
}
/*********************************************
 * Pattern:  PATTERN_TICK
 * The outer ring has a streak that moves clockwise.
 * The inner ring has a bump.
 * Whenever the outer ring "touches" the inner bump,
 * it pushes (or "ticks") it over by one.
 */

// To make all this happen, we need to keep track of which "inner index" and
// "outer index" our rings are on. 
int outer_pos;
int inner_pos;

// one other fun thing...if we don't "wait" after pushing the inner ring to push it again,
// our rings end up in sync.  
#define TOUCH_DELAY 3

// indexed by inner position...gives outer index where the leds are
// considered "touching".  Note the 3/2 ratio, so we've got some rounding.
// Note that for this array, it's NOT the absolute LED position...it's the
// relative position around the circle.  Also note that this can be used in
// either the clockwise or counter clockwise direction.
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

void init_tick_pattern( void )
{
  make_outer_clockwise_streak(8, CRGB::Blue, CRGB::Red);
  make_inner_bump(3, CRGB::Blue, CRGB::Red);
  current_pattern = PATTERN_TICK;
  
  inner_pos = 8;
  outer_pos = 7;
  
}

void move_tick_pattern( void )
{
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
}

/********************************************
 * PATTERN:  test
 */
int test_index;

void init_test( void )
{
  fill_all(CRGB::Black);
  test_index = 0;
  current_pattern = PATTERN_TEST;
}

#define TEST_PATTERN_TIME_INCREMENT 500
void move_test( void )
{
  static unsigned long last_update_time=0;
  unsigned long        current_time;
  CRGB                 color;

  if (test_index >= NUM_LEDS) return;

  current_time = millis();
  
  // The test pattern is not affected by the loop delay setting.
  if (current_time > last_update_time + TEST_PATTERN_TIME_INCREMENT)
  {
    // make the "zeroth" led red.
    if ((test_index == INNER_START) || (test_index == OUTER_START))
    {
      color = CRGB::Red;
    }
    
    // make the "first" led around the ring green.  This will show direction.
    else if ((test_index == INNER_START + 1) || (test_index == OUTER_START + 1))
    {
      color = CRGB::Green;
    }

    // all the rest should be blue.
    else
    {
      color = CRGB::Blue;
    }

    leds[test_index] = color;

    last_update_time = current_time;
    test_index++;
    
  }  // if it's time for an update.
  
}  // end of move_test

/*===================  MAIN FUNCTIONS ==============================*/
void move_pattern( void )
{
  switch (current_pattern)
  {
    case PATTERN_TICK:  
      move_tick_pattern();  
    break;

    case PATTERN_SYNC_CLOCKWISE:
      move_sync_clockwise();
    break;
    
    case PATTERN_SYNC_COUNTER:
      move_sync_counter();
    break;

    case PATTERN_PULSE:
      move_pulse();
    break;

    case PATTERN_OPPOSITES:
      move_opposites();
    break;

    case PATTERN_TEST:
      move_test();
    break;

  }
}

void print_help( void )
{
  Serial.println("Commands:");
  Serial.println("+ to speed up");
  Serial.println("- to slow down");
  Serial.println("1 selects tick pattern");
  Serial.println("2 selects clockwise sync");
  Serial.println("3 selects counter-clockwise sync");
  Serial.println("4 pulses colors");
  Serial.println("5 moves in opposite directions");
  Serial.println("6 prints the test pattern");
  Serial.println("0 blacks out display");
}

void user_input( void )
{
  char command;
  if (Serial.available())
  {
    command = Serial.read();

    switch (command)
    {
      case '+':
        // speed up
        if (loop_delay > MIN_LOOP_DELAY) 
        {
          loop_delay = loop_delay - 10;
          Serial.print("delay = ");
          Serial.println(loop_delay);
        }
      break;

      case '-':
        // slow down
        if (loop_delay < MAX_LOOP_DELAY)
        {
          loop_delay = loop_delay + 10;
          Serial.print("delay = ");
          Serial.println(loop_delay);
        }
      break;

      case '1':
        init_tick_pattern();
        Serial.println("Tick pattern chosen");
      break;

      case '2':
        init_sync_clockwise();
        Serial.println("Sync clockwise chosen");
      break;

      case '3':
        init_sync_counter();
        Serial.println("Sync counter-clockwise chosen");
      break;

      case '4':
         init_pulse();
         Serial.println("Pulse mode chosen");
      break;

      case '5':
         init_opposites();
         Serial.println("Opposites mode chosen");
      break;

      case '6':
          init_test();
          Serial.println("Test pattern selected");
      break;
      
      case '0':
         blackout();
         Serial.println("BLACKOUT!!!");
      break;

      case '\n':
        //do nothing with returns
      break;

      default:
        print_help();
        
    }
  }
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

    print_help();
    
    init_tick_pattern();
}

void loop()
{
    user_input();
    move_pattern();
    
    FastLED.show();

    //while (!Serial.available());               //wait for character...
    //while (Serial.available()) Serial.read();  // and clear the buffer and move on...
    FastLED.delay(loop_delay);

}
