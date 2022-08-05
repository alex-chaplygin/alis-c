/**
 * @file   sprite.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 13:07:15 2022
 * 
 * @brief  Работа со спрайтами
 * Спрайты организованы в списки: списки объектов и списки отображений.
 * Спрайты добавляются в оба списка, список потока сортируется по
 * тегам, для последующей возможности удаления по тегу.
 * Список отображения используется для отрисовки
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sprite.h"
#include "interpret.h"
#include "get.h"
#include "memory.h"
#include "render.h"
#include "objects.h"
#include "res.h"
#include "palette.h"

sprite_t *sprites;		/**< таблица спрайтов */
int num_sprites;		/**< всего спрайтов */
sprite_t *main_sprite;		/**< главный спрайт */
sprite_t *sprite2;
sprite_t *cursor_sprite;	/**< спрайт курсора мыши */
sprite_t *free_sprite;		/**< последний свободный спрайт из таблицы */
sprite_t *prev_sprite;		/**< предыдущий спрайт в списке текущего потока */
vec_t current_origin;		/**< начало координат текущего объекта */
int remove_from_view = 0;	/**< нужно ли удалять из сцены */
int image_flag = 0;

/** 
 * Создание таблицы спрайтов
 * 3 спрайта резервируются: главный, 2-й и курсор
 * все остальные спрайты помещаются в список свободных
 * @param num всего спрайтов
 */
void sprites_init(int num)
{
  num_sprites = num + 3;
  sprites = xmalloc(num_sprites * sizeof(sprite_t));
  main_sprite = sprites;
  main_sprite->origin.x = 0;
  main_sprite->origin.y = 0;
  main_sprite->max.x = 319;
  main_sprite->max.y = 199;
  sprite2 = sprites + 1;
  sprite2->tag = 0;
  cursor_sprite = sprite2 + 1;
  cursor_sprite->origin.x = 0;
  cursor_sprite->origin.y = 0;
  cursor_sprite->state = SPRITE_CURSOR; // 11111110
  free_sprite = cursor_sprite + 1;
  sprite_t *c = free_sprite;
  for (int i = 0; i < num - 4; i++) {
    c->next = c + 1;
    c++;
  }
  sprites[num_sprites - 1].next = 0;
}


/// возвращает число свободных спрайтов
int num_free_sprites()
{
  sprite_t *c = free_sprite;
  int count = 0;
  while (c) {
    count++;
    c = c->next;
  }
#ifdef DEBUG
  printf("num free sprites: %d\n", count);
#endif
  exit(1);
  return count;
}

/** 
 * Перемещает все спрайты текущего объекта, если было перемещение
 * @param delta вектор перемещения
 */
void sprites_translate(vec_t *delta)
{
  if (!delta->z && !delta->x && !delta->y)
    return;
  printf("sprite translation delta = (%d %d %d)\n", delta->x, delta->y, delta->z);
  sprite_t *c = run_object->sprite_list;
  while (c) {
    if (c->state == SPRITE_READY)
      c->state = SPRITE_UPDATED;
    vec_add(&c->center, delta, &c->center);
#ifdef DEBUG
    printf("Translate sprite %d (%d %d %d)\n", (int)(c - sprites), c->center.x, c->center.y, c->center.z);
#endif
    c = c->next;
  }
}

/** 
 * Установка номера объекта, которому будут принадлежать новые спрайты
 */
void set_sprites_object()
{
  new_get();
#ifdef DEBUG
  printf("set sprites object: %x prev: %x\n", current_value, run_object->sprites_object);
#endif
  run_object->sprites_object = current_value;
}

/** 
 * Команда - установка слоя для новых спрайтов.
 * Спрайты при отрисовке будут сортироваться по убыванию слоев.
 */
void set_sprites_layer()
{
  new_get();
#ifdef DEBUG
  printf("set sprites layer = %x; %d\n", (char)current_value, (char)current_value);
#endif
  run_object->layer = (char)current_value;
}

/** 
 * Ищет спрайт с заданным тегом
 * Спрайты группируются по тегам, от большего к меньшему
 * отображения идут от меньшей к большей
 * 1. найден спрайт с таким же тегом в текущем отображении (возвращает спрайт, prev)
 * 2. спрайт в текущем отображении, тег больше (возвращает 0, prev)
 * 3. список пустой (возвращает 0, 0)
 * 4. спрайт большего отображения (возвращает 0, prev)
 * 5. список закончился, все теги  меньше заданного (возвращает 0, prev)
 * @param tag заданный тег
 * @param c указатель куда будет записан спрайт, перед которым будет
 * добавлен новый
 * @return 0, если не найден спрайт с тегом, иначе 1
 */
int sprite_find_by_tag(int tag, sprite_t **c)
{
  prev_sprite = 0;
  *c = run_object->sprite_list;
  if (!*c)
    return 0;
  goto start;
  do {
    prev_sprite = *c;
    *c = (*c)->next;
    if (!*c)
      break;
  start:
    if (run_object->current_view < (*c)->view)
      continue;
    if (run_object->current_view != (*c)->view)
	break;
    if (tag < (*c)->tag)
      continue;
    if (tag != (*c)->tag)
      break;
    return 1;
  } while (*c);
  return 0;
}

/// вывод списка спрайтов
void dump_sprites(view_t *sc)
{
  printf("run_object list:\n");
  sprite_t *c = run_object->sprite_list;
  while (c) {
    printf("->center(%d %d %d)tag(%d)", c->center.x, c->center.y, c->center.z, c->tag);
    c = c->next;
  }
  printf("\ncurrent view_list:\n");
  c = sprites + sc->view_sprite;
  while (c) {
    printf("->origin(%d %d %d)center(%d(%4x) %d(%4x) %d(%4x))tag(%d)state(%d) layer(%2x)\n", c->origin.x, c->origin.y, c->origin.z, c->center.x, c->center.x, c->center.y, c->center.y, c->center.z, c->center.z, c->tag, c->state, c->layer);
    c = c->next_in_view;
  }
}

/** 
 * Установка параметров спрайта.
 * Координаты устанавливаются с учетом текущего перемещения
 * @param c спрайт
 * @param image данные изображения
 * @param x_flip отражение по горизонтали
 * @param coord координаты центра спрайта
 */
void sprite_set_params(sprite_t *c, byte *image, int x_flip, vec_t *coord)
{
  c->image = image;
  c->x_flip = x_flip;
  c->object = run_object->sprites_object;
  c->layer = run_object->layer;
  c->f24 = run_object->f2c;
  c->f1c = run_object->f25;
  // к центру спрайта добавляются координаты объекта
  vec_add(&current_origin, coord, &c->center);
#ifdef DEBUG
  printf("sprite set params: (%d %d %d)\n", coord->x, coord->y, coord->z);
#endif  
}

/** 
 * добавляет новый спрайт в список текущего потока перед заданным
 * спрайтом в общий список потока или создает новый список в потоке
 * также добавляет в голову списка отрисовки
 * view_sprite содержит голову списка
 * @param c спрайт перед которым добавляется новый
 * @param tag тег нового спрайта
 * @param image данные изображения
 * @param x_flip если 1 - то изображение зеркально поворачивается
 * @param coord координаты центра спрайта
 */
void sprite_new(sprite_t *c, int tag, byte *image, int x_flip, vec_t *coord)
{
  sprite_t *newc = free_sprite;
  if (!newc) {
    printf("no free sprites\n");
    exit(1);
  }
  free_sprite = newc->next;
  if (!prev_sprite)
    // новый список
    run_object->sprite_list = newc;
  else
    prev_sprite->next = newc;
  newc->next = c;
  newc->view = run_object->current_view;
  newc->tag = tag;
  newc->state = SPRITE_NEW;
  // спрайты могут создаваться разными объектами, но принадлежать одному
  sprite_t *sc = sprites + run_object->current_view->view_sprite;
  newc->next_in_view = sc->next_in_view;
  sc->next_in_view = newc;
  sprite_set_params(newc, image, x_flip, coord);
}

/** 
 * Удаляет спрайт из списка отображения, к которому он принадлежит
 * голова списка сцены находится в view_sprite
 * @param c удаляемый спрайт
 */
void sprite_remove_from_view(sprite_t *c)
{
  view_t *sc = c->view;
  sprite_t *sc_c = sprites + sc->view_sprite;
  if (c == sc_c) {
    sc->view_sprite = c->next_in_view - sprites;
    return;
  }
  while (1) {
    if (c == sc_c->next_in_view) {
      sc_c->next_in_view = c->next_in_view;
      break;
    }
    sc_c = sc_c->next_in_view;
  }
}

/** 
 * Удаляет спрайт из списка объекта.
 * Для спрайта устанавливается состояние - удален, он будет
 * удален из списка отображения при отрисовке
 * @param c спрайт
 * @param remove_from_view если 1 - то спрайт будет удален из списка
 * сцены и возвращен в список свободных спрайтов
 * @return спрайт после удаленного
 */
sprite_t *sprite_remove(sprite_t *c, int remove_from_view)
{
  if (c->state >= SPRITE_READY)
    c->state = SPRITE_REMOVED;
  if (!prev_sprite) {
#ifdef DEBUG
    printf("remove head\n");
#endif
    run_object->sprite_list = c->next;
  } else
    prev_sprite->next = c->next;
#ifdef DEBUG
  printf("remove sprite: center(%d %d %d)\n", c->center.x, c->center.y, c->center.z);
#endif  
  if (remove_from_view) {
    c->next = free_sprite;
    free_sprite = c;
    sprite_remove_from_view(c);
  }
  if (!prev_sprite) {
#ifdef DEBUG
    printf("next: %x\n", (int)run_object->sprite_list);
#endif
    return run_object->sprite_list;
  } else
    return prev_sprite->next;
}

/** 
 * Ищет следующий спрайт по заданному тегу
 * 
 * @param c текущий спрайт
 * @param tag слой
 * @param c2 указатель куда будет записан спрайт, следующий за найденным или
 * спрайт, где останавливается поиск
 * @return 0 - если поиск остановлен, иначе 1
 */
int sprite_next_on_tag(sprite_t *c, int tag, sprite_t **c2)
{
  prev_sprite = c;
  *c2 = c->next;
  if (*c2 && run_object->current_view == (*c2)->view)
    if ((*c2)->tag == tag)
      return 1;
  return 0;
}

/** 
 * Удаление спрайтов текущего объекта с заданным тегом
 */
void sprites_clear_with_tag()
{
  new_get();
  int tag = (char)current_value;
  int found;
  sprite_t *c;
#ifdef DEBUG
  printf("clear object tag: %d\n", tag);
#endif
  while (1) {
    found = sprite_find_by_tag(tag, &c);
    if (!found) {
      break;
    }
    c = sprite_remove(c, remove_from_view);
  }
  remove_from_view = 0;
}

/** 
 * Удаление спрайтов текущего объекта с заданным тегом.
 * Удаляет спрайты объекта из всех списков.
 */
void sprites_clear_with_tag_view()
{
  remove_from_view = 1;
  sprites_clear_with_tag();
}

/** 
 * Удаляет все спрайты из текущего объекта
 * 
 * @param sp первый спрайт списка
 * @param remove если 1 - то также удаление из списка отображения
 */
void remove_all_sprites(sprite_t *sp, int remove)
{
#ifdef DEBUG
  printf("remove all sprites\n");
#endif
  prev_sprite = 0;
  while (sp)
    sp = sprite_remove(sp, remove);
}

/** 
 * Удаляет спрайты с заданным тегом, если у них состояние - готов
 * Используется для изменения отдельных спрайтов (не объектов)
 * когда новый спрайт добавляется в список, а старый - удаляется
 * @param tag 
 */
void sprites_clear_ready(int tag)
{
  sprite_t *c;
  sprite_t *c2;
  int found = sprite_find_by_tag(tag, &c);
  if (found)
    while (c) {
#ifdef DEBUG
      printf("sprites_clear_ready: check sprite center(%d %d %d)\n", c->center.x, c->center.y, c->center.z);
#endif
      if (c->state == SPRITE_READY) {
	c = sprite_remove(c, 0);
	if (!c)
	  break;
	if (run_object->current_view != c->view)
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
 * Добавление нового спрайта в список спрайтов объекта и отображения
 * Новый спрайт группируется по тегу с другими спрайтами одного объекта.
 * Если объект с заданным тегом уже был, то он меняется на новую позицию
 * или у него меняется изображение (в этом случае ставится состояние - 
 * обновленный)
 * @param num номер изображения
 * @param origin координаты центра спрайта
 * @param x_flip отражение по горизонтали
 * @param is_object 1 - для объекта, 0 - спрайт
 */
void sprite_add(int num, vec_t *origin, int x_flip, int is_object, int tag)
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
  // ищем спрайт с тегом
  found = sprite_find_by_tag(tag, &c);
  // если не найден, то добавляем
  if (!found)
    sprite_new(c, tag, res_get_image(num), x_flip, origin);
  else {
    while(c) {
      if (c->state == SPRITE_READY) {
	printf("add sprite: update sprite\n");
	// обновить спрайт
	c->state = SPRITE_UPDATED;
	sprite_set_params(c, res_get_image(num), x_flip, origin);
	goto end;
      }
      found = sprite_next_on_tag(c, tag, &c2);
      c = c2;
      if (!found)
	break;
    }
    sprite_new(c, tag, res_get_image(num), x_flip, origin);
  }
 end:
  if (!is_object)
    sprites_clear_ready(tag);
  image_flag = 0;
}

/** 
 * Добавление композитного спрайта
 * Состоит из одного или более спрайтов
 *
 * @param img данные объекта
 * @param coord координаты центра
 * @param x_flip зеркальное отражение
 * @param tag тег спрайтов
 */
void add_composite_sprite(byte *img, vec_t *coord, int x_flip, int tag)
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
    sprite_add(num, &vec, x_fl, 1, tag);
    sub++;
  }
  sprites_clear_ready(tag);
  image_flag = 0;
}

/** 
 * Добавление спрайтов
 * Обработка палитры и составных спрайтов
 * при загрузке палитры prev_value - скорость появления, 0 - без появления* 
 * @param coord центр изображения 
 * @param x_flip зеркальное отражение
 * @param tag тег
 */
void sprite_add_with_flip(vec_t *coord, int x_flip, int tag)
{
  byte *img = res_get_image(current_value);
  if (*img == RES_PALETTE) {
    printf("loading palette\n");
    exit(1);
    palette_load(img + 1);
  } else if (*img == RES_COMPOSITE_SPRITE)
    add_composite_sprite(img, coord, x_flip, tag);
  else 
    sprite_add(current_value, coord, x_flip, 0, tag);
}

/** 
 * Показать спрайт с заданными координатами и тегом
 * 
 * @param x_flip - если 1, то зеркальное отражение по вертикали
 */
void sprite_show_with_flip(int x_flip)
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
  printf("show sprite (%d, %d, %d) xflip = %d res_num = %d tag = %d\n", coord.x, coord.y, coord.z, x_flip, current_value, tag); 
#endif
  sprite_add_with_flip(&coord, x_flip, tag);
}

/// показать спрайт по координатам центра.
void sprite_show()
{
  load_main_res = 0;
  sprite_show_with_flip(run_object->x_flip);
}

/// показать спрайты с отражением по горизонтали
void sprite_show_flipped()
{
  load_main_res = 0;
#ifdef DEBUG
  printf("sprite show flipped\n");
#endif
  sprite_show_with_flip(run_object->x_flip ^ 1);  
}

/// установка тега
void set_tag()
{
  new_get();
#ifdef DEBUG
  printf("set tag %x\n", current_value);
#endif
}

/// Удаляет все спрайты текущего объекта
void sprites_clear_all_view()
{
  #ifdef DEBUG
  printf("clear all sprites\n");
  #endif
  remove_all_sprites(run_object->sprite_list, 1);
}

/// показывает спрайт со всеми координатами и тегом 0
void sprite_show_0()
{
  vec_t coord;
  load_main_res = 0;
  new_get();
  coord.x = coord.y = coord.z = 0;
#ifdef DEBUG
  printf("show sprite 0 (0 0 0) res = %d xflip = %d\n", current_value, run_object->x_flip); 
#endif
  sprite_add_with_flip(&coord, run_object->x_flip, 0);
}

/// Удаляет все спрайты текущего объекта, может не удалять из окна
void sprites_clear_all()
{
  #ifdef DEBUG
  printf("sprites clear all\n");
  #endif
  remove_all_sprites(run_object->sprite_list, remove_from_view);
}
