/**
 * @file   sprite.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 13:07:15 2022
 * 
 * @brief  Работа со спрайтами
 * Спрайты организованы в списки: списки потоков и списки сцен.
 * Спрайты добавляются в оба списка, список потока сортируется по
 * тегам, для последующей возможности удаления по тегу.
 * Список сцены используется для отрисовки
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sprite.h"
#include "interpret.h"
#include "get.h"
#include "memory.h"
#include "render.h"
#include "threads.h"

sprite_t *sprites;		/**< таблица спрайтов */
int num_sprites;		/**< всего спрайтов */
sprite_t *main_sprite;		/**< главный спрайт */
sprite_t *sprite2;
sprite_t *cursor_sprite;	/**< спрайт курсора мыши */
sprite_t *free_sprite;		/**< последний свободный спрайт из таблицы */
sprite_t *prev_sprite;		/**< предыдущий спрайт в списке текущего потока */
int remove_from_scene = 0;		/**< если 1 - то спрайты удаляются из сцены */
int sprite_flags;
vec_t translate;			/**< вектор перемещения для всех спрайтов */
int reg4;

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
  sprite_flags = 0;
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
  return num_sprites - (free_sprite - sprites);
}

/** 
 * устанавливает вектор перемещения для всех спрайтов
 * 
 * @param data адрес регистров перемещения
 */
void set_translate(word *data)
{
  translate.x = *data++;
  translate.y = *data++;
  translate.z = *data++;
  reg4 = *data;
#ifdef DEBUG
  printf("Set translate (%d %d %d %d)\n", translate.x, translate.y, translate.z, reg4);
#endif
}

/** 
 * Обновляет координаты для всех спрайтов в списке
 * текущего потока, если было перемещение
 * @param data адрес регистров перемещения
 */
void sprites_translate(word *data)
{
  vec_t delta;
  delta.x = translate.x - *data++;
  delta.y = translate.y - *data++;
  delta.z = translate.z - *data++;
  if (!delta.z && !delta.x && !delta.y)
    return;
  printf("sprite translation\n");
  exit(1);
  sprite_t *c = run_thread->sprite_list;
  while (c) {
    if (c->state == SPRITE_READY)
      c->state = SPRITE_UPDATED;
    vec_add(&c->center, &delta, &c->center);
    c = c->next;
#ifdef DEBUG
    printf("Translate sprite %d (%d %d %d)\n", (int)(c - sprites), c->center.x, c->center.y, c->center.z);
#endif
  }
}

/** 
 * Ищет место добавления нового спрайта с заданным тегом
 * Спрайты группируются по тегам, от большего к меньшему
 * сцены идут от меньшей к большей
 * 1. найден спрайт с таким же тегом в главной сцене (возвращает спрайт, prev
 * 2. спрайт в главной сцене, тег больше (возвращает 0, prev)
 * 3. список пустой (возвращает 0, 0)
 * 4. спрайт большей сцены (возвращает 0, prev)
 * 5. список потока закончился, все теги  меньше заданного (возвращает 0, prev)
 * @param tag заданный тег
 * @param c указатель куда будет записан спрайт, перед которым будет
 * добавлен новый
 * @return 0, если не найден спрайт с тегом, иначе 1
 */
int sprite_find(int tag, sprite_t **c)
{
  prev_sprite = 0;
  *c = run_thread->sprite_list;
  if (!*c)
    return 0;
  goto start;
  do {
    prev_sprite = *c;
    *c = (*c)->next;
    if (!*c)
      break;
  start:
    if (run_thread->current_scene < (*c)->scene)
      continue;
    if (run_thread->current_scene != (*c)->scene)
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
void dump_sprites()
{
  printf("run_thread list:\n");
  sprite_t *c = run_thread->sprite_list;
  while (c) {
    printf("->center(%d %d %d)tag(%d)", c->center.x, c->center.y, c->center.z, c->tag);
    c = c->next;
  }
  printf("\ncurrent scene_list:\n");
  c = sprites + run_thread->current_scene->scene_sprite;
  while (c) {
    printf("->origin(%d %d %d)center(%d %d %d)tag(%d)state(%d)\n", c->origin.x, c->origin.y, c->origin.z, c->center.x, c->center.y, c->center.z, c->tag, c->state);
    c = c->next_in_scene;
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
void sprite_set(sprite_t *c, byte *image, int x_flip, vec_t *coord)
{
  c->image = image;
  c->x_flip = x_flip;
  // много полей
  // применение трансформации перемещения
  vec_add(&translate, coord, &c->center);
#ifdef DEBUG
  printf("update sprite: (%d %d %d)\n", coord->x, coord->y, coord->z);
  dump_sprites();
#endif  
}

/** 
 * добавляет новый спрайт в список текущего потока перед заданным
 * спрайтом в общий список потока или создает новый список в потоке
 * также добавляет в голову списка отрисовки
 * scene_sprite содержит голову списка
 * @param c спрайт перед которым добавляется новый
 * @param tag тег нового спрайта
 * @param image данные изображения
 * @param x_flip если 1 - то изображение зеркально поворачивается
 * @param coord координаты центра спрайта
 */
void sprite_new_insert(sprite_t *c, int tag, byte *image, int x_flip, vec_t *coord)
{
  sprite_t *newc = free_sprite;
  if (!newc) {
    printf("no free sprites\n");
    exit(1);
  }
  free_sprite = newc->next;
  if (!prev_sprite)
    // новый список
    run_thread->sprite_list = newc;
  else
    prev_sprite->next = newc;
  newc->next = c;
  newc->scene = run_thread->current_scene;
  newc->tag = tag;
  newc->state = SPRITE_NEW;
  newc->f24 = run_thread->f2c;
  newc->layer = run_thread->layer;
  sprite_t *sc = sprites + run_thread->current_scene->scene_sprite;
  newc->next_in_scene = sc->next_in_scene;
  sc->next_in_scene = newc;
  sprite_set(newc, image, x_flip, coord);
}

/** 
 * Удаляет спрайт из списка отрисовки сцены, к которой он принадлежит
 * голова списка сцены находится в scene_sprite
 * @param c удаляемый спрайт
 */
void sprite_remove_from_scene_list(sprite_t *c)
{
  scene_t *sc = c->scene;
  sprite_t *sc_c = sprites + sc->scene_sprite;
  if (c == sc_c) {
    sc->scene_sprite = c->next_in_scene - sprites;
    return;
  }
  while (1) {
    if (c == sc_c->next_in_scene) {
      sc_c->next_in_scene = c->next_in_scene;
      break;
    }
    sc_c = sc_c->next_in_scene;
  }
}

/** 
 * Удаляет спрайт из списка потока. Если есть флаг сцены, или установлен
 * глобальный флаг удаления, то удаляется из списка сцены.
 * @param c спрайт
 * @return спрайт после удаленного
 */
sprite_t *sprite_remove(sprite_t *c)
{
  if (!remove_from_scene && c->state >= SPRITE_READY)
    c->state = SPRITE_REMOVED;
  if (!prev_sprite)
    run_thread->sprite_list = c->next;
  else
    prev_sprite->next = c->next;
  if (remove_from_scene || c->state < SPRITE_READY) {
    // добавление в список свободных спрайтов
    c->next = free_sprite;
    free_sprite = c;
    sprite_remove_from_scene_list(c);
  }
#ifdef DEBUG
  printf("remove sprite: center(%d %d %d)\n", c->center.x, c->center.y, c->center.z);
  dump_sprites();
#endif  
  if (!prev_sprite)
    return run_thread->sprite_list;
  else
    return prev_sprite->next;
}

/** 
 * Перемещается на следующий спрайт по одному слою
 * 
 * @param c текущий спрайт
 * @param tag слой
 * 
 * @return следующий спрайт или 0, если слой закончился или сцена закончилась или список закончился
 */
sprite_t *sprite_next_on_tag(sprite_t *c, int tag)
{
  prev_sprite = c;
  sprite_t *c2 = c->next;
  if (c2 && run_thread->current_scene == c2->scene)
    if (c2->tag == tag)
      return c2;
  return 0;
}

/// очистка всех спрайтов объекта с заданным тегом
void clear_object()
{
  new_get();
  int tag = (byte)current_value;
  sprite_t *c;
#ifdef DEBUG
  printf("clear object tag: %d\n", tag);
  exit(1);
#endif
  while (1) {
    // c = sprite_find(tag);
    if (!c) {
      remove_from_scene = 0;
      break;
    }
    c = sprite_remove(c);
  };
#ifdef DEBUG
  dump_sprites();
#endif
}
