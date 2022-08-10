/**
 * @file   draw.c
 * @author alex <alex@localhost>
 * @date   Wed Feb  9 17:55:49 2022
 * 
 * @brief  Модуль отрисовки изображений в видео буфер
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "vector.h"
#include "res.h"
#include "draw.h"
#include "graphics.h"

enum image_type_e {		/**< типы изображений */
  IMAGE_4_A = 0,		/**< 4 бита с прозрачностью */
  IMAGE_FILL = 1,		/**< заполнение постоянным цветом */
  IMAGE_4 = 2,			/**< 4 бит без прозрачности без смещения */
  IMAGE_4_A_PAL = 0x10,		/**< 4 бита с прозрачностью и смещением палитры */
  IMAGE_4_PAL = 0x12,		/**< 4 бита без прозрачности со смещением палитры */
  IMAGE_8_A = 0x14,		/**< 8 бит с прозрачностью */
  IMAGE_8 = 0x16,		/**< 8 бит без прозрачности */
};
  
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
 * @param blit_rec прямоугольник экранных координат, куда будет выведено
 * @param bit4 1 - если 4 бита на точку
 * изображение
 */
void draw_setup(vec_t *origin, image_t *im, int x_flip, rectangle_t *blit_rec, int bit4)
{
  odd_data = 0;
  // сколько по горизонтали не рисуется точек изображения
  image_add = im->maxx - blit_rec->max_x + blit_rec->min_x;
  if (bit4)
    image_add >>= 1;
  int video = SCREEN_WIDTH - 1 - blit_rec->max_x + blit_rec->min_x;
  if (x_flip && im->type != IMAGE_FILL)
    video_add = 2 * SCREEN_WIDTH - video;
  else
    video_add = video;
  num_cols = blit_rec->max_x - blit_rec->min_x + 1;
  num_rows = blit_rec->max_y - blit_rec->min_y + 1;
  int pos = blit_rec->min_y - origin->y;
  if (pos > 0) {
    pos = pos * (im->maxx + 1);
    if (bit4)
      pos >>= 1;
  }
  int posx = blit_rec->min_x - origin->x;
  if (posx > 0) {
    if (bit4 && (posx & 1)) {
      odd_data = 1;
      if (!(num_cols & 1))
	--image_add;
	}
    if (bit4)
      posx >>= 1;
    pos += (x_flip) ? -posx : posx;
  }
  if (x_flip & bit4) {
    int t = im->maxx + origin->x - blit_rec->max_x;
    if (t > 0 && t & 1) {
      odd_data = 1;
      if (!(num_cols & 1))
	pos--;
    }
    pos += (im->maxx + 1 - num_cols) >> 1;
  } else if (x_flip)
    pos += im->maxx + 1 - num_cols;
  blit_src = (byte *)(im + 1) + pos;
  blit_dst = video_buffer + blit_rec->min_y * SCREEN_WIDTH + blit_rec->min_x;
#ifdef DEBUG
  printf("draw setup: rows = %d cols = %d vid_add = %d im_add = %d x_fl = %d src = %d dst = %d odd = %d type = %x\n", num_rows, num_cols, video_add, image_add, x_flip, (int)(blit_src - (byte *)im), (int)(blit_dst - video_buffer), odd_data, im->type);
#endif
  if (x_flip)
    blit_dst += num_cols;
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
 * Отрисовка точки с прозрачностью 
 * Если цвет точки непрозрачный то выводит точку
 * 0 - прозрачный
 * @param c цвет точки
 * @param pal_ofs смещение в палитре
 * @param x_flip зеркальное отражение
 * @param alpha если 1 - то с прозрачностью
 */
void draw_pixel(byte c, int pal_ofs, int x_flip, int alpha)
{
  if (!x_flip) {
    if (c || !alpha)
      *blit_dst++ = c + pal_ofs;
    else
      ++blit_dst;
  } else { // рисуем точки справо - налево
    if (c  || !alpha)
      *(blit_dst - 1) = c + pal_ofs;
    --blit_dst;
  }
}

/** 
 * Рисует изображение 8 бит с прозрачностью.
 * 0 - прозрачный цвет
 * @param x_flip зеркальное отражение
 */
void draw_image_alpha(int x_flip)
{
  for (int y = 0; y < num_rows; y++) {
    for (int x = 0; x < num_cols; x++)
      draw_pixel(*blit_src++, 0, x_flip, 1);
    blit_src += image_add;
    blit_dst += video_add;
  }
}

/** 
 * Рисует изображение 8 бит без прозрачности
 * 
 * @param x_flip отражение по горизонтали
 */
void draw_image8(int x_flip)
{
  for (int y = 0; y < num_rows; y++) {
    if (x_flip)
      for (int x = 0; x < num_cols; x++) {
	*(blit_dst - 1) = *blit_src++;
	--blit_dst;
      }
    else {
      memcpy(blit_dst, blit_src, num_cols);
      blit_dst += num_cols;
      blit_src += num_cols;
    }
    blit_src += image_add;
    blit_dst += video_add;
  }
}

/** 
 * Рисует изображение 4 бит
 * 
 * @param x_flip зеркальное отражение
 * @param pal_ofs смещение в палитре
 * @param alpha если 1 - то с прозрачностью
 */
void draw_image4(int x_flip, int pal_ofs, int alpha)
{
  byte c;
  byte c2;
  int h = odd_data;
  if (h)
    num_cols--;
  for (int y = 0; y < num_rows; y++) {
    if (h) {
      c = *blit_src++;
      draw_pixel(c & 0xf, pal_ofs, x_flip, alpha);
    }
    for (int x = 0; x < num_cols; x++) {
      c = *blit_src++;
      draw_pixel(c >> 4, pal_ofs, x_flip, alpha);
      ++x;
      if (x == num_cols)
	break;
      draw_pixel(c & 0xf, pal_ofs, x_flip, alpha);
    }      
    blit_src += image_add;
    blit_dst += video_add;
  }
}

/** 
 * Отрисовка изображения
 * 
 * @param origin координаты левого верхнего угла спрайта
 * @param im данные изображения
 * @param x_flip 1 - если зеркальное отражение
 * @param blit_rec координатное окно, куда выводится целое изображение или часть
 */
void draw_image(vec_t *origin, image_t *im, int x_flip, rectangle_t *blit_rec)
{
  switch (im->type) {
  case IMAGE_FILL:
    draw_setup(origin, im, x_flip, blit_rec, 0);
    fill_image(im->fill_color);
    break;
  case IMAGE_8_A:
    draw_setup(origin, im, x_flip, blit_rec, 0);
    draw_image_alpha(x_flip);
    break;
  case IMAGE_8:
    draw_setup(origin, im, x_flip, blit_rec, 0);
    draw_image8(x_flip);
    break;
  case IMAGE_4:
    draw_setup(origin, im, x_flip, blit_rec, 1);
    blit_src -= 2;
    draw_image4(x_flip, 0, 0);
    break;
  case IMAGE_4_A:
    draw_setup(origin, im, x_flip, blit_rec, 1);
    blit_src -= 2;
    draw_image4(x_flip, 0, 1);
    break;
  case IMAGE_4_A_PAL:
    draw_setup(origin, im, x_flip, blit_rec, 1);
    draw_image4(x_flip, im->palette_offset, 1);
    break;
  case IMAGE_4_PAL:
    draw_setup(origin, im, x_flip, blit_rec, 1);
    draw_image4(x_flip, im->palette_offset, 0);
    break;
  default:
    printf("Unknown image type: %x\n", im->type);
    exit(1);
  }
}

/** 
 * Отрисовка изображения курсора мыши
 * 
 * @param im исходное изображение курсора
 * @param buf изображение на выходе
 * @param w ширина
 * @param h высота
 */
void draw_cursor(image_t *im, byte *buf, int w, int h)
{
  image_add = 0;
  video_add = 0;
  num_cols = w;
  num_rows = h;
  odd_data = 0;
  blit_src = (byte *)(im + 1);
  blit_dst = buf;
  switch (im->type) {
  case IMAGE_4_A_PAL:
    draw_image4(0, im->palette_offset, 1);
    break;
  default:
    printf("Unknown cursor image type: %x\n", im->type);
    exit(1);
  }
}
