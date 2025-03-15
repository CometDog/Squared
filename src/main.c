/*
Squared
    Copyright (C) 2015 James Downs
    james.j.downs@icloud.com
*/

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
    layer_mark_dirty(s_background_layer);
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
      digit_layer->parent_layer, &start, &finish);
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
  // Define start and end positions
  const struct
  {
    GRect out_of_frame;
    GRect in_frame;
  } positions[] = {
      {GRect(-144, 0, BOX_X, BOX_Y), GRect(0, 0, BOX_X, BOX_Y)},   // top-left
      {GRect(72, -168, BOX_X, BOX_Y), GRect(72, 0, BOX_X, BOX_Y)}, // top-right
      {GRect(0, 252, BOX_X, BOX_Y), GRect(0, 84, BOX_X, BOX_Y)},   // bottom-left
      {GRect(216, 84, BOX_X, BOX_Y), GRect(72, 84, BOX_X, BOX_Y)}  // bottom-right
  };

  // Animate digit layer
  animate_digit(digit_layer,
                digit_layer->out_of_frame // Start
                    ? positions[digit_layer->position].out_of_frame
                    : positions[digit_layer->position].in_frame,
                digit_layer->out_of_frame // Finish
                    ? positions[digit_layer->position].in_frame
                    : positions[digit_layer->position].out_of_frame);
}

static void update_bg(Layer *layer, GContext *ctx)
{
  bool any_out_of_frame = s_hour1->out_of_frame || s_hour2->out_of_frame || s_minute1->out_of_frame || s_minute2->out_of_frame;
  if (!any_out_of_frame)
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

  if (idle)
  {
    layer_mark_dirty(s_background_layer);
    layer_set_update_proc(s_background_layer, update_bg);
  }
  else
  {
    do_animation(s_minute2);
    if (minute2 == 0)
    {
      do_animation(s_minute1);
      if (minute1 == 0)
      {
        do_animation(s_hour2);
        if (hour2 == 0)
        {
          do_animation(s_hour1);
          if (clock_is_24h_style() == false && hour2 == 1 && hour1 != 1)
          {
            do_animation(s_hour1);
          }
        }
      }
    }
  }
}

static void timer_callback(void *data)
{
  idle = true;
}

static void tap_handler(AccelAxisType axis, int32_t direction)
{
  idle = false;
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

  s_hour1->position = 0;
  s_hour2->position = 1;
  s_minute1->position = 2;
  s_minute2->position = 3;

  s_hour1->out_of_frame = true;
  s_hour2->out_of_frame = true;
  s_minute1->out_of_frame = true;
  s_minute2->out_of_frame = true;

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

  do_animation(s_hour1);
  do_animation(s_hour2);
  do_animation(s_minute1);
  do_animation(s_minute2);
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