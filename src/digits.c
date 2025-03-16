#include "digits.h"

// Define private
// Size of the digit boxes
#define BOX_X 72
#define BOX_Y 84

// Animation duration and delay between in and out animations
#define ANIM_DURATION 500
#define ANIM_DELAY 300

/**
 * Type for digit position values structure
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

/**
 * All DigitLayer structures contained as a single layer
 */
DigitLayers *digit_layers = NULL;
void animate_digit_layer(DigitLayer *digit_layer);

/**
 * Return the DigitLayer for the given digit value
 * @param digit Digit to return an associated DigitLayer from
 */
DigitLayer *get_digit_layer_for_digit(DIGIT digit)
{
    DigitLayer *digit_layer = NULL;
    switch (digit)
    {
    case HOUR1:
        digit_layer = digit_layers->hour1;
        break;
    case HOUR2:
        digit_layer = digit_layers->hour2;
        break;
    case MINUTE1:
        digit_layer = digit_layers->minute1;
        break;
    case MINUTE2:
        digit_layer = digit_layers->minute2;
        break;
    default:
        break;
    }

    return digit_layer;
}

/**
 * Adds the next appropriate bitmap to the DigitLayer given the current time
 */
void update_digit_layer_bitmap(DigitLayer *digit_layer)
{
    gbitmap_destroy_safe(digit_layer->material.bitmap);
    digit_layer->material.bitmap = gbitmap_create_with_resource(DIGIT_RESOURCE_IDS[digit_layer->value]);
#ifdef PBL_BW
    if (digit_layer->position == 1 || digit_layer->position == 2)
        invert_bitmap(digit_layer->material.bitmap);
#endif
    bitmap_layer_set_bitmap(digit_layer->material.bitmap_layer, digit_layer->material.bitmap);
}

/**
 * Update the value of a digit
 * @param digit Digit to update on the clock
 * @param value Value to update the digit to
 */
void update_digit_value(DIGIT digit, int value)
{
    DigitLayer *digit_layer = get_digit_layer_for_digit(digit);
    if (!digit_layer)
        return;

    digit_layer->value = value;
}

/**
 * Return value for a given digit
 * @param digit Digit to return the value for
 */
int get_digit_value(DIGIT digit)
{
    DigitLayer *digit_layer = get_digit_layer_for_digit(digit);
    if (!digit_layer)
        return -1;

    return digit_layer->value;
}

/**
 * Handles the completion of an animation
 * @param animation Pointer to the Animation that stopped
 * @param finished Whether the animation finished successfully
 * @param context Pointer to the context data. Assumed to be a DigitLayer struct
 */
void anim_stopped_handler(Animation *animation, bool finished, void *context)
{
    if (finished)
    {
        DigitLayer *digit_layer = (DigitLayer *)context;
        if (digit_layer->out_of_frame)
        {
            update_digit_layer_bitmap(digit_layer);
            animate_digit_layer(digit_layer);
        }
    }
}

/**
 * Handles the start of an animation
 * @param animation Pointer to the Animation that stopped
 * @param context Pointer to the context data. Assumed to be a DigitLayer struct
 */
void anim_started_handler(Animation *animation, void *context)
{
    // Flips the out_of_frame flag as soon as the animation starts
    DigitLayer *digit_layer = (DigitLayer *)context;
    digit_layer->out_of_frame = !digit_layer->out_of_frame;
}

/**
 * Creates and schedules an animation for a digit layer
 * @param digit_layer The digit layer to animate
 * @param start Starting position
 * @param finish Ending position
 */
void begin_digit_layer_animation(DigitLayer *digit_layer, GRect start, GRect finish)
{
    // Create and configure animation
    digit_layer->animation = property_animation_create_layer_frame(
        digit_layer->material.parent_layer, &start, &finish);
    Animation *anim = (Animation *)digit_layer->animation;

    animation_set_handlers(anim, (AnimationHandlers){.started = anim_started_handler, .stopped = anim_stopped_handler}, digit_layer);

    // Set common animation properties
    animation_set_duration(anim, ANIM_DURATION);
    animation_set_delay(anim, ANIM_DELAY);
    animation_set_curve(anim, AnimationCurveEaseInOut);
    animation_schedule(anim);
}

/**
 * Run the animation for a given DigitLayer
 * @param digit_layer DigitLayer to run the animation for
 */
void animate_digit_layer(DigitLayer *digit_layer)
{
    // Define positions in and out of frame for each position
    GRect out_of_frame = GRect(
        DIGIT_POSITION_VALUES[digit_layer->position].out_of_frame[0],
        DIGIT_POSITION_VALUES[digit_layer->position].out_of_frame[1],
        BOX_X,
        BOX_Y);
    GRect in_frame = GRect(
        DIGIT_POSITION_VALUES[digit_layer->position].in_frame[0],
        DIGIT_POSITION_VALUES[digit_layer->position].in_frame[1],
        BOX_X,
        BOX_Y);

    GRect start = digit_layer->out_of_frame
                      ? out_of_frame
                      : in_frame;
    GRect finish = digit_layer->out_of_frame
                       ? in_frame
                       : out_of_frame;

    // Animate digit layer
    begin_digit_layer_animation(digit_layer, start, finish);
}

/**
 * Manages the animation sequence for all digits
 * @param digit The digit to animate
 */
void animate_digit(DIGIT digit)
{
    DigitLayer *digit_layer = get_digit_layer_for_digit(digit);
    if (!digit_layer)
        return;

    animate_digit_layer(digit_layer);
}

/**
 * Add all DigitLayers to the given layer
 * @param layer Layer to add DigitLayers to
 */
void add_digit_layers_to_layer(Layer *layer)
{
    DigitLayer *digit_layer_array[4] = {digit_layers->hour1, digit_layers->hour2, digit_layers->minute1, digit_layers->minute2};
    for (int i = 0; i < 4; i++)
    {
        layer_add_to_layer(digit_layer_array[i]->material.parent_layer, layer);
    }
};

/**
 * Initial load of digit layers
 */
void load_digit_layers()
{
    DigitLayer *digit_layer_array[4] = {digit_layers->hour1, digit_layers->hour2, digit_layers->minute1, digit_layers->minute2};
    for (int i = 0; i < 4; i++)
    {
        DigitLayer *digit_layer = digit_layer_array[i];
        digit_layer->position = i;
        digit_layer->out_of_frame = true;

        digit_layer->material.parent_layer = layer_create(GRect(
            DIGIT_POSITION_VALUES[i].out_of_frame[0],
            DIGIT_POSITION_VALUES[i].out_of_frame[1],
            BOX_X, BOX_Y));

        digit_layer->material.bitmap_layer = bitmap_layer_create(GRect(0, 0, BOX_X, BOX_Y));
        update_digit_layer_bitmap(digit_layer);

        bitmap_layer_add_to_layer(digit_layer->material.bitmap_layer, digit_layer->material.parent_layer);
    }
}

/**
 * Unload of digit layers
 */
void unload_digit_layers()
{
    DigitLayer *digit_layer_array[4] = {digit_layers->hour1, digit_layers->hour2, digit_layers->minute1, digit_layers->minute2};
    for (int i = 0; i < 4; i++)
    {
        layer_destroy_safe(digit_layer_array[i]->material.parent_layer);
        bitmap_layer_destroy_safe(digit_layer_array[i]->material.bitmap_layer);
        gbitmap_destroy_safe(digit_layer_array[i]->material.bitmap);
    }
}

/**
 * Initialize all the digit layers
 */
void init_digit_layers()
{
    DigitLayer *digit_layer_hour1 = malloc(sizeof(DigitLayer));
    DigitLayer *digit_layer_hour2 = malloc(sizeof(DigitLayer));
    DigitLayer *digit_layer_minute1 = malloc(sizeof(DigitLayer));
    DigitLayer *digit_layer_minute2 = malloc(sizeof(DigitLayer));

    digit_layers = malloc(sizeof(DigitLayers));
    digit_layers->hour1 = digit_layer_hour1;
    digit_layers->hour2 = digit_layer_hour2;
    digit_layers->minute1 = digit_layer_minute1;
    digit_layers->minute2 = digit_layer_minute2;
}