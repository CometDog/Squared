#include "pebble.h"
#include "libs/pebble-assist.h"
#include "elements.h"

/**
 * Handles the completion of an animation
 * @param animation Pointer to the Animation that stopped
 * @param finished Whether the animation finished successfully
 * @param context Pointer to the context data. Assumed to be a DigitLayer struct
 */
static void anim_stopped_handler(Animation *animation, bool finished, void *context)
{
  if (finished)
  {
    DigitLayer *digit_layer = (DigitLayer *)context;
    layer_mark_dirty(s_background->parent_layer);
    if (digit_layer->out_of_frame)
    {
      do_animation(digit_layer);
    }
  }
}

/**
 * Handles the start of an animation
 * @param animation Pointer to the Animation that stopped
 * @param context Pointer to the context data. Assumed to be a DigitLayer struct
 */
static void anim_started_handler(Animation *animation, void *context)
{
  // Flips the out_of_frame flag as soon as the animation starts
  DigitLayer *digit_layer = (DigitLayer *)context;
  digit_layer->out_of_frame = !digit_layer->out_of_frame;
}

/**
 * Creates and schedules an animation for a digit layer
 * @param digit_layer The digit layer to animate
 * @param start Starting position
 * @param finish Ending position
 */
static void animate_digit(DigitLayer *digit_layer, GRect start, GRect finish)
{
  // Create and configure animation
  digit_layer->animation = property_animation_create_layer_frame(
      digit_layer->material.parent_layer, &start, &finish);
  Animation *anim = (Animation *)digit_layer->animation;

  animation_set_handlers(anim, (AnimationHandlers){.started = anim_started_handler, .stopped = anim_stopped_handler}, digit_layer);

  // Set common animation properties
  animation_set_duration(anim, ANIM_DURATION);
  animation_set_delay(anim, ANIM_DELAY);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_schedule(anim);
}

/**
 * Manages the animation sequence for all digits
 * @param digit_layer The digit layer to animate
 */
static void do_animation(DigitLayer *digit_layer)
{
  // Define positions in and out of frame for each position
  GRect out_of_frame = GRect(
      DIGIT_POSITION_VALUES[digit_layer->position].out_of_frame[0],
      DIGIT_POSITION_VALUES[digit_layer->position].out_of_frame[1],
      BOX_X,
      BOX_Y);
  GRect in_frame = GRect(
      DIGIT_POSITION_VALUES[digit_layer->position].in_frame[0],
      DIGIT_POSITION_VALUES[digit_layer->position].in_frame[1],
      BOX_X,
      BOX_Y);

  GRect start = digit_layer->out_of_frame
                    ? out_of_frame
                    : in_frame;
  GRect finish = digit_layer->out_of_frame
                     ? in_frame
                     : out_of_frame;

  // Animate digit layer
  animate_digit(digit_layer, start, finish);
}

/**
 * Updates the given layer bitmap
 * @param layer The layer to update. Unused
 * @param context The graphics context. Unused
 */
static void update_layer(Layer *layer, GContext *context)
{
  // Prevent updating the layer if any digit is out of frame to avoid replacing digits mid-animation
  bool any_out_of_frame = s_hour1->out_of_frame || s_hour2->out_of_frame || s_minute1->out_of_frame || s_minute2->out_of_frame;
  if (any_out_of_frame)
    return;

  gbitmap_destroy_safe(s_hour1->material.bitmap);
  gbitmap_destroy_safe(s_hour2->material.bitmap);
  gbitmap_destroy_safe(s_minute1->material.bitmap);
  gbitmap_destroy_safe(s_minute2->material.bitmap);

  s_hour1->material.bitmap = gbitmap_create_with_resource(DIGIT_RESOURCE_IDS[clock_digits[0]]);
  s_hour2->material.bitmap = gbitmap_create_with_resource(DIGIT_RESOURCE_IDS[clock_digits[1]]);
  s_minute1->material.bitmap = gbitmap_create_with_resource(DIGIT_RESOURCE_IDS[clock_digits[2]]);
  s_minute2->material.bitmap = gbitmap_create_with_resource(DIGIT_RESOURCE_IDS[clock_digits[3]]);

  bitmap_layer_set_bitmap(s_hour1->material.bitmap_layer, s_hour1->material.bitmap);
  bitmap_layer_set_bitmap(s_hour2->material.bitmap_layer, s_hour2->material.bitmap);
  bitmap_layer_set_bitmap(s_minute1->material.bitmap_layer, s_minute1->material.bitmap);
  bitmap_layer_set_bitmap(s_minute2->material.bitmap_layer, s_minute2->material.bitmap);
}

/**
 * Updates the time and triggers animations
 */
static void update_time()
{
  time_t epoch = time(NULL);
  struct tm *t = localtime(&epoch);

  int hour = t->tm_hour;
  if (!clock_is_24h_style())
  {
    hour %= 12;
    if (hour == 0)
      hour = 12;
  }

  // This relies on divison by assuming rounding down
  clock_digits[0] = hour / 10;
  clock_digits[1] = hour % 10;
  clock_digits[2] = t->tm_min / 10;
  clock_digits[3] = t->tm_min % 10;

  if (idle)
  {
    // When idle, just perform a non-animated update
    layer_mark_dirty(s_background->parent_layer);
  }
  else
  {
    // Last minute always updates
    do_animation(s_minute2);

    // If the ones position of minute is not 0, we have not rolled over yet
    if (clock_digits[3] != 0)
      return;

    do_animation(s_minute1);

    // If the tens position of minute is not 0, we have not rolled over yet
    if (clock_digits[2] != 0)
      return;

    do_animation(s_hour2);

    // If the ones position of hour is not 0, we have not rolled over yet
    if (clock_digits[1] != 0)
      return;

    do_animation(s_hour1);

    // Special case for 12-hour format when rolling from 12 to 1
    if (!clock_is_24h_style() && clock_digits[1] == 1 && clock_digits[0] != 1)
    {
      do_animation(s_hour1);
    }
  }
}

/**
 * Timer callback to set the idle flag
 * @param data Unused
 */
static void timer_callback(void *data)
{
  idle = true;
}

/**
 * Tap handler to reset the idle timer
 * @param axis The axis of the tap. Unused
 * @param direction The direction of the tap. Unused
 */
static void tap_handler(AccelAxisType axis, int32_t direction)
{
  idle = false;
  app_timer_cancel(timer);
  // After 3 minutes of inactivity, set the idle flag to disable animations
  timer = app_timer_register(180 * 1000, timer_callback, NULL);
}

/**
 * Bluetooth connection handler to vibrate on connection status change
 * @param connected Whether the connection is established
 */
static void bt_handler(bool connected)
{
  if (connected)
  {
    vibes_short_pulse();
  }
  else
  {
    vibes_double_pulse();
  }
}

/**
 * Handle time tick event
 * @param tick_time Pointer to the time structure. Unused
 * @param units_changed The units that have changed.
 */
static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  // Update time every minute
  if (units_changed & MINUTE_UNIT)
  {
    update_time();
  }
}

/**
 * Main window load handler
 * @param window The window being loaded
 */
static void main_window_load(Window *window)
{
  GRect bounds = window_get_bounds(window);

  s_background = malloc(sizeof(MaterialLayer));
  DigitLayer *digits[] = {
      s_hour1 = malloc(sizeof(DigitLayer)),
      s_hour2 = malloc(sizeof(DigitLayer)),
      s_minute1 = malloc(sizeof(DigitLayer)),
      s_minute2 = malloc(sizeof(DigitLayer))};

  s_background->parent_layer = layer_create(bounds);
  s_background->bitmap_layer = bitmap_layer_create(bounds);
  s_background->bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  bitmap_layer_set_bitmap(s_background->bitmap_layer, s_background->bitmap);

  layer_add_to_window(s_background->parent_layer, window);
  bitmap_layer_add_to_layer(s_background->bitmap_layer, s_background->parent_layer);

  layer_set_update_proc(s_background->parent_layer, update_layer);
  update_time();

  // Initialize digit layers
  for (int i = 0; i < 4; i++)
  {
    DigitLayer *digit = digits[i];
    digit->position = i;
    digit->out_of_frame = true;

    digit->material.parent_layer = layer_create(GRect(
        DIGIT_POSITION_VALUES[i].out_of_frame[0],
        DIGIT_POSITION_VALUES[i].out_of_frame[1],
        BOX_X, BOX_Y));

    digit->material.bitmap_layer = bitmap_layer_create(GRect(0, 0, BOX_X, BOX_Y));

    layer_add_to_layer(digit->material.parent_layer, s_background->parent_layer);
    bitmap_layer_add_to_layer(digit->material.bitmap_layer, digit->material.parent_layer);

    do_animation(digits[i]);
  }
}

/**
 * Main window unload handler
 * @param window The window being unloaded
 */
static void main_window_unload(Window *window)
{
  DigitLayer *digits[] = {s_hour1, s_hour2, s_minute1, s_minute2};

  // Deinit digit layers
  for (int i = 0; i < 4; i++)
  {
    layer_destroy_safe(digits[i]->material.parent_layer);
    bitmap_layer_destroy_safe(digits[i]->material.bitmap_layer);
    gbitmap_destroy_safe(digits[i]->material.bitmap);
  }

  layer_destroy_safe(s_background->parent_layer);
  bitmap_layer_destroy_safe(s_background->bitmap_layer);
  gbitmap_destroy_safe(s_background->bitmap);
}

/**
 * Initialize the app
 */
static void init()
{
  s_main_window = window_create();
  window_handlers(s_main_window, main_window_load, main_window_unload);
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(tap_handler);
  bluetooth_connection_service_subscribe(bt_handler);

  timer = app_timer_register(180 * 1000, timer_callback, NULL);
}

/**
 * Deinitialize the app
 */
static void deinit()
{
  animation_unschedule_all();
  accel_tap_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_destroy_safe(s_main_window);
}

/**
 * Main entry point
 */
int main(void)
{
  init();
  app_event_loop();
  deinit();
}