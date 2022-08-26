/**
 * @file   render.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 17:27:56 2022
 * 
 * @brief  Визуализация списка спрайтов.
 * Вычисление областей, где нужно перерисовать спрайты.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "sprite.h"
#include "view.h"
#include "memory.h"
#include "vector.h"
#include "draw.h"
#include "project.h"
#include "interpret.h"
#include "get.h"
#include "graphics.h"
#include "palette.h"
#include "clip.h"
#include "mouse.h"

byte frames_to_skip = 0;		/**< через сколько кадров обновляется экран */
byte frame_num = 0;			/**< текущий счетчик кадров */
rectangle_t sprites_rec;	/**< окно вывода новых спрайтов */
rectangle_t clip_rec;	/**< окно вывода новых спрайтов, отсеченное по окну view */
int sprite_object;	/**< номер объекта, чьи спрайты обрабатываются вместе */

/** 
 * Новый спрайт помещается на новое место в списке отрисовки спрайтов
 * Идет сортировка по убыванию z координаты, затем по порядку (order),
 * по убыванию тегов. Состояние спрайта становится - READY - готов к 
 * отрисовке
 * @param sc спрайт головы списка отображения
 * @param sprite текущий спрайт
 * @return спрайт перед которым добавляется новый, если новый спрайт в конце списка, то возвращается он
 */
sprite_t *insert_sort_sprite(sprite_t *sc, sprite_t *sprite)
{
  sprite_t *prev;
  sprite_t* c = sc;
#ifdef DEBUG
  printf("sort sprite: origin(%d %d %d)center(%d %d %d)tag(%d)layer(%d)state(%d)\n", sprite->origin.x, sprite->origin.y, sprite->origin.z, sprite->center.x, sprite->center.y, sprite->center.z, sprite->tag, sprite->layer, sprite->state);
#endif
  do {
    prev = c;
    c = c->next_in_view;
    if (!c)
      break;
#ifdef DEBUG
    printf("sort: origin(%d %d %d)center(%d %d %d)tag(%d)state(%d)\n", c->origin.x, c->origin.y, c->origin.z, c->center.x, c->center.y, c->center.z, c->tag, c->state);
#endif
    if (c->state < SPRITE_READY) // спрайт помещается после новых
      continue;
    if (sprite->origin.z < c->origin.z) // сортировка по z с уменьшением z
      continue;
    if (sprite->origin.z > c->origin.z)
      break;
    if (sprite->layer < c->layer) // сортировка по слою по убыванию
      continue;
    if (sprite->layer > c->layer)
      break;
    if (sprite->tag < c->tag) // сортировка по тегам по убыванию
      continue;
    if (sprite->tag > c->tag)
      break;
    if (sprite->state == SPRITE_NEW)
      break;
  } while (c);
  sprite->next_in_view = c;
  prev->next_in_view = sprite;
  sprite->state = SPRITE_READY;
  if (prev != sc)
    return sc;
  else 
    return sprite;
}

/** 
 * Обработка спрайта: проецирование, сортировка.
 * 
 * @param sc спрайт отображения
 * @param prev предыдущий спрайт
 * @param c обрабатываемый спрайт спрайт
 */
sprite_t *project_sort_sprite(sprite_t *sc, sprite_t *prev, sprite_t *c)
{
  // устанавливаем левый верхний угол после проецирования
  project_sprite(c, &c->origin);
  // удаление спрайта из списка отображения
  prev->next_in_view = c->next_in_view;
  // устанавливаем изображение отрисовки
  c->render_image = c->image;
  //clip_update_rec(c, &sprites_rec);
  sprite_t *s = insert_sort_sprite(sc, c);
   return prev;
}

/** 
 * Удаление спрайта из списка сцены
 * Место, где был спрайт добавляется в прямоугольник перерисовки
 * Возвращение спрайта в список свободных спрайтов
 * @param prev предыдущий спрайт
 * @param c удаляемый спрайт
 * @return спрайт перед удаленным
 */
sprite_t *delete_sprite(sprite_t *prev, sprite_t *c)
{
  //clip_update_rec(c, &sprites_rec);
  prev->next_in_view = c->next_in_view;
  c->next = free_sprite;
  free_sprite = c;
  return prev;
}

/** 
 * Обработка спрайтов: проецирование, сортировка, добавление в область
 * отрисовки, удаление
 * @param sc_sprite голова списка спрайтов
 */
/*void process_sprites(sprite_t *sc_sprite)
{
  sprite_t *sprite = sc_sprite;
  sprite_t *prev;
#ifdef DEBUG
  printf("process sprites\n");
#endif
  do {
    prev = sprite;
    sprite = sprite->next_in_view;
    if (!sprite)
      break;
#ifdef DEBUG
    printf("sprite: origin(%d %d %d)center(%d %d %d)tag(%d)state(%d)\n", sprite->origin.x, sprite->origin.y, sprite->origin.z, sprite->center.x, sprite->center.y, sprite->center.z, sprite->tag, sprite->state);
#endif
    // обрабатываются спрайты заданного объекта
    if (sprite_object != sprite->object)
      continue;
    if (sprite->state == SPRITE_READY)
      continue;
    else if (sprite->state < SPRITE_READY) {
      project_sort_sprite(sc_sprite, prev, sprite);
    } else if (sprite->state == SPRITE_UPDATED) {
      clip_update_rec(sprite, &sprites_rec);
      project_sort_sprite(sc_sprite, prev, sprite);
    } else 
      delete_sprite(prev, sprite);
    sprite = prev;
  } while (1);
  }*/

/** 
 * Отрисовка спрайта.
 * Временно координаты центра заменяются на координаты правого нижнего угла
 * Отсечение спрайта относительно окна отрисовки, если спрайт за пределами окна, то 
 * не рисуется. Вычисленный прямоугольник отрисовки отправляется в модуль отрисовки.
 * @param sp спрайт
 * @param clip координатное окно области отрисовки новых спрайтов
 */
void render_sprite(sprite_t *sp, rectangle_t *clip)
{
  rectangle_t blit;
  image_t *im = (image_t *)sp->render_image;
#ifdef DEBUG
  printf("Rendering sprite: %x %d origin(%d %d %d)size(%d %d)type(%x)\n", sp->class, sp->image_res_num, sp->origin.x, sp->origin.y, sp->origin.z, im->maxx, im->maxy, im->type);
#endif
  int mx = sp->max.x;
  int my = sp->max.y;
  sp->max.x = sp->origin.x + im->maxx;
  sp->max.y = sp->origin.y + im->maxy;
  int cl = clip_sprite(sp, clip, &blit, 0);
#ifdef DEBUG
  printf("Sprite blit: res = %d", cl);
  clip_print_rec(&blit);
#endif
  sp->max.x = mx;
  sp->max.y = my;
  if (!cl) 
    return;
  draw_image(&sp->origin, im, sp->x_flip, &blit);
#ifdef DEBUG
  char str[30];
  sprintf(str, "%x %d", sp->class, sp->image_res_num);
  //  graphics_print(sp->origin.x, sp->origin.y, str);
#endif
}

/** 
 * Перерисовка всех отображений относительно вычисленного окна clip_rec
 * Вычисленное окно clip_rec пересекается с окном вывода и вычисляется
 * окно отсечения blit_rec, внутри которого будет отрисовка всех новых спрайтов
 * Спрайты с отрицательным z или несортированные не рисуются.
 */
/*void render_views()
{
  view_t *s = view_list_head;
  sprite_t *spr;
  rectangle_t blit_rec;
  while (1) {
#ifdef DEBUG
    printf("View %x flags: %x flags2: %x\n", (int)(s - view_list_head), s->flags, s->flags2);
#endif
    if (!(s->flags & VIEW_HIDDEN)) {
      spr = sprites + s->view_sprite;
      if (clip_sprite(spr, &clip_rec, &blit_rec, 1)) {
#ifdef DEBUG
	printf("Blit rec: ");
	clip_print_rec(&blit_rec);
#endif
	if (!(s->flags2 & VIEW2_MOUSE)) {
	  printf("Set mouse flags\n");
	  exit(1);
	}
	spr = spr->next_in_view;
	while (spr) {
	  if (spr->state >= SPRITE_READY && spr->origin.z >= 0)
	    render_sprite(spr, &blit_rec);
	  spr = spr->next_in_view;
	}
      }
    }
    if (!s->next)
      break;
    s = (view_t *)(memory + s->next);
  }
  }*/

/** 
 * Визуализация спрайтов
 * Проверка отображения: если скрытое, то отрисовка не происходит
 * Попытка отсчеь все спрайты относительно окна текущего отображения
 * Вычисляется координатное окно пересечения всех новых спрайтов 
 * и текущего окна
 * Перерисовка всех окон в той части где были новые спрайты
 * @param view текущее окно
 * @param spr голова списка окна
 */
void render_sprites(view_t *view, sprite_t *spr)
{
  rectangle_t view_rec;
  if (view->flags & VIEW_HIDDEN)
    return;
  view_rec.min_x = 0;//spr->origin.x;
  view_rec.min_y = 0;//spr->origin.y;
  view_rec.max_x = 319;//spr->max.x;
  view_rec.max_y = 199;//spr->max.y;
  spr = spr->next_in_view;
  //  clip_reset_rec(&sprites_rec);
  while (spr) {
    if (spr->state >= SPRITE_READY && spr->origin.z >= 0)
      render_sprite(spr, &view_rec);
    spr = spr->next_in_view;
  }
  if (view->flags2 & VIEW2_NOBLIT) {
    printf("View: no blit\n");
    exit(1);
  }
}

/** 
 * Визуализация окна отображения
 * если нет новых спрайтов, рендеринг не происходит
 * если появились новые спрайты, то перерисовывается часть сцены
 * где появились новые спрайты или были изменения
 * @param view окно
 * @param sprite голова списка спрайтов
 */
void render_view(view_t *view, sprite_t *sprite)
{
  if (view->flags & VIEW_NOTTRANSLATED)
    view_translate(view, sprite);
  if (view->flags2 & VIEW2_FLAG1) {
    printf("view2 flag1\n");
    exit(1);
  }
  sprite_t *sc_sprite = sprite;
  sprite_t *prev = sprite;
  sprite = sprite->next_in_view;
  //  clip_reset_rec(&sprites_rec);
  while (sprite) {
    switch (sprite->state) {
    case SPRITE_READY:
      break;
    case SPRITE_NEW:
      sprite_object = sprite->object;
      sprite = project_sort_sprite(sc_sprite, prev, sprite);
      break;
    case SPRITE_UPDATED:
      sprite_object = sprite->object;
      //clip_update_rec(sprite, &sprites_rec);// прямоугольник рассчитан на предыдущую позицию объекта
      sprite = project_sort_sprite(sc_sprite, prev, sprite); // к нему добавляется позиция изменившегося объекта
      break;
    default: 
      sprite_object = sprite->object;
      sprite = delete_sprite(prev, sprite);
    }
    prev = sprite;
    sprite = sprite->next_in_view;
  }
  render_sprites(view, sc_sprite);
}

/// главный цикл визуализации
void render_all()
{
  view_t *s = view_list_head;
#ifdef DEBUG
  printf("skipping frames: %d\n", frames_to_skip);
#endif
  while (frames_to_skip > frame_num) {
    graphics_sleep();
    palette_update();
    frame_num++;
  }
  frame_num = 0;
  while (1) {
#ifdef DEBUG
    printf("Rendering view %x flags = %x flags2 = %x\n", (int)((byte *)s - memory), s->flags, s->flags2);
#endif
    if (!(s->flags & VIEW_HIDDEN))
      render_view(s, sprites + s->view_sprite);
    if (!s->next)
      break;
    s = (view_t *)(memory + s->next);
  }
#ifdef DEBUG
  printf("After rendering:\n");
  dump_sprites();
#endif
}

/// установка числа кадров, через сколько обновляется экран
void set_frames_to_skip()
{
  new_get();
  frames_to_skip = (byte)current_value;
#ifdef DEBUG
  printf("\t\tset frame num %d\n", frames_to_skip);
#endif
}
