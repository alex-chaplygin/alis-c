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
void graphics_set_cursor(byte *img, int w, int h);
void graphics_read_buffer(int x, int y, int w, int h, byte *buf, int *neww, int *newh);
void graphics_write_buffer(int x, int y, int w, int h, byte *buf);

extern byte *video_buffer;
extern int mouse_x;			/**< состояние мыши */
extern int mouse_y;
extern int mouse_buttons;
