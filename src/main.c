#include "main.h"
#include "@pebble-libraries/debug-tick-timer-service/debug-tick-timer-service.h"

/**
 * Timer callback to set the idle flag
 * @param data Unused
 */
static void timer_callback(void *data)
{
  idle = true;
}

/**
 * Register 3 minute timer to set idle status
 */
static void register_idle_timer()
{
  idle = false;
  app_timer_cancel(timer);
  timer = app_timer_register(180 * 1000, timer_callback, NULL);
}

/**
 * Updates the time and triggers animations based on given time
 * @param t tm struct representing the time to update the clock to
 */
static void update_time(struct tm *t)
{
  int hour = t->tm_hour;
  if (!clock_is_24h_style())
  {
    hour %= 12;
    if (hour == 0)
      hour = 12;
  }

  void (*function_to_run)(DIGIT);
  function_to_run = animate_digit;
  if (idle)
  {
    function_to_run = update_digit_bitmap;
  }

  int new_minute_2_value = t->tm_min % 10;
  if (get_digit_value(MINUTE2) != new_minute_2_value)
  {
    update_digit_value(MINUTE2, new_minute_2_value);
    function_to_run(MINUTE2);
  }

  int new_minute_1_value = t->tm_min / 10;
  if (get_digit_value(MINUTE1) != new_minute_1_value)
  {
    update_digit_value(MINUTE1, new_minute_1_value);
    function_to_run(MINUTE1);
  }

  int new_hour_2_value = hour % 10;
  if (get_digit_value(HOUR2) != new_hour_2_value)
  {
    update_digit_value(HOUR2, new_hour_2_value);
    function_to_run(HOUR2);
  }

  int new_hour_1_value = hour / 10;
  if (get_digit_value(HOUR1) != new_hour_1_value)
  {
    update_digit_value(HOUR1, new_hour_1_value);
    function_to_run(HOUR1);
  }

  // Special case for 12-hour format when rolling from 12 to 1
  if (!clock_is_24h_style() && get_digit_value(HOUR2) == 1 && get_digit_value(HOUR1) != 1)
  {
    update_digit_value(HOUR1, 0);
    function_to_run(HOUR1);
  }
}

/**
 * Updates the time and triggers animations based on current time
 */
static void update_time_now()
{
  time_t epoch = time(NULL);
  struct tm *t = localtime(&epoch);
  update_time(t);
}

/**
 * Tap handler to reset the idle timer
 * @param axis The axis of the tap. Unused
 * @param direction The direction of the tap. Unused
 */
static void tap_handler(AccelAxisType axis, int32_t direction)
{
  register_idle_timer();
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
 * @param tick_time Pointer to the time structure
 * @param units_changed The units that have changed.
 */
static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  // Update time every minute
  if (units_changed & MINUTE_UNIT)
  {
    update_time(tick_time);
  }
}

/**
 * Main window load handler
 * @param window The window being loaded
 */
static void main_window_load(Window *window)
{
  GRect bounds = window_get_bounds(window);

  background = malloc(sizeof(MaterialLayer));
  init_digit_layers();

  background->parent_layer = layer_create(bounds);
  background->bitmap_layer = bitmap_layer_create(bounds);
  background->bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  bitmap_layer_set_bitmap(background->bitmap_layer, background->bitmap);

  layer_add_to_window(background->parent_layer, window);
  bitmap_layer_add_to_layer(background->bitmap_layer, background->parent_layer);

  update_time_now();

  // Initialize digit layers
  load_digit_layers();

  add_digit_layers_to_layer(background->parent_layer);

  animate_digit(HOUR1);
  animate_digit(HOUR2);
  animate_digit(MINUTE1);
  animate_digit(MINUTE2);
}

/**
 * Main window unload handler
 * @param window The window being unloaded
 */
static void main_window_unload(Window *window)
{
  // Deinit digit layers
  unload_digit_layers();

  layer_destroy_safe(background->parent_layer);
  bitmap_layer_destroy_safe(background->bitmap_layer);
  gbitmap_destroy_safe(background->bitmap);
}

/**
 * Initialize the app
 */
static void init()
{
  main_window = window_create();
  window_handlers(main_window, main_window_load, main_window_unload);
  window_stack_push(main_window, true);

  debug_tick_timer_service_subscribe(MINUTE_UNIT, tick_handler, REAL);
  accel_tap_service_subscribe(tap_handler);
  bluetooth_connection_service_subscribe(bt_handler);

  register_idle_timer();
}

/**
 * Deinitialize the app
 */
static void deinit()
{
  debug_tick_timer_service_unsubscribe();
  animation_unschedule_all();
  accel_tap_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_destroy_safe(main_window);
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