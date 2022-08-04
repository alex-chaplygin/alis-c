/**
 * @file   scene.c
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
#include "window.h"
#include "interpret.h"
#include "memory.h"

window_t *window_list_head = 0;	/**< список окон */

/// добавление окна в конец списка окон
void window_add(window_t *cl)
{
  window_t *c;
  cl->next = 0;
  if (!window_list_head)
    window_list_head = cl;
  else {
    c = window_list_head;
    while (c->next)
      c = (window_t *)memory_read(c->next);
    c->next = (byte *)cl - memory;	/**< адрес в главной памяти */
  }
}

/** 
 * Вычисление левого ортонормированного базиса через векторное произведение векторов
 * 
 * @param cl окно
 */
void window_calc_basis(window_t *cl)
{
  cl->cx = cl->by * cl->az - cl->ay * cl->bz;
  cl->cy = cl->bz * cl->ax - cl->az * cl->bx;
  cl->cz = cl->bx * cl->ay - cl->ax * cl->by;
}

/// создание нового окна
void window_new()
{
  word adr = fetch_word();
  window_t *cl = (window_t *)memory_read(adr);
  cl->flags |= WINDOW_HIDDEN;
  cl->flags2 = fetch_byte();
  memcpy((byte *)cl + 6, current_ip, 32);
  current_ip += 32;
  sprite_t *sprite = free_sprite;
  free_sprite = free_sprite->next;
  cl->window_sprite = sprite - sprites; /**< номер спрайта */
  sprite->next_in_window = 0;
  cl->next = 0;
  sprite->flags2 = cl->flags2;  
  sprite->origin.x = cl->origin_x & 0xfff0; /**< выравнивание по 16 */
  sprite->origin.y = cl->origin_y;
  sprite->origin.z = 32767;
  sprite->max.x = (cl->origin_x + cl->width) | 15; /**< выравнивание 16 */
  sprite->max.y =cl->origin_y + cl->height;
  cl->delta_x = cl->delta_y = cl->delta_z = 0;
  window_add(cl);
  window_calc_basis(cl);
#ifdef DEBUG
  printf("New window: %x flags = %x flags2 = %d\n", adr, cl->flags, cl->flags2);
  printf("window_sprite = %d\n", cl->window_sprite);
  printf("width = %d height = %d\n", cl->width, cl->height);
  printf("origin = %d %d %d max = %d %d\n", sprite->origin.x, sprite->origin.y, sprite->origin.z, sprite->max.x, sprite->max.y);
  printf("a = (%d %d %d) b = (%d %d %d) c = (%d %d %d)\n", cl->ax, cl->ay, cl->az, cl->bx, cl->by, cl->bz, cl->cx, cl->cy, cl->cz);
  printf("%d %d %d\n%d %d %d\n%d %d\n", cl->f1, cl->f2, cl->f3, cl->f4, cl->f5, cl->f6, cl->f7, cl->f8);
  //  ASSERT((byte *)window_list_head - memory, 0xe)
#endif
}

/// удаление окна из списка
void window_remove(window_t *sc)
{
  window_t *s = window_list_head;
  window_t *s2;
  if (!s)
    return;
  if (sc == s) {
    if (!sc->next)
      window_list_head = 0;
    else
      window_list_head = (window_t *)memory_read(sc->next);
    return;
  }
  do {
    s2 = (window_t *)memory_read(s->next);
    if (s2 == sc) { // s -> s2(sc) ->s3
      s2 = (window_t *)memory_read(sc->next);
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
void window_show()
{
  word w = fetch_word();
  window_t *sc = (window_t *)memory_read(w);
  sc->flags &= ~WINDOW_HIDDEN;
  sc->flags |= WINDOW_NOTTRANSLATED;
  window_remove(sc);
  window_add(sc);
#ifdef DEBUG
  printf("window show %x flags = %x\n", w, sc->flags);
#endif
}

/** 
 * Команда - сделать окно невидимым
 */
void window_hide()
{
  word w = fetch_word();
  window_t *sc = (window_t *)memory_read(w);
  sc->flags |= WINDOW_HIDDEN;
#ifdef DEBUG
  printf("window hide %x flags = %x\n", w, sc->flags);
#endif
}

/// установка текущей окна
void window_set()
{
  word w = fetch_word();
  // if ((window_t *)(w + memory) != run_thread->current_window)
    run_thread->current_window = (window_t *)memory_read(w);
#ifdef DEBUG
  printf("set current window %x %x\n", w, (int)((byte *)run_thread->current_window - memory));
#endif
}

/** 
 * Перемещает начало координат окна. Обновляет спрайты.
 * 
 * @param window сцена
 * @param c спрайт сцены
 */
void window_translate(window_t *window, sprite_t *c)
{
  window->origin_x += window->delta_x;
  window->origin_y += window->delta_y;
  window->origin_z += window->delta_z;
  window->delta_x = window->delta_y = window->delta_z = 0;
  c = c->next_in_window;
  while (c) {
    if (c->state == SPRITE_READY)
      c->state = SPRITE_UPDATED;
    c = c->next_in_window;
  }
  window->flags &= ~WINDOW_NOTTRANSLATED;
#ifdef DEBUG
  printf("window translate: (%d %d %d) flags = %x\n", window->origin_x, window->origin_y, window->origin_z, window->flags);
#endif  
}

/** 
 * Удаляет все спрайты из окна
 * 
 */
void window_free_sprites()
{
  window_t *s = window_list_head;
  sprite_t *sp;
  while (1) {
    sp = sprites + s->window_sprite;
    sp = sp->next_in_window;
    while (sp) {
      //      printf("windows free sprites\n");
      //exit(1);
      sp = sp->next;
    }
    if (!s->next)
      break;
    s = (window_t *)(memory + s->next);
  }
}
