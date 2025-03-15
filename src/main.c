/*
Squared
    Copyright (C) 2015 James Downs
    james.j.downs@icloud.com
*/

#include "pebble.h"
#include "libs/pebble-assist.h"
#include "elements.h"

/**
 * Resets state for a new animation
 * @param animation Pointer to the Animation that stopped
 * @param finished Whether the animation finished successfully
 * @param context Pointer to the context data. Unused
 */
static void anim_stopped_handler(Animation *animation, bool finished, void *context)
{
  if (finished)
  {
    running = false;
    do_reverse = false;
    layer_mark_dirty(s_background_layer);
    do_animation();
    do_hour1 = do_hour2 = do_minute1 = do_minute2 = false;
  }
}

/**
 * Creates and schedules an animation for a digit layer
 * @param digit_layer The digit layer to animate
 * @param start Starting position
 * @param finish Ending position
 */
static void animate_digit(DigitLayer *digit_layer, GRect start, GRect finish)
{
  // Select direction based on do_reverse flag
  GRect from = do_reverse ? finish : start;
  GRect to = do_reverse ? start : finish;

  // Create and configure animation
  digit_layer->animation = property_animation_create_layer_frame(
      digit_layer->parent_layer, &from, &to);
  Animation *anim = (Animation *)digit_layer->animation;

  animation_set_handlers(anim, (AnimationHandlers){.stopped = anim_stopped_handler}, NULL);

  // Set common animation properties
  animation_set_duration(anim, ANIM_DURATION);
  animation_set_delay(anim, ANIM_DELAY);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_schedule(anim);
}

/**
 * Manages the animation sequence for all digits
 */
static void do_animation()
{
  running = do_reverse;

  // Define start and end positions
  const struct
  {
    GRect start;
    GRect finish;
  } positions[] = {
      {GRect(-144, 0, BOX_X, BOX_Y), GRect(0, 0, BOX_X, BOX_Y)},   // hour1
      {GRect(72, -168, BOX_X, BOX_Y), GRect(72, 0, BOX_X, BOX_Y)}, // hour2
      {GRect(0, 252, BOX_X, BOX_Y), GRect(0, 84, BOX_X, BOX_Y)},   // minute1
      {GRect(216, 84, BOX_X, BOX_Y), GRect(72, 84, BOX_X, BOX_Y)}  // minute2
  };

  // Animate digits when necessary
  if (do_hour1)
    animate_digit(s_hour1, positions[0].start, positions[0].finish);
  if (do_hour2)
    animate_digit(s_hour2, positions[1].start, positions[1].finish);
  if (do_minute1)
    animate_digit(s_minute1, positions[2].start, positions[2].finish);
  if (do_minute2)
    animate_digit(s_minute2, positions[3].start, positions[3].finish);
}

static void update_bg(Layer *layer, GContext *ctx)
{

  if (running == false)
  {
    gbitmap_destroy_safe(s_hour1->bitmap);
    gbitmap_destroy_safe(s_hour2->bitmap);
    gbitmap_destroy_safe(s_minute1->bitmap);
    gbitmap_destroy_safe(s_minute2->bitmap);

    GRect bounds = layer_get_bounds(layer);
    GPoint center = grect_center_point(&bounds);

    s_hour1->bitmap = gbitmap_create_with_resource(DIGIT_RESOURCE_IDS[hour1]);
    s_hour2->bitmap = gbitmap_create_with_resource(DIGIT_RESOURCE_IDS[hour2]);
    s_minute1->bitmap = gbitmap_create_with_resource(DIGIT_RESOURCE_IDS[minute1]);
    s_minute2->bitmap = gbitmap_create_with_resource(DIGIT_RESOURCE_IDS[minute2]);

    bitmap_layer_set_bitmap(s_hour1->bitmap_layer, s_hour1->bitmap);
    bitmap_layer_set_bitmap(s_hour2->bitmap_layer, s_hour2->bitmap);
    bitmap_layer_set_bitmap(s_minute1->bitmap_layer, s_minute1->bitmap);
    bitmap_layer_set_bitmap(s_minute2->bitmap_layer, s_minute2->bitmap);
  }
}

static void update_time()
{
  time_t epoch = time(NULL);
  struct tm *t = localtime(&epoch);

  if (clock_is_24h_style() == true)
  {
    hour = t->tm_hour;
  }
  else
  {
    hour = t->tm_hour % 12;
  }

  if (clock_is_24h_style() == false && (hour == 12 || hour == 0))
  {
    hour1 = 1;
    hour2 = 2;
  }
  else
  {
    hour1 = hour / 10;
    hour2 = hour - (hour1 * 10);
  }
  minute1 = t->tm_min / 10;
  minute2 = t->tm_min - (minute1 * 10);

  if (first_run)
  {
    do_minute2 = true;
    if (minute2 == 0)
    {
      do_minute1 = true;

      if (minute1 == 0)
      {
        do_hour2 = true;

        if (hour2 == 0)
        {
          do_hour1 = true;

          if (clock_is_24h_style() == false && hour2 == 1 && hour1 != 1)
          {
            do_hour1 = true;
          }
        }
      }
    }
    do_reverse = true;
    if (animations == true)
    {
      do_animation();
    }
    else
    {
      layer_mark_dirty(s_background_layer);
      layer_set_update_proc(s_background_layer, update_bg);
    }
  }
  else if (!first_run)
  {
    first_run = true;
    layer_mark_dirty(s_background_layer);
    layer_set_update_proc(s_background_layer, update_bg);
  }
}

static void timer_callback(void *data)
{
  animations = false;
}

static void tap_handler(AccelAxisType axis, int32_t direction)
{
  animations = true;
  app_timer_cancel(timer);
  timer = app_timer_register(180 * 1000, timer_callback, NULL);
}

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

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time();
}

static void main_window_load(Window *window)
{
  GRect bounds = window_get_bounds(window);

  s_background_layer = layer_create(bounds);

  s_hour1 = (DigitLayer *)malloc(sizeof(DigitLayer));
  s_hour2 = (DigitLayer *)malloc(sizeof(DigitLayer));
  s_minute1 = (DigitLayer *)malloc(sizeof(DigitLayer));
  s_minute2 = (DigitLayer *)malloc(sizeof(DigitLayer));

  s_hour1->parent_layer = layer_create(GRect(-144, 0, BOX_X, BOX_Y));
  s_hour2->parent_layer = layer_create(GRect(72, -168, BOX_X, BOX_Y));
  s_minute1->parent_layer = layer_create(GRect(0, 252, BOX_X, BOX_Y));
  s_minute2->parent_layer = layer_create(GRect(216, 84, BOX_X, BOX_Y));

  s_material_layer = bitmap_layer_create(bounds);
  s_material_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  bitmap_layer_set_bitmap(s_material_layer, s_material_bitmap);

  s_hour1->bitmap_layer = bitmap_layer_create(GRect(0, 0, BOX_X, BOX_Y));
  s_hour2->bitmap_layer = bitmap_layer_create(GRect(0, 0, BOX_X, BOX_Y));
  s_minute1->bitmap_layer = bitmap_layer_create(GRect(0, 0, BOX_X, BOX_Y));
  s_minute2->bitmap_layer = bitmap_layer_create(GRect(0, 0, BOX_X, BOX_Y));

  layer_add_to_window(s_background_layer, window);

  bitmap_layer_add_to_layer(s_material_layer, s_background_layer);

  layer_add_to_layer(s_hour1->parent_layer, s_background_layer);
  layer_add_to_layer(s_hour2->parent_layer, s_background_layer);
  layer_add_to_layer(s_minute1->parent_layer, s_background_layer);
  layer_add_to_layer(s_minute2->parent_layer, s_background_layer);

  bitmap_layer_add_to_layer(s_hour1->bitmap_layer, s_hour1->parent_layer);
  bitmap_layer_add_to_layer(s_hour2->bitmap_layer, s_hour2->parent_layer);
  bitmap_layer_add_to_layer(s_minute1->bitmap_layer, s_minute1->parent_layer);
  bitmap_layer_add_to_layer(s_minute2->bitmap_layer, s_minute2->parent_layer);

  update_time();
}

static void main_window_unload(Window *window)
{
  layer_destroy_safe(s_background_layer);

  layer_destroy_safe(s_hour1->parent_layer);
  layer_destroy_safe(s_hour2->parent_layer);
  layer_destroy_safe(s_minute1->parent_layer);
  layer_destroy_safe(s_minute2->parent_layer);

  bitmap_layer_destroy_safe(s_hour1->bitmap_layer);
  bitmap_layer_destroy_safe(s_hour2->bitmap_layer);
  bitmap_layer_destroy_safe(s_minute1->bitmap_layer);
  bitmap_layer_destroy_safe(s_minute2->bitmap_layer);
  bitmap_layer_destroy_safe(s_material_layer);

  gbitmap_destroy_safe(s_hour1->bitmap);
  gbitmap_destroy_safe(s_hour2->bitmap);
  gbitmap_destroy_safe(s_minute1->bitmap);
  gbitmap_destroy_safe(s_minute2->bitmap);
  gbitmap_destroy_safe(s_material_bitmap);
}

static void init()
{
  s_main_window = window_create();
  window_handlers(s_main_window, main_window_load, main_window_unload);
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(tap_handler);
  bluetooth_connection_service_subscribe(bt_handler);

  timer = app_timer_register(180 * 1000, timer_callback, NULL);

  do_hour1 = true;
  do_hour2 = true;
  do_minute1 = true;
  do_minute2 = true;
  do_animation();
  do_hour1 = false;
  do_hour2 = false;
  do_minute1 = false;
  do_minute2 = false;
}

static void deinit()
{
  animation_unschedule_all();
  accel_tap_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_destroy_safe(s_main_window);
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}