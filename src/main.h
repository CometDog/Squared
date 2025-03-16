#include "digits.h"

static Window *main_window;
static MaterialLayer *background;

static AppTimer *timer = NULL;

/**
 * Whether the watch has not been interacted with for a while
 */
static bool idle = true;
