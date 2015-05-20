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

static void update_bg(Layer *layer, GContext *ctx) {
  gbitmap_destroy_safe(s_hour1_bitmap);
  gbitmap_destroy_safe(s_hour2_bitmap);
  gbitmap_destroy_safe(s_minute1_bitmap);
  gbitmap_destroy_safe(s_minute2_bitmap);
  
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  time_t epoch = time(NULL); 
  struct tm *t = localtime(&epoch);
  
  int8_t hour;
  
  if (clock_is_24h_style() == true) {
    hour = t->tm_hour;
  }
  else {
    hour = t->tm_hour % 12;
  }
  
  int8_t hour1 = hour / 10;
  int8_t hour2 = hour - (hour1 * 10);
  int8_t minute1 = t->tm_min / 10;
  int8_t minute2 = t->tm_min - (minute1 * 10);
  
  s_hour1_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[hour1]);
  s_hour2_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[hour2]);
  s_minute1_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[minute1]);
  s_minute2_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[minute2]);
  
  bitmap_layer_set_bitmap(s_hour1_layer, s_hour1_bitmap);
  bitmap_layer_set_bitmap(s_hour2_layer, s_hour2_bitmap);
  bitmap_layer_set_bitmap(s_minute1_layer, s_minute1_bitmap);
  bitmap_layer_set_bitmap(s_minute2_layer, s_minute2_bitmap);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_background_layer);
}

static void main_window_load(Window *window) {
  GRect bounds = window_get_bounds(window);
  
  s_background_layer = layer_create(bounds);
  s_hour1_layer = bitmap_layer_create(GRect(0,0,BOXX,BOXY));
  s_hour2_layer = bitmap_layer_create(GRect(72,0,BOXX,BOXY));
  s_minute1_layer = bitmap_layer_create(GRect(0,84,BOXX,BOXY));
  s_minute2_layer = bitmap_layer_create(GRect(72,84,BOXX,BOXY));

  layer_set_update_proc(s_background_layer, update_bg);
  
  layer_add_to_window(s_background_layer, window);
  
  bitmap_layer_add_to_layer(s_hour1_layer, s_background_layer);
  bitmap_layer_add_to_layer(s_hour2_layer, s_background_layer);
  bitmap_layer_add_to_layer(s_minute1_layer, s_background_layer);
  bitmap_layer_add_to_layer(s_minute2_layer, s_background_layer);
}

static void main_window_unload(Window *window) {
  layer_destroy_safe(s_background_layer);
  
  bitmap_layer_destroy_safe(s_hour1_layer);
  bitmap_layer_destroy_safe(s_hour2_layer);
  bitmap_layer_destroy_safe(s_minute1_layer);
  bitmap_layer_destroy_safe(s_minute2_layer);
  
  gbitmap_destroy_safe(s_hour1_bitmap);
  gbitmap_destroy_safe(s_hour2_bitmap);
  gbitmap_destroy_safe(s_minute1_bitmap);
  gbitmap_destroy_safe(s_minute2_bitmap);
}

static void init() {
  s_main_window = window_create();
  window_handlers(s_main_window, main_window_load, main_window_unload);
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy_safe(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}