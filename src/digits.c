#include "digits.h"

// Init extern
DigitLayers *digit_layers = NULL;
const digit_position_t DIGIT_POSITION_VALUES[4] = {
    {{-144, 0}, {0, 0}},   // top-left
    {{72, -168}, {72, 0}}, // top-right
    {{0, 252}, {0, 84}},   // bottom-left
    {{216, 84}, {72, 84}}  // bottom-right
};

/**
 * Adds the next appropriate bitmap to the DigitLayer given the current time
 */
void update_digit_layer_bitmap(DigitLayer *digit_layer)
{
    gbitmap_destroy_safe(digit_layer->material.bitmap);
    digit_layer->material.bitmap = gbitmap_create_with_resource(DIGIT_RESOURCE_IDS[digit_layer->value]);
    bitmap_layer_set_bitmap(digit_layer->material.bitmap_layer, digit_layer->material.bitmap);
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
void animate_digit(DigitLayer *digit_layer, GRect start, GRect finish)
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
 * Manages the animation sequence for all digits
 * @param digit_layer The digit layer to animate
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
    animate_digit(digit_layer, start, finish);
}

/**
 * Initial load of digit layer
 * @param digit_layer The DigitLayer to load
 * @param position The position on the screen to initialize the DigitLayer into. 0 index
 */
void load_digit_layer(DigitLayer *digit_layer, int position)
{
    digit_layer->position = position;
    digit_layer->out_of_frame = true;

    digit_layer->material.parent_layer = layer_create(GRect(
        DIGIT_POSITION_VALUES[position].out_of_frame[0],
        DIGIT_POSITION_VALUES[position].out_of_frame[1],
        BOX_X, BOX_Y));

    digit_layer->material.bitmap_layer = bitmap_layer_create(GRect(0, 0, BOX_X, BOX_Y));
    update_digit_layer_bitmap(digit_layer);
    bitmap_layer_add_to_layer(digit_layer->material.bitmap_layer, digit_layer->material.parent_layer);
}

/**
 * Unload of digit layer
 * @param digit_layer The DigitLayer to unload
 */
void unload_digit_layer(DigitLayer *digit_layer)
{
    layer_destroy_safe(digit_layer->material.parent_layer);
    bitmap_layer_destroy_safe(digit_layer->material.bitmap_layer);
    gbitmap_destroy_safe(digit_layer->material.bitmap);
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