#include "types.h"

void graphics_init();
int graphics_update();
void graphics_close();
void graphics_set_palette(byte *palette);

extern byte *video_buffer;
