/*
Squared
    Copyright (C) 2015 James Downs
    james.j.downs@icloud.com
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    
*/

#include "pebble.h"
#include "libs/pebble-assist.h"
#include "elements.h"

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  if (finished && again == true) {
    running = false;
    again = false;
    do_reverse = false;
    layer_mark_dirty(s_background_layer);
    do_animation();
    do_hour1 = false;
    do_hour2 = false;
    do_minute1 = false;
    do_minute2 = false;
  }
}

static void do_animation() {
  if (do_reverse) {
    running = true;
  }
  
  GRect hour1_start, hour1_finish;
  GRect hour2_start, hour2_finish;
  GRect minute1_start, minute1_finish;
  GRect minute2_start, minute2_finish;
  
  hour1_start = GRect(-144, 0, BOX_X, BOX_Y);
  hour1_finish = GRect(0, 0, BOX_X, BOX_Y);
  
  hour2_start = GRect(72, -168, BOX_X, BOX_Y);
  hour2_finish = GRect(72, 0, BOX_X, BOX_Y);

  minute1_start = GRect(0, 252, BOX_X, BOX_Y);
  minute1_finish = GRect(0, 84, BOX_X, BOX_Y);

  minute2_start = GRect(216, 84, BOX_X, BOX_Y);
  minute2_finish = GRect(72, 84, BOX_X, BOX_Y);

  if (do_hour1 == true) {
    if (do_reverse == true) {
      s_hour1_animation = property_animation_create_layer_frame(s_hour1_parent, &hour1_finish, &hour1_start);
    }
    else {
      s_hour1_animation = property_animation_create_layer_frame(s_hour1_parent, &hour1_start, &hour1_finish);
    }
    animation_set_duration((Animation*)s_hour1_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_hour1_animation, ANIM_DELAY);
    animation_set_curve((Animation*)s_hour1_animation, AnimationCurveEaseInOut);
    animation_set_handlers((Animation*)s_hour1_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
    animation_schedule((Animation*)s_hour1_animation);
  }
  if (do_hour2 == true) {
    if (do_reverse == true) {
      s_hour2_animation = property_animation_create_layer_frame(s_hour2_parent, &hour2_finish, &hour2_start);
    }
    else {
      s_hour2_animation = property_animation_create_layer_frame(s_hour2_parent, &hour2_start, &hour2_finish);
    }
    animation_set_duration((Animation*)s_hour2_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_hour2_animation, ANIM_DELAY);
    animation_set_curve((Animation*)s_hour2_animation, AnimationCurveEaseInOut);
    animation_set_handlers((Animation*)s_hour2_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
    animation_schedule((Animation*)s_hour2_animation);
  }
  if (do_minute1 == true) {
    if (do_reverse == true) {
      s_minute1_animation = property_animation_create_layer_frame(s_minute1_parent, &minute1_finish, &minute1_start);
    }
    else {
      s_minute1_animation = property_animation_create_layer_frame(s_minute1_parent, &minute1_start, &minute1_finish);
    }
    animation_set_duration((Animation*)s_minute1_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_minute1_animation, ANIM_DELAY);
    animation_set_curve((Animation*)s_minute1_animation, AnimationCurveEaseInOut);
    animation_set_handlers((Animation*)s_minute1_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
    animation_schedule((Animation*)s_minute1_animation);
  }
  if (do_minute2 == true) {
    if (do_reverse == true) {
      s_minute2_animation = property_animation_create_layer_frame(s_minute2_parent, &minute2_finish, &minute2_start);
    }
    else {
      s_minute2_animation = property_animation_create_layer_frame(s_minute2_parent, &minute2_start, &minute2_finish);
    }
    animation_set_duration((Animation*)s_minute2_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_minute2_animation, ANIM_DELAY);
    animation_set_curve((Animation*)s_minute2_animation, AnimationCurveEaseInOut);
    animation_set_handlers((Animation*)s_minute2_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
    animation_schedule((Animation*)s_minute2_animation);
  }
  if (do_reverse == true) {
    again = true;
  }
}

static void update_bg(Layer *layer, GContext *ctx) {
  
  if (running == false) {
    gbitmap_destroy_safe(s_hour1_bitmap);
    gbitmap_destroy_safe(s_hour2_bitmap);
    gbitmap_destroy_safe(s_minute1_bitmap);
    gbitmap_destroy_safe(s_minute2_bitmap);
  
    GRect bounds = layer_get_bounds(layer);
    GPoint center = grect_center_point(&bounds);
  
    s_hour1_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[hour1]);
    s_hour2_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[hour2]);
    s_minute1_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[minute1]);
    s_minute2_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[minute2]);
  
    bitmap_layer_set_bitmap(s_hour1_layer, s_hour1_bitmap);
    bitmap_layer_set_bitmap(s_hour2_layer, s_hour2_bitmap);
    bitmap_layer_set_bitmap(s_minute1_layer, s_minute1_bitmap);
    bitmap_layer_set_bitmap(s_minute2_layer, s_minute2_bitmap);
  }
}

static void update_time() {
  time_t epoch = time(NULL); 
  struct tm *t = localtime(&epoch);
  
  if (clock_is_24h_style() == true) {
    hour = t->tm_hour;
  }
  else {
    hour = t->tm_hour % 12;
  }
  
  if (clock_is_24h_style() == false && hour == 12) {
    hour1 = 1;
    hour2 = 2;
  }
  else {
    hour1 = hour / 10;
    hour2 = hour - (hour1 * 10);
  }
  minute1 = t->tm_min / 10;
  minute2 = t->tm_min - (minute1 * 10);
  
  if (first_run == 3) {
    do_minute2 = true;
    if (minute2 == 0) {
     do_minute1 = true;
     
     if (minute1 == 0) {
      do_hour2 = true;
       
       if (hour2 == 0) {
        do_hour1 = true;
       
        if (clock_is_24h_style() == false && hour2 == 1 && hour1 != 1) {
          do_hour1 = true;
        }
       }
     }
    }
    do_reverse = true;
    if (animations == true) {
      do_animation();
    }
    else {
      layer_mark_dirty(s_background_layer);
      layer_set_update_proc(s_background_layer, update_bg);
    }
  }
  else if (first_run < 3) {
    first_run += 1;
    layer_mark_dirty(s_background_layer);
    layer_set_update_proc(s_background_layer, update_bg);
  }
}

static void timer_callback(void *data) {
  animations = false;
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  animations = true;
  app_timer_cancel(timer);
  timer = app_timer_register(180 * 1000, timer_callback, NULL);
}

static void bt_handler(bool connected) {
  if (connected) {
    vibes_short_pulse();
  }
  else {
    vibes_double_pulse();
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  GRect bounds = window_get_bounds(window);
  
  s_background_layer = layer_create(bounds);
  
  s_hour1_parent = layer_create(GRect(-144,0,BOX_X,BOX_Y));
  s_hour2_parent = layer_create(GRect(72,-168,BOX_X,BOX_Y));
  s_minute1_parent = layer_create(GRect(0,252,BOX_X,BOX_Y));
  s_minute2_parent = layer_create(GRect(216,84,BOX_X,BOX_Y));
  
  s_material_layer = bitmap_layer_create(bounds);
  s_material_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  bitmap_layer_set_bitmap(s_material_layer, s_material_bitmap);
  
  s_hour1_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_hour2_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_minute1_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_minute2_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  
  layer_add_to_window(s_background_layer, window);
  
  bitmap_layer_add_to_layer(s_material_layer, s_background_layer);
  
  layer_add_to_layer(s_hour1_parent, s_background_layer);
  layer_add_to_layer(s_hour2_parent, s_background_layer);
  layer_add_to_layer(s_minute1_parent, s_background_layer);
  layer_add_to_layer(s_minute2_parent, s_background_layer);
  
  bitmap_layer_add_to_layer(s_hour1_layer, s_hour1_parent);
  bitmap_layer_add_to_layer(s_hour2_layer, s_hour2_parent);
  bitmap_layer_add_to_layer(s_minute1_layer, s_minute1_parent);
  bitmap_layer_add_to_layer(s_minute2_layer, s_minute2_parent);
  
  update_time();
}

static void main_window_unload(Window *window) {
  layer_destroy_safe(s_background_layer);
  
  layer_destroy_safe(s_hour1_parent);
  layer_destroy_safe(s_hour2_parent);
  layer_destroy_safe(s_minute1_parent);
  layer_destroy_safe(s_minute2_parent);
  
  bitmap_layer_destroy_safe(s_hour1_layer);
  bitmap_layer_destroy_safe(s_hour2_layer);
  bitmap_layer_destroy_safe(s_minute1_layer);
  bitmap_layer_destroy_safe(s_minute2_layer);
  bitmap_layer_destroy_safe(s_material_layer);
  
  gbitmap_destroy_safe(s_hour1_bitmap);
  gbitmap_destroy_safe(s_hour2_bitmap);
  gbitmap_destroy_safe(s_minute1_bitmap);
  gbitmap_destroy_safe(s_minute2_bitmap);
  gbitmap_destroy_safe(s_material_bitmap);
}

static void init() {
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

static void deinit() {
  animation_unschedule_all();
  accel_tap_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_destroy_safe(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}