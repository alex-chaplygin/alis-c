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
#include "res.h"

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
int skip_palette = 0;		/**< пропуск загрузки палитры */
int palette_lock = 0;		/**< блокировка изменения палитры */
byte palette[768];		/**< текущая палитра */
byte load_palette[768];		/**< палитра, к которой стремится текущая при появлении / увядании */

/// печать текущей палитры
void dump_palette(byte *palette)
{
  for (int i = 0; i < 768 / 16; i++) {
    for (int j = 0; j < 16; j++)
      printf("%02x ", palette[i * 16 + j]);
    printf("\n");
  }
}

/** 
 * Появление / угасание палитры
 * шаг изменения текущей палитры к палитре назначения
 */
void palette_fade_step()
{
  int p;
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
  if (!need_to_update_palette)
    return;
  if (palette_lock)
    return;
#ifdef DEBUG
  printf("update palette\n");
  printf("load palette:\n");
  dump_palette(load_palette);
  printf("palette:\n");
  dump_palette(palette);
#endif
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
  palette_lock = 1;
  fade_offset = 1;
  if (!fade)
    return;
  if (fade >= 63) {
    palette_parameter = 63;
    fade_ticks = palette_fade_ticks = fade / palette_parameter;
  } else {
    // рассчет скорости появления / увядания
    fade_offset = 63 / (byte)fade + 1;
    palette_parameter = fade;
    fade_ticks = palette_fade_ticks = 1;
  } 
#ifdef DEBUG
  printf("fade = %d fade offset: %d param: %d\n", fade, fade_offset, palette_parameter);
#endif
}

/** 
 * Загрузка 16-ти цветной палитры
 * 
 * @param src данные палитры
 * @param dst куда загружается
 */
void load_palette16(byte *src, byte *dst)
{
#ifdef DEBUG
    printf("palette load: count = 0 16 colors\n");
#endif
    src -= 2;
    for (int i = 0; i < 16; i++) {
      byte b = *src++;
      byte b2 = *src++;
      b <<= 3;
      if (b)
	b |= 7;
      *dst++ = b;
      b = (b2 & 0xf0) >> 1;
      if (b)
	b |= 7;
      *dst++ = b;
      b = (b2 & 0x0f) << 3;
      if (b)
	b |= 7;
      *dst++ = b;
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
  palette_init_fade(prev_value);
  current_value = prev_value;
  palette_t *p = (palette_t *)pal;
#ifdef DEBUG
  printf("palette load: index = %d num = %d fade = %d\n", p->color_index, p->count, current_value);
#endif
  int ofs = p->color_index * 3;
  byte *src = (byte *)(p + 1);
  byte *dst = load_palette + ofs;
  if (p->count == 0)
    load_palette16(src, dst);
  else
  for (int i = 0; i < p->count + 1; i++)
    for (int j = 0; j < 3; j++)
      *dst++ = *src++ >> 2;
  if (!current_value) {
    fade_ticks = palette_parameter = palette_fade_ticks = 0;
    memcpy(palette, load_palette, 768);
    palette_lock = 0;
    graphics_set_palette(palette);
  } else {
    palette_lock = 1;
    palette_fade_step();
    palette_lock = 0;
#ifdef DEBUG
    printf("load palette:\n");
    dump_palette(load_palette);
    printf("palette:\n");
    dump_palette(palette);
#endif
  }
  need_to_update_palette = 1;
  skip_palette = 0;
}

/** 
 * Команда bc
 * Очищает палитру и устанавливает скорость увядания
 */
void palette_clear_fade()
{
  int fade;
  switch_get();
  palette_init_fade(prev_value);
  memset(load_palette, 0, sizeof(load_palette));
  palette_lock = 1;
  palette_fade_step();
  palette_lock = 0;
  need_to_update_palette = 1;
  skip_palette = 0;
#ifdef DEBUG
  printf("palette clear: fade = %d ticks = %d fade_offset = %d param = %d\n", prev_value, palette_fade_ticks, fade_offset, palette_parameter);
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

/** 
 * Команда be
 * Загружает новую палитру из ресурса
 * Устанавливает скорость появления
 */
void palette_set_fade()
{
  load_main_res = 0;
  new_get();
  switch_get();
#ifdef DEBUG
  printf("palette set: fade = %d  res num= %d\n", prev_value, current_value);
#endif
  if (skip_palette) {
    printf("palette set: skip palette = 1\n");
    exit(1);
  }
  if (current_value < 0) {
    printf("palette_set  res_num < 0\n");
    exit(1);
  }
  byte *pal = res_get_image(current_value);
  if (*pal == RES_PALETTE)
    palette_load(pal + 1);
  skip_palette = 0;
}

/** 
 * Команда d7.
 * Установка числа кадров для пропуска палитры
 */
void set_skip_palette()
{
  new_get();
  skip_palette = (byte)current_value;
#ifdef DEBUG
  printf("set skip_palette = %d\n", skip_palette);
#endif
}

/** 
 * Команда: 68
 * Устанавливает палитру по номеру ресурса без появления/увядания
 */
void set_palette_from_res()
{
  load_main_res = 0;
  new_get();
  if (current_value == -2 || current_value == -1) {
    fade_ticks = palette_fade_ticks = palette_parameter = 0;
    memset(load_palette, 0, 768);
    memset(palette, 0, 768);
    graphics_set_palette(palette);
    return;
  } else if (current_value < 0) {
    printf("set palette from res < 0 %d\n", current_value);
    exit(1);
  }
#ifdef DEBUG
  printf("set palette from res: %x\n", current_value);
#endif
  if (!skip_palette) {
    byte *pal = res_get_image(current_value);
    if (*pal == RES_PALETTE) {
      fade_ticks = palette_fade_ticks = palette_parameter = prev_value = 0;
      palette_load(pal + 1);
    }
  }
  skip_palette = 0;
}
