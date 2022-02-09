#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "vector.h"
#include "image.h"
#include "graphics.h"

#define IMAGE_4_A 0		/**< 4 бита с прозрачностью */
#define IMAGE_FILL 1		/**< заполнение постоянным цветом */
#define IMAGE_8_A 0x14		/**< 8 бит с прозрачностью */

int image_add;			/**< смещение на следующую строчку изображения */
int video_add;			/**< семещение на следующую строчку для видео буфера */
int num_cols;			/**< число столбцов */
int num_rows;			/**< число строк */
int odd_data;			/**< для 4-х битных изображений 1 если нечетная позиция */
byte *blit_src;			/**< указатель на изображение */
byte *blit_dst;			/**< указатель на видеобуфер */

/** 
 * Настройка отрисовки. Вычисляет число строк и столбцов, 
 * смещения для видео буфера и для изображения, указатели для видео
 * буфера и изображения.
 * @param origin левый верхний угол спрайта
 * @param im данные изображения (заголовок + данные)
 * @param x_flip если 1 - зеркальное отражение
 * @param blit прямоугольник экранных координат, куда будет выведено
 * @param bit4 1 - если 4 бита на точку
 * изображение
 */
void draw_setup(vec_t *origin, image_t *im, int x_flip, rectangle_t *blit, int bit4)
{
  odd_data = 0;
  image_add = im->maxx - blit->max_x + blit->min_x;
  if (bit4)
    image_add >>= 1;
  int video = SCREEN_WIDTH - 1 - blit->max_x + blit->min_x;
  if (x_flip && im->type != IMAGE_FILL)
    video_add = 2 * SCREEN_WIDTH - video;
  else
    video_add = video;
  num_cols = blit->max_x - blit->min_x + 1;
  num_rows = blit->max_y - blit->min_y + 1;
  int pos = blit->min_y - origin->y;
  if (pos > 0)
    pos = pos * (im->maxx + 1);
  int posx = blit->min_x - origin->x;
  if (posx > 0) {
    if (bit4 && posx & 1) {
      odd_data = 1;
      if (!(num_cols & 1))
	--image_add;
    }
    if (bit4)
      posx >>= 1;
    pos += (x_flip) ? -posx : posx;
  }
  if (!x_flip)
    pos += im->maxx + 1 - num_cols;
  blit_src = (byte *)(im + 1) + pos;
  if (bit4)
    blit_src -= 2;
  blit_dst = video_buffer + blit->min_y * SCREEN_WIDTH + blit->min_x;
#ifdef DEBUG
  printf("draw setup: rows = %d cols = %d vid_add = %d im_add = %d x_fl = %d src = %d dst = %d odd = %d type = %x\n", num_rows, num_cols, video_add, image_add, x_flip, (int)(blit_src - (byte *)im), (int)(blit_dst - video_buffer), odd_data, im->type);
#endif
  if (x_flip) {
    printf("xflip: test\n");
    exit(1);
  }
}

/** 
 * Заполняет вычисленную область постоянным цветом
 * 
 * @param color номер цвета
 */
void fill_image(byte color)
{
  for (int y = 0; y < num_rows; y++) {
    memset(blit_dst, color, num_cols);
    blit_dst += num_cols + video_add;
  }
}

/** 
 * Если цвет точки непрозрачный то выводит точку
 * 0 - прозрачный
 * @param c цвет точки
 */
void draw_alpha_pixel(byte c)
{
  if (c)
    *blit_dst++ = c;
  else
    ++blit_dst;
}

/** 
 * Рисует изображение 8 бит с прозрачностью.
 * 0 - прозрачный цвет
 */
void draw_image_alpha()
{
  for (int y = 0; y < num_rows; y++) {
    for (int x = 0; x < num_cols; x++)
      draw_alpha_pixel(*blit_src++);
    blit_src += image_add;
    blit_dst += video_add;
  }
}

void draw_image4_alpha()
{
  byte c;
  byte c2;
  int l = 0;
  int h = odd_data;
  for (int y = 0; y < num_rows; y++) {
    if (h) {
      c = *blit_src++;
      draw_alpha_pixel((c & 0xf) + l);
    }
    for (int x = 0; x < num_cols; x++) {
      c = *blit_src++;
      draw_alpha_pixel((c >> 4) + l);
      ++x;
      draw_alpha_pixel((c & 0xf) + l);
    }      
    blit_src += image_add;
    blit_dst += video_add;
  }
}

void render_image(vec_t *origin, image_t *im, int x_flip, rectangle_t *blit)
{
  switch (im->type) {
  case IMAGE_FILL:
    draw_setup(origin, im, x_flip, blit, 0);
    fill_image(im->fill_color);
    break;
  case IMAGE_8_A:
    draw_setup(origin, im, x_flip, blit, 0);
    draw_image_alpha();
    break;
  case IMAGE_4_A:
    draw_setup(origin, im, x_flip, blit, 1);
    draw_image4_alpha();
    break;
  default:
    printf("Unknown image type: %x\n", im->type);
    exit(1);
  }
}