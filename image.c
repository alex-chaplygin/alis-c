/**
 * @file   image.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 16:05:03 2022
 * 
 * @brief  Интерпретатор - работа с ресурсами, загрузка изображений
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "get.h"
#include "palette.h"
#include "image.h"

#pragma pack(1)

/// таблица ресурсов
typedef struct {
  dword image_table;		/**< смещение таблицы изображений */
  word image_count;		/**< число изображений */
  dword anim_table;
  word anim_count;
  dword sound_table;		/**< смещение таблицы звуков */
  word sound_count;		/**< число звуков */
} resource_table_t;

/// спрайт в составе объекта
typedef struct {
  word num;			/**< номер спрайта */
  short ofs_x;			/**< смещение спрайта по x*/
  short ofs_y;			/**< y */
  short ofs_z;			/**< z */
} subimage_t;

int load_main_res = 0;	/**< флаг загрузки ресурса из главного потока */
int image_flag = 0;

/// получение данных изображения из таблицы ресурсов
byte *get_resource(int num)
{
  byte *script = run_thread->script;
  if (load_main_res)
    script = main_thread->script;
  script_t *h = (script_t *)script;
  resource_table_t *r = (resource_table_t *)(script + h->resources);
#ifdef DEBUG
    printf("get_resource: main %d num %d\n", load_main_res, num);
#endif
  if (num >= r->image_count) {
    printf("get_resource: main %d num %d > total %d\n", load_main_res, num, r->image_count);
    exit(1);
  }
  byte *pos = (byte *)r + r->image_table + num * sizeof(dword);
  return pos + *(dword *)pos;
}

/** 
 * Удаляет спрайты с заданным тегом, если у них состояние - готов
 * Используется для изменения отдельных спрайтов (не объектов)
 * когда новый спрайт добавляется в список, а старый - удаляется
 * @param tag 
 */
void delete_sprites(int tag)
{
  sprite_t *c;
  sprite_t *c2;
  int found = sprite_find(tag, &c);
  if (found)
    while (c) {
#ifdef DEBUG
      printf("delete_sprites: check sprite center(%d %d %d)\n", c->center.x, c->center.y, c->center.z);
#endif
      if (c->state == SPRITE_READY) {
	c = sprite_remove(c, 0);
	if (!c)
	  break;
	if (run_thread->current_scene != c->scene)
	  break;
	if (tag != c->tag)
	  break;
      } else {
	found = sprite_next_on_tag(c, tag, &c2);
	c = c2;
	if (!found)
	  break;
      }
    }
}

/** 
 * Добавление нового спрайта в список спрайтов текущей сцены
 * Новый спрайт группируется по тегу с другими спрайтами одного объекта.
 * Если объект с заданным тегом уже был, то он меняется на новую позицию
 * или у него меняется изображение (в этом случае ставится состояние - 
 * обновленный)
 * @param num номер изображения
 * @param origin координаты центра спрайта
 * @param x_flip отражение по горизонтали
 * @param is_object 1 - для объекта, 0 - спрайт
 */
void add_sprite(int num, vec_t *origin, int x_flip, int is_object, int tag)
{
  sprite_t *c;
  sprite_t *c2;
  int found;
#ifdef DEBUG
  printf("add sprite %d (%d, %d, %d) xflip = %d num = %d tag = %d\n", is_object, origin->x, origin->y, origin->z, x_flip, num, tag); 
#endif
  if (image_flag) {
    printf("image flag = 1\n");
    exit(1);
  }
  // ищем объект
  found = sprite_find(tag, &c);
  // если не найден, то добавляем
  if (!found)
    sprite_new_insert(c, tag, get_resource(num), x_flip, origin);
  else {
    while(c) {
      if (c->state == SPRITE_READY) {
	printf("add sprite: update sprite\n");
	// обновить спрайт
	c->state = SPRITE_UPDATED;
	sprite_set(c, get_resource(num), x_flip, origin);
	goto end;
      }
      found = sprite_next_on_tag(c, tag, &c2);
      c = c2;
      if (!found)
	break;
    }
    sprite_new_insert(c, tag, get_resource(num), x_flip, origin);
  }
 end:
  if (!is_object)
    delete_sprites(tag);
  image_flag = 0;
}

/** 
 * Загрузка объекта.
 * Состоит из одного или более спрайтов
 *
 * @param img данные объекта
 * @param coord координаты центра объекта
 * @param x_flip зеркальное отражение
 * @param tag тег объекта
 */
void load_object(byte *img, vec_t *coord, int x_flip, int tag)
{
  vec_t vec;
  int x_fl, num;
  int num_sub = *(img + 1);
  subimage_t *sub = (subimage_t *)(img + 2);
  for (int i = 0; i < num_sub; i++) {
    x_fl = x_flip;
    if (!x_fl)
      vec.x = coord->x + sub->ofs_x;
    else
      vec.x = coord->x - sub->ofs_x;
    vec.y = coord->y + sub->ofs_y;
    vec.z = coord->z + sub->ofs_z;
    num = sub->num;
    if ((short)num < 0) {
      num &= 0x7fff;
      x_fl ^= 1;
    }
#ifdef DEBUG
    printf("offset = (%d %d %d) res_num = %d flip = %d\n", sub->ofs_x, sub->ofs_y, sub->ofs_z, num, x_fl);
#endif
    add_sprite(num, &vec, x_fl, 1, tag);
    sub++;
  }
  delete_sprites(tag);
  image_flag = 0;
}

/** 
 * Добавление изображения
 * Обработка палитры и составных изображений
 * при загрузке палитры prev_value - скорость появления, 0 - без появления* 
 * @param coord центр изображения 
 * @param x_flip зеркальное отражение
 * @param tag тег
 */
void load_resource(vec_t *coord, int x_flip, int tag)
{
  byte *img = get_resource(current_value);
  if (*img == RES_PALETTE) {
    printf("loading palette\n");
    exit(1);
    palette_load(img + 1);
  } else if (*img == RES_OBJECT)
    load_object(img, coord, x_flip, tag);
  else 
    add_sprite(current_value, coord, x_flip, 0, tag);
}

/** 
 * Показать изображение
 * 
 * @param x_flip - если 1, то зеркальное отражение по вертикали
 */
void show_object_with_flip(int x_flip)
{
  vec_t coord;
  new_get();
  coord.x = current_value;
  new_get();
  coord.y = current_value;
  new_get();
  coord.z = current_value;
  new_get();
  switch_get();
  int tag = prev_value;
#ifdef DEBUG
  printf("show object (%d, %d, %d) xflip = %d res_num = %d tag = %d\n", coord.x, coord.y, coord.z, x_flip, current_value, tag); 
#endif
  load_resource(&coord, x_flip, tag);
}

/// показать изображение по координатам центра.
void show_object()
{
  load_main_res = 0;
  show_object_with_flip(run_thread->x_flip);
}

/// показать изображение, отраженное по горизонтали
void show_object_flipped()
{
  load_main_res = 0;
#ifdef DEBUG
  printf("show object flipped\n");
#endif
  show_object_with_flip(run_thread->x_flip ^ 1);  
}

void set_tag()
{
  new_get();
#ifdef DEBUG
  printf("set tag %x\n", current_value);
#endif
}

/// Удаляет все объекты текущего потока
void clear_all_objects()
{
  #ifdef DEBUG
  printf("clear all objects\n");
  #endif
  remove_all_sprites(run_thread->sprite_list, 1);
}
