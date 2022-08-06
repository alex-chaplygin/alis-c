/**
 * @file   view.c
 * @author alex <alex@localhost>
 * @date   Mon Jan 31 09:13:45 2022
 * 
 * @brief  Функции для работы с окнами
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "view.h"
#include "interpret.h"
#include "memory.h"

view_t *view_list_head = 0;	/**< список окон */

/// добавление окна в конец списка окон
void view_add(view_t *cl)
{
  view_t *c;
  cl->next = 0;
  if (!view_list_head)
    view_list_head = cl;
  else {
    c = view_list_head;
    while (c->next)
      c = (view_t *)memory_read(c->next);
    c->next = (byte *)cl - memory;	/**< адрес в главной памяти */
  }
}

/** 
 * Вычисление левого ортонормированного базиса через векторное произведение векторов
 * 
 * @param cl окно
 */
void view_calc_basis(view_t *cl)
{
  cl->cx = cl->by * cl->az - cl->ay * cl->bz;
  cl->cy = cl->bz * cl->ax - cl->az * cl->bx;
  cl->cz = cl->bx * cl->ay - cl->ax * cl->by;
}

/// создание нового окна
void view_new()
{
  word adr = fetch_word();
  view_t *cl = (view_t *)memory_read(adr);
  cl->flags |= VIEW_HIDDEN;
  cl->flags2 = fetch_byte();
  memcpy((byte *)cl + 6, current_ip, 32);
  current_ip += 32;
  sprite_t *sprite = free_sprite;
  free_sprite = free_sprite->next;
  cl->view_sprite = sprite - sprites; /**< номер спрайта */
  sprite->next_in_view = 0;
  cl->next = 0;
  sprite->flags2 = cl->flags2;  
  sprite->origin.x = cl->origin_x & 0xfff0; /**< выравнивание по 16 */
  sprite->origin.y = cl->origin_y;
  sprite->origin.z = 32767;
  sprite->max.x = (cl->origin_x + cl->width) | 15; /**< выравнивание 16 */
  sprite->max.y =cl->origin_y + cl->height;
  cl->delta_x = cl->delta_y = cl->delta_z = 0;
  view_add(cl);
  view_calc_basis(cl);
#ifdef DEBUG
  printf("New view: %x flags = %x flags2 = %d\n", adr, cl->flags, cl->flags2);
  printf("view_sprite = %d\n", cl->view_sprite);
  printf("width = %d height = %d\n", cl->width, cl->height);
  printf("origin = %d %d %d max = %d %d\n", sprite->origin.x, sprite->origin.y, sprite->origin.z, sprite->max.x, sprite->max.y);
  printf("a = (%d %d %d) b = (%d %d %d) c = (%d %d %d)\n", cl->ax, cl->ay, cl->az, cl->bx, cl->by, cl->bz, cl->cx, cl->cy, cl->cz);
  printf("%d %d %d\n%d %d %d\n%d %d\n", cl->f1, cl->f2, cl->f3, cl->f4, cl->f5, cl->f6, cl->f7, cl->f8);
  //  ASSERT((byte *)view_list_head - memory, 0xe)
#endif
}

/// удаление окна из списка
void view_remove(view_t *sc)
{
  view_t *s = view_list_head;
  view_t *s2;
  if (!s)
    return;
  if (sc == s) {
    if (!sc->next)
      view_list_head = 0;
    else
      view_list_head = (view_t *)memory_read(sc->next);
    return;
  }
  do {
    s2 = (view_t *)memory_read(s->next);
    if (s2 == sc) { // s -> s2(sc) ->s3
      s2 = (view_t *)memory_read(sc->next);
      s->next = (byte *)s2 - memory;
      return;
    }
    s = s2;
  } while(s2);
}

/** 
 * Делает окно видимым и
 * помещает в конец списка
 */
void view_show()
{
  word w = fetch_word();
  view_t *sc = (view_t *)memory_read(w);
  sc->flags &= ~VIEW_HIDDEN;
  sc->flags |= VIEW_NOTTRANSLATED;
  view_remove(sc);
  view_add(sc);
#ifdef DEBUG
  printf("view show %x flags = %x\n", w, sc->flags);
#endif
}

/** 
 * Команда - сделать окно невидимым
 */
void view_hide()
{
  word w = fetch_word();
  view_t *sc = (view_t *)memory_read(w);
  sc->flags |= VIEW_HIDDEN;
#ifdef DEBUG
  printf("view hide %x flags = %x\n", w, sc->flags);
#endif
}

/// установка текущего окна
void view_set()
{
  word w = fetch_word();
  // if ((view_t *)(w + memory) != run_object->current_view)
    run_object->current_view = (view_t *)memory_read(w);
#ifdef DEBUG
  printf("set current view %x %x\n", w, (int)((byte *)run_object->current_view - memory));
#endif
}

/** 
 * Перемещает начало координат окна. Обновляет спрайты.
 * 
 * @param view сцена
 * @param c спрайт сцены
 */
void view_translate(view_t *view, sprite_t *c)
{
  view->origin_x += view->delta_x;
  view->origin_y += view->delta_y;
  view->origin_z += view->delta_z;
  view->delta_x = view->delta_y = view->delta_z = 0;
  c = c->next_in_view;
  while (c) {
    if (c->state == SPRITE_READY)
      c->state = SPRITE_UPDATED;
    c = c->next_in_view;
  }
  view->flags &= ~VIEW_NOTTRANSLATED;
#ifdef DEBUG
  printf("view translate: (%d %d %d) flags = %x\n", view->origin_x, view->origin_y, view->origin_z, view->flags);
#endif  
}

/** 
 * Удаляет все спрайты из окна
 */
void view_free_sprites()
{
  view_t *s = view_list_head;
  sprite_t *sp;
  while (1) {
    sp = sprites + s->view_sprite;
    sp = sp->next_in_view;
    while (sp) {
      //      printf("views free sprites\n");
      //exit(1);
      sp = sp->next_in_view;
    }
    if (!s->next)
      break;
    s = (view_t *)(memory + s->next);
  }
}
