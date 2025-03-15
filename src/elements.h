/*
Squared
  Copyright (C) 2015 James Downs
  james.j.downs@icloud.com
*/

#define BOX_X 72
#define BOX_Y 84

#define ANIM_DURATION 500
#define ANIM_DELAY 300

typedef struct
{
  Layer *parent_layer;
  BitmapLayer *bitmap_layer;
  GBitmap *bitmap;
  PropertyAnimation *animation;
  bool out_of_frame;
  int position;
} DigitLayer;

static Window *s_main_window;
static Layer *s_background_layer;
static DigitLayer *s_hour1, *s_hour2, *s_minute1, *s_minute2;
static BitmapLayer *s_material_layer;
static GBitmap *s_material_bitmap;

static AppTimer *timer = NULL;

static int8_t hour;
static int8_t hour1;
static int8_t hour2;
static int8_t minute1;
static int8_t minute2;

static bool idle = true;

const int DIGIT_RESOURCE_IDS[] = {
    RESOURCE_ID_T0,
    RESOURCE_ID_T1,
    RESOURCE_ID_T2,
    RESOURCE_ID_T3,
    RESOURCE_ID_T4,
    RESOURCE_ID_T5,
    RESOURCE_ID_T6,
    RESOURCE_ID_T7,
    RESOURCE_ID_T8,
    RESOURCE_ID_T9};

static void do_animation(DigitLayer *digit_layer);
static void update_bg(Layer *layer, GContext *ctx);