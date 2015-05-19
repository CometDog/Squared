#define BOXX 72
#define BOXY 84

static Window *s_main_window;
static Layer *s_background_layer;
static BitmapLayer *s_hour1_layer, *s_hour2_layer, *s_minute1_layer, *s_minute2_layer;
static GBitmap *s_hour1_bitmap, *s_hour2_bitmap, *s_minute1_bitmap, *s_minute2_bitmap; 

const int TIME_RESOURCE_IDS[] = {
  RESOURCE_ID_ZERO,
  RESOURCE_ID_ONE,
  RESOURCE_ID_TWO,
  RESOURCE_ID_THREE,
  RESOURCE_ID_FOUR,
  RESOURCE_ID_FIVE,
  RESOURCE_ID_SIX,
  RESOURCE_ID_SEVEN,
  RESOURCE_ID_EIGHT,
  RESOURCE_ID_NINE
};