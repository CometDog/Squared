/*
Squared
    Copyright (C) 2015 James Downs
    james.j.downs@icloud.com
*/

#define BOX_X 72
#define BOX_Y 84

#define ANIM_DURATION 500
#define ANIM_DELAY 300

static Window *s_main_window;
static Layer *s_background_layer;
static Layer *s_hour1_parent, *s_hour2_parent, *s_minute1_parent, *s_minute2_parent;
static BitmapLayer *s_hour1_layer, *s_hour2_layer, *s_minute1_layer, *s_minute2_layer, *s_material_layer;
static GBitmap *s_hour1_bitmap, *s_hour2_bitmap, *s_minute1_bitmap, *s_minute2_bitmap, *s_material_bitmap; 
static PropertyAnimation *s_hour1_animation, *s_hour2_animation, *s_minute1_animation, *s_minute2_animation;

static AppTimer *timer = NULL;

static int8_t hour;
static int8_t hour1;
static int8_t hour2;
static int8_t minute1;
static int8_t minute2;

static int8_t first_run = 1;
  
static bool do_hour1;
static bool do_hour2;
static bool do_minute1;
static bool do_minute2;
static bool do_reverse;
static bool again = false;
static bool running = false;
static bool animations = true;

const int TIME_RESOURCE_IDS[] = {
  RESOURCE_ID_T0,
  RESOURCE_ID_T1,
  RESOURCE_ID_T2,
  RESOURCE_ID_T3,
  RESOURCE_ID_T4,
  RESOURCE_ID_T5,
  RESOURCE_ID_T6,
  RESOURCE_ID_T7,
  RESOURCE_ID_T8,
  RESOURCE_ID_T9
};

static void do_animation();
static void update_bg(Layer *layer, GContext *ctx);