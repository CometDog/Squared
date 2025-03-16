#include "pebble.h"
#include "libs/pebble-assist.h"

/**
 * Represents a layer with a bitmap
 */
typedef struct
{
    Layer *parent_layer;
    BitmapLayer *bitmap_layer;
    GBitmap *bitmap;
} MaterialLayer;

void invert_bitmap(GBitmap *bitmap);