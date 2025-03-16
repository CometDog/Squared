#include "base.h"

/**
 * Inverts a palettized bitmap in place without creating a new bitmap
 * @param bitmap The reference to the bitmap to invert
 */
void invert_bitmap(GBitmap *bitmap)
{
    if (!bitmap)
        return;

    GBitmapFormat format = gbitmap_get_format(bitmap);
    if (format == GBitmapFormat1Bit)
    {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Unable to invert non-palettized 1-bit bitmaps");
        return;
    }

    uint8_t *data = gbitmap_get_data(bitmap);
    int bytes_per_row = gbitmap_get_bytes_per_row(bitmap);
    GSize size = gbitmap_get_bounds(bitmap).size;
    int total_bytes = bytes_per_row * size.h;
    uint8_t *end = data + total_bytes;

    // Invert all bytes one by one
    while (data < end)
    {
        *data = ~(*data);
        data++;
    }
}