#include "base.h"

/**
 * An enum representing the digits on a clock
 */
typedef enum
{
    HOUR1,
    HOUR2,
    MINUTE1,
    MINUTE2
} DIGIT;

void animate_digit(DIGIT digit);
void update_digit_value(DIGIT digit, int value);
int get_digit_value(DIGIT digit);
void add_digit_layers_to_layer(Layer *layer);
void load_digit_layers();
void unload_digit_layers();
void init_digit_layers();