/**
 * @file   scene.c
 * @author alex <alex@localhost>
 * @date   Mon Jan 31 09:13:45 2022
 * 
 * @brief  Функции для работы со сценами
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "scene.h"
#include "interpret.h"
#include "memory.h"

scene_t *scene_list_head = 0;	/**< список сцен */

/// добавление сцены в конец списка сцен
void scene_add(scene_t *cl)
{
  scene_t *c;
  cl->next = 0;
  if (!scene_list_head)
    scene_list_head = cl;
  else {
    c = scene_list_head;
    while (c->next)
      c = (scene_t *)memory_read(c->next);
    c->next = (byte *)cl - memory;	/**< адрес в главной памяти */
  }
}

/** 
 * Вычисление левого ортонормированного базиса через векторное произведение векторов
 * 
 * @param cl сцена
 */
void scene_calc_basis(scene_t *cl)
{
  cl->cx = cl->by * cl->az - cl->ay * cl->bz;
  cl->cy = cl->bz * cl->ax - cl->az * cl->bx;
  cl->cz = cl->bx * cl->ay - cl->ax * cl->by;
}

/// создание новой сцены
void scene_new()
{
  word adr = fetch_word();
  scene_t *cl = (scene_t *)memory_read(adr);
  cl->flags |= SCENE_HIDDEN;
  byte tag = fetch_byte();
  cl->tag = tag;
  memcpy((byte *)cl + 6, current_ip, 32);
  current_ip += 32;
  sprite_t *sprite = free_sprite;
  free_sprite = free_sprite->next;
  cl->scene_sprite = sprite - sprites; /**< номер спрайта */
  sprite->next_in_scene = 0;
  cl->next = 0;
  sprite->tag = cl->tag;
  sprite->min.x = cl->min_x & 0xfff0; /**< выравнивание по 16? */
  sprite->min.y = cl->min_y;
  sprite->min.z = 32767;
  sprite->max.x = (cl->min_x + cl->width) | 15; /**< выравнивание 16 */
  sprite->max.y =cl->min_y + cl->height;
  cl->delta_x = cl->delta_y = cl->delta_z = 0;
  scene_add(cl);
  scene_calc_basis(cl);
#ifdef DEBUG
  printf("New scene: %x flags = %x tag = %d\n", adr, cl->flags, cl->tag);
  printf("scene_sprite = %d\n", cl->scene_sprite);
  printf("width = %d height = %d\n", cl->width, cl->height);
  printf("min = %d %d %d max = %d %d\n", sprite->min.x, sprite->min.y, sprite->min.z, sprite->max.x, sprite->max.y);
  printf("a = (%d %d %d) b = (%d %d %d) c = (%d %d %d)\n", cl->ax, cl->ay, cl->az, cl->bx, cl->by, cl->bz, cl->cx, cl->cy, cl->cz);
  printf("%d %d %d\n%d %d %d\n%d %d\n", cl->f1, cl->f2, cl->f3, cl->f4, cl->f5, cl->f6, cl->f7, cl->f8);
  ASSERT((byte *)scene_list_head - memory, 0xe)
#endif
}

/// удаление сцены из списка сцен
void scene_remove(scene_t *sc)
{
  scene_t *s = scene_list_head;
  scene_t *s2;
  if (!s)
    return;
  if (sc == s) {
    if (!sc->next)
      scene_list_head = 0;
    else
      scene_list_head = (scene_t *)memory_read(sc->next);
    return;
  }
  do {
    s2 = (scene_t *)memory_read(s->next);
    if (s2 == sc) { // s -> s2(sc) ->s3
      s2 = (scene_t *)memory_read(sc->next);
      s->next = (byte *)s2 - memory;
      return;
    }
    s = s2;
  } while(s2);
}

/** 
 * Делает указанную сцену видимой и
 * помещает в конец списка сцен
 */
void scene_show()
{
  word w = fetch_word();
  scene_t *sc = (scene_t *)memory_read(w);
  sc->flags &= ~SCENE_HIDDEN;
  sc->flags |= SCENE_NOTUPDATED;
  scene_remove(sc);
  scene_add(sc);
#ifdef DEBUG
  printf("scene show %x flags = %x\n", w, sc->flags);
#endif
}

/// установка текущей сцены
void scene_set()
{
  word w = fetch_word();
  // if ((scene_t *)(w + memory) != run_thread->current_scene)
    run_thread->current_scene = (scene_t *)memory_read(w);
#ifdef DEBUG
  printf("set current scene %x %x\n", w, (int)((byte *)run_thread->current_scene - memory));
#endif
}
