/**
 * @file   palette.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 17:09:27 2022
 * 
 * @brief  Функции работы с палитрой
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "interpret.h"
#include "get.h"
#include "graphics.h"
#include "image.h"

#pragma pack(1)
/// заголовок палитры в файле
typedef struct {
  byte count;			/**< число хранящихся цветов в палитре */
  byte color_index;		/**< индекс первого цвета для палитры*/
  byte b4;
} palette_t;

bool need_to_update_palette = 0; /**< флаг, что палитру нужно обновить в оборудовании */
int fade_ticks = 0;		/**< текущий кадр появления / угасания */
int palette_fade_ticks = 0;	/**< число кадров между шагами появления / угасания (скорость) */
int fade_offset = 0;		/**< шаг яркости в палитре для появления / угасания */
int palette_parameter;
int skip_palette = 0;
byte palette[768];		/**< текущая палитра */
byte load_palette[768];		/**< загружаемая палитра */

/** 
 * Появление / угасание палитры
 * шаг изменения текущей палитры к палитре назначения
 */
void palette_fade_step()
{
  byte p;
  need_to_update_palette = 0;
  for (int i = 0; i < 768; i++) 
    if (palette[i] != load_palette[i]) {
      need_to_update_palette = 1;
      if (load_palette[i] > palette[i]) {
	p = palette[i] + fade_offset; // fade in
	if (p > load_palette[i])
	  p = load_palette[i];
      } else {
	p = palette[i] - fade_offset; // fade out
	if (p < load_palette[i])
	  p = load_palette[i];
      }
      palette[i] = p;
    }
}

/** 
 * обновление палитры
 * вызывается каждый кадр
 */
void palette_update()
{
  if (need_to_update_palette != 0)
    return;
  graphics_set_palette(palette);
  fade_ticks--;
  if (fade_ticks >= 0)
    return;
  fade_ticks = palette_fade_ticks;
  need_to_update_palette = 0;
  palette_fade_step();
}

/// рассчет парметров появления/угасания
void palette_init_fade(int fade)
{
  fade_offset = 1;
  if (fade && fade >= 63) {
    palette_parameter = 63;
    fade_ticks = palette_fade_ticks = fade / palette_parameter;
  } else if (fade) {
    // рассчет скорости появления / увядания
    fade_offset = 63 / (byte)fade + 1;
    palette_parameter = fade;
    fade_ticks = palette_fade_ticks = 1;
  } else {
    printf("failed\n");
    exit(1);
  }
}

/** 
 * Загружает палитру из файла. prev_value - параметр, если 0, то палитра
 * устанавливается сразу без появления / увядания.
 * палитра устанавливается с заданным количеством цветов (count),
 * начиная заданного цвета (color_index)
 * @param pal адрес палитры
 */
void palette_load(byte *pal)
{
  byte a;
  /*if (skip_palette) {
    skip_palette = 1;
    return;
    }*/
  palette_init_fade(prev_value);
  current_value = prev_value;
  palette_t *p = (palette_t *)pal;
#ifdef DEBUG
  printf("palette load: index = %d num = %d fade = %d\n", p->color_index, p->count, current_value);
#endif
  int ofs = p->color_index * 3;
  byte *src = (byte *)(p + 1);
  byte *dst = load_palette + ofs;
  if (p->count) {
    for (int i = 0; i < p->count; i++)
      for (int j = 0; j < 3; j++)
	*dst++ = *src++ >> 2;
  // если медленная скорость то fade_ticks = 0 param = 0 current = 0
    if (!current_value) {
      // без появления / увядания
      fade_ticks = palette_parameter = palette_fade_ticks = 0;
      memcpy(palette, load_palette, sizeof(palette));
      need_to_update_palette = -1;
      graphics_set_palette(palette);
      return;
    }
  } else {
    src -= 2;
    do {
      a = *src++ << 3;
      if (a)
	a |= 7;
      *dst++ = a;
      a = *src & 0xf0 >> 1;
      if (a)
	a |= 7;
      *dst++ = a;
      a = *src++ & 0xf << 3;
      if (a)
	a |= 7;
      *dst++ = a;
    } while (--p->count);
  }
  palette_fade_step();
  need_to_update_palette = 1;
}

/// начало угасания палитры
void palette_fade_out()
{
  int fade;
  switch_get();
  fade = current_value = prev_value;
  palette_init_fade(fade);
  memset(load_palette, 0, sizeof(load_palette));
  palette_fade_step();
  need_to_update_palette = 1;
  skip_palette = 0;
#ifdef DEBUG
  printf("palette set fade = %d ticks = %d fade_offset = %d param = %d\n", prev_value, palette_fade_ticks, fade_offset, palette_parameter);
  if (prev_value == 0xa) {
    ASSERT(fade_offset, 7)
      ASSERT(palette_parameter, 10)
      ASSERT(fade_ticks, 1)
      ASSERT(palette_fade_ticks, 1)
      ASSERT(need_to_update_palette, 1)
      ASSERT(skip_palette, 0)
  }
#endif
}

/// загрузка палитры по номеру ресурса
void palette_load_from_res()
{
  load_main_image = 0;
  new_get();
  switch_get();
#ifdef DEBUG
  printf("palette load from res: fade = %d  res num= %d\n", prev_value, current_value);
#endif
  if (!skip_palette) {
    if (current_value > 0) {
      byte *pal = get_resource(current_value);
      if (*pal == RES_PALETTE)
	palette_load(pal);
    } else {
      printf("load new palette num = %d\n", current_value);
      exit(1);
    }
  }
  skip_palette = 0;
}
