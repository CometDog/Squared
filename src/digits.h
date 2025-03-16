#include "base.h"

// Size of the digit boxes
#define BOX_X 72
#define BOX_Y 84

// Animation duration and delay between in and out animations
#define ANIM_DURATION 500
#define ANIM_DELAY 300

/**
 * Type for digit position values structure
 */
typedef struct
{
    int out_of_frame[2];
    int in_frame[2];
} digit_position_t;

/**
 * Type for array of positional values of digits
 */
extern const digit_position_t DIGIT_POSITION_VALUES[4];

/**
 * Represents a digit layer with a bitmap, animation, and positional props
 */
typedef struct
{
    MaterialLayer material;
    PropertyAnimation *animation;
    bool out_of_frame;
    int position;
    int value;
} DigitLayer;

/**
 * Represents all of the DigitLayers available on the clock
 */
typedef struct
{
    DigitLayer *hour1;
    DigitLayer *hour2;
    DigitLayer *minute1;
    DigitLayer *minute2;
} DigitLayers;

/**
 * All DigitLayers on the clock
 */
extern DigitLayers *digit_layers;

/**
 * Resource IDs for each digit image
 */
static const int DIGIT_RESOURCE_IDS[] = {
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

void animate_digit_layer(DigitLayer *digit_layer);
void load_digit_layer(DigitLayer *digit_layer, int position);
void unload_digit_layer(DigitLayer *digit_layer);
void init_digit_layers();