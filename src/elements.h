// Size of the digit boxes
#define BOX_X 72
#define BOX_Y 84

// Animation duration and delay between in and out animations
#define ANIM_DURATION 500
#define ANIM_DELAY 300

/**
 * Represents a layer with a bitmap
 */
typedef struct
{
  Layer *parent_layer;
  BitmapLayer *bitmap_layer;
  GBitmap *bitmap;
} MaterialLayer;

/**
 * Represents a digit layer with a bitmap, animation, and positional props
 */
typedef struct
{
  MaterialLayer material;
  PropertyAnimation *animation;
  bool out_of_frame;
  int position;
} DigitLayer;

/**
 * Represents all of the DigitLayers available on the watch face
 */
typedef struct
{
  DigitLayer *hour1;
  DigitLayer *hour2;
  DigitLayer *minute1;
  DigitLayer *minute2;
} DigitLayers;

static Window *s_main_window;
static MaterialLayer *s_background;
static DigitLayers *digit_layers;

static AppTimer *timer = NULL;

/**
 * Represents the digits of a clock
 */
static int clock_digits[4];

/**
 * Whether the watch has not been interacted with for a while
 */
static bool idle = true;

/**
 * Resource IDs for each digit image
 */
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

/**
 * Values for the position of each digit when in and out of frame
 */
const struct
{
  int out_of_frame[2];
  int in_frame[2];
} DIGIT_POSITION_VALUES[4] = {
    {{-144, 0}, {0, 0}},   // top-left
    {{72, -168}, {72, 0}}, // top-right
    {{0, 252}, {0, 84}},   // bottom-left
    {{216, 84}, {72, 84}}  // bottom-right
};

static void do_animation(DigitLayer *digit_layer);
static void update_layer(Layer *layer, GContext *context);