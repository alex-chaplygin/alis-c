#include "types.h"

#define SCREEN_WIDTH 320	/**< размеры буфера экрана */
#define SCREEN_HEIGHT 200

void graphics_init();
int graphics_update();
void graphics_close();
void graphics_set_palette(byte *palette);
void graphics_sleep();
int graphics_get_events();
void graphics_palette_update();

extern byte *video_buffer;
