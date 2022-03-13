/**
 * @file   render.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 17:27:56 2022
 * 
 * @brief  Отрисовка спрайтов: сортировка, отсечение.
 * Вычисление областей, где нужно перерисовать спрайты.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "sprite.h"
#include "scene.h"
#include "memory.h"
#include "vector.h"
#include "image.h"
#include "draw.h"
#include "project.h"
#include "interpret.h"
#include "get.h"
#include "graphics.h"
#include "palette.h"

byte frames_to_skip;		/**< через сколько кадров обновляется экран */
byte frame_num = 0;			/**< текущий счетчик кадров */
rectangle_t sprites_rec;	/**< окно вывода новых спрайтов */
rectangle_t clip_rec;	/**< окно вывода новых спрайтов, отсеченное по окну сцены */
int sprite_thread_offset;	/**< поток, чьи спрайты обрабатываются вместе */

/// Сброс минимальных и максимальных значений окна новых спрайтов
void reset_sprites_rec()
{
  sprites_rec.min_x = sprites_rec.min_y = 32000;
  sprites_rec.max_x = sprites_rec.max_y = -32000;
}

/// Отладочная печать окна
void print_rec(rectangle_t *rec)
{
  printf("min = (%d %d) max = (%d %d)\n", rec->min_x, rec->min_y, rec->max_x, rec->max_y);
}

/// Добавление области вывода спрайта в координатное окно новых спрайтов
void update_sprites_rec(sprite_t *c)
{
  image_t *im = (image_t *)c->render_image;
#ifdef DEBUG
  printf("prev sprites rec: ");
  print_rec(&sprites_rec);
#endif
  if (c->origin.z < 0)
    return;
  if (c->origin.x < sprites_rec.min_x)
    sprites_rec.min_x = c->origin.x;
  int max_x = c->origin.x + im->maxx;
  if (max_x > sprites_rec.max_x)
    sprites_rec.max_x = max_x;
  if (c->origin.y < sprites_rec.min_y)
    sprites_rec.min_y = c->origin.y;
  int max_y = c->origin.y + im->maxy;
  if (max_y > sprites_rec.max_y)
    sprites_rec.max_y = max_y;
#ifdef DEBUG
  printf("image rec: %d %d\n", im->maxx, im->maxy);
  printf("sprites rec: ");
  print_rec(&sprites_rec);
#endif
}

/** 
 * Новый спрайт помещается на новое место в списке отрисовки спрайтов
 * Идет сортировка по убыванию z координаты, затем по порядку (order),
 * по убыванию тегов. Состояние спрайта становится - READY - готов к 
 * отрисовке
 * @param sc спрайт сцены
 * @param sprite текущий спрайт
 * @return спрайт перед которым добавляется новый, если новый спрайт в конце списка, то возвращается он
 */
sprite_t *sort_sprite(sprite_t *sc, sprite_t *sprite)
{
  sprite_t *prev;
  sprite_t* c = sc;
#ifdef DEBUG
  printf("sort sprite: origin(%d %d %d)center(%d %d %d)tag(%d)layer(%d)state(%d)\n", sprite->origin.x, sprite->origin.y, sprite->origin.z, sprite->center.x, sprite->center.y, sprite->center.z, sprite->tag, sprite->layer, sprite->state);
#endif
  do {
    prev = c;
    c = c->next_in_scene;
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
  sprite->next_in_scene = c;
  prev->next_in_scene = sprite;
  sprite->state = SPRITE_READY;
  if (prev != sc)
    return sc;
  else 
    return sprite;
}

/** 
 * Обработка нового спрайта: проецирование, сортировка.
 * 
 * @param sc спрайт сцены
 * @param prev предыдущий спрайт
 * @param c текущий спрайт
 */
sprite_t *process_sprite(sprite_t *sc, sprite_t *prev, sprite_t *c)
{
  // устанавливаем левый верхний угол после проецирования
  project_sprite(c, &c->origin);
  // удаление спрайта из списка сцены
  prev->next_in_scene = c->next_in_scene;
  // устанавливаем изображение отрисовки
  c->render_image = c->image;
  update_sprites_rec(c);
  sprite_t *s = sort_sprite(sc, c);
#ifdef DEBUG
   dump_sprites();
#endif
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
  update_sprites_rec(c);
  prev->next_in_scene = c->next_in_scene;
  c->next = free_sprite;
  free_sprite = c;
  return prev;
}

/** 
 * Обработка вновь добавленных спрайтов: проецирование, сортировка
 * 
 * @param sc_sprite голова списка спрайтов
 */
void process_new_sprites(sprite_t *sc_sprite)
{
  sprite_t *sprite = sc_sprite;
  sprite_t *prev;
#ifdef DEBUG
  printf("process new sprites\n");
#endif
  do {
    prev = sprite;
    sprite = sprite->next_in_scene;
    if (!sprite)
      break;
#ifdef DEBUG
    printf("sprite: origin(%d %d %d)center(%d %d %d)tag(%d)state(%d)\n", sprite->origin.x, sprite->origin.y, sprite->origin.z, sprite->center.x, sprite->center.y, sprite->center.z, sprite->tag, sprite->state);
#endif
    // обрабатываются спрайты только одного потока
    if (sprite_thread_offset != sprite->thread_offset)
      continue;
    if (sprite->state == SPRITE_READY)
      continue;
    else if (sprite->state < SPRITE_READY) {
      process_sprite(sc_sprite, prev, sprite);
    } else if (sprite->state == SPRITE_UPDATED) {
      update_sprites_rec(sprite);
      process_sprite(sc_sprite, prev, sprite);
    } else 
      delete_sprite(prev, sprite);
    sprite = prev;
  } while (1);
}

/** 
 * Определяет, попадает ли спрайт в заданное координатное окно и
 * вычисляет пересечение спрайта и окна
 * 
 * @param sc_sprite спрайт
 * @param rec координатное окно, по которому идет отсечение
 * @param out координатное окно, которое получилось в результате 
 * пересечения спрайта и окна
 * @param round если 1, то координаты x выравниваются по сетке 16
 * @return 0, если спрайт полностью выходят за пределы окна, иначе 1
 */
int clip_sprite(sprite_t *sc_sprite, rectangle_t *rec, rectangle_t *out, int round)
{
  // проверка, что регион отрисовки полностью выходит за пределы
  // окна вывода
  if (rec->min_x > sc_sprite->max.x) // для спрайта сцены center - это max
    return 0;
  if (rec->min_y > sc_sprite->max.y)
    return 0;
  if (rec->max_x < sc_sprite->origin.x)
    return 0;
  if (rec->max_y < sc_sprite->origin.y)
    return 0;
  if (round)
    out->min_x = rec->min_x & 0xfff0; // округление до 16 в меньшую сторону
  else
    out->min_x = rec->min_x;
  // отсечение по окну сцены из спрайта сцены
  if (out->min_x < sc_sprite->origin.x)
    out->min_x = sc_sprite->origin.x;
  out->min_y = rec->min_y;
  if (out->min_y < sc_sprite->origin.y)
    out->min_y = sc_sprite->origin.y;
  if (round)
    out->max_x = rec->max_x | 0xf; // округление до 16 - 1 в большую сторону
  else
    out->max_x = rec->max_x;
  if (out->max_x > sc_sprite->max.x)
    out->max_x = sc_sprite->max.x;
  out->max_y = rec->max_y;
  if (out->max_y > sc_sprite->max.y)
    out->max_y = sc_sprite->max.y;
  return 1;
}

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
  printf("Rendering sprite: origin(%d %d %d)size(%d %d)type(%x)\n", sp->origin.x, sp->origin.y, sp->origin.z, im->maxx, im->maxy, im->type);
#endif
  int mx = sp->max.x;
  int my = sp->max.y;
  sp->max.x = sp->origin.x + im->maxx;
  sp->max.y = sp->origin.y + im->maxy;
  int cl = clip_sprite(sp, clip, &blit, 0);
#ifdef DEBUG
  printf("Sprite blit: ");
  print_rec(&blit);
#endif
  sp->max.x = mx;
  sp->max.y = my;
  if (!cl) 
    return;
  draw_image(&sp->origin, im, sp->x_flip, &blit);
}

/** 
 * Перерисовка всех сцен относительно вычисленного окна clip_rec
 * Вычисленное окно clip_rec пересекается с окном сцены и вычисляется
 * окно отсечения blit_rec, внутри которого будет отрисовка всех новых спрайтов
 * Спрайты с отрицательным z или несортированные не рисуются.
 */
void render_all_scenes()
{
  scene_t *s = scene_list_head;
  sprite_t *spr;
  rectangle_t blit_rec;
  while (1) {
#ifdef DEBUG
    printf("Scene %x flags: %x flags2: %x\n", (int)(s - scene_list_head), s->flags, s->flags2);
#endif
    if (!(s->flags & SCENE_HIDDEN)) {
      spr = sprites + s->scene_sprite;
      if (clip_sprite(spr, &clip_rec, &blit_rec, 1)) {
#ifdef DEBUG
	printf("Blit rec: ");
	print_rec(&blit_rec);
#endif
	if (!(s->flags2 & SCENE2_MOUSE)) {
	  printf("Set mouse flags\n");
	  exit(1);
	}
	spr = spr->next_in_scene;
	while (spr) {
	  if (spr->state >= SPRITE_READY && spr->origin.z >= 0)
	    render_sprite(spr, &blit_rec);
	  spr = spr->next_in_scene;
	}
      }
    }
    if (!s->next)
      break;
    s = (scene_t *)(memory + s->next);
  }
}

/** 
 * Проверка сцены: если скрытая, то отрисовка не происходит
 * Попытка отсчеь все спрайты относительно окна текущей сцены
 * Вычисляется координатное окно пересечения всех новых спрайтов 
 * и окна текущей сцены
 * Перерисовка всех сцен в той части где были новые спрайты
 * @param scene текущая сцена
 * @param spr спрайт сцены
 */
void render_sprites(scene_t *scene, sprite_t *spr)
{
  if (scene->flags & SCENE_HIDDEN)
    return;
  int clip = clip_sprite(spr, &sprites_rec, &clip_rec, 1);
#ifdef DEBUG
  printf("Clip rec: ");
  print_rec(&clip_rec);
#endif
  if (!clip)
    return;
  render_all_scenes();
  if (scene->flags2 & SCENE2_NOBLIT) {
    printf("Scene: no blit\n");
    exit(1);
  }
}

/** 
 * Рендеринг сцены
 * если нет новых спрайтов, рендеринг не происходит
 * если появились новые спрайты, то перерисовывается часть сцены
 * где появились новые спрайты или было изменения
 * @param scene сцена
 * @param sprite голова списка холстов
 */
void render_scene(scene_t *scene, sprite_t *sprite)
{
  if (scene->flags & SCENE_NOTTRANSLATED)
    scene_translate(scene, sprite);
  if (scene->flags2 & SCENE2_FLAG1) {
    printf("scene2 flag1\n");
    exit(1);
  }
#ifdef DEBUG
   dump_sprites();
#endif
  sprite_t *sc_sprite = sprite;
  sprite_t *prev = sprite;
  sprite = sprite->next_in_scene;
  reset_sprites_rec();
  while (sprite) {
    switch (sprite->state) {
    case SPRITE_READY:
      break;
    case SPRITE_NEW:
      sprite_thread_offset = sprite->thread_offset;
      sprite = process_sprite(sc_sprite, prev, sprite);
      process_new_sprites(sprite);
      render_sprites(scene, sc_sprite);
      break;
    case SPRITE_UPDATED:
      sprite_thread_offset = sprite->thread_offset;
      update_sprites_rec(sprite);// прямоугольник рассчитан на предыдущую позицию объекта
      sprite = process_sprite(sc_sprite, prev, sprite); // к нему добавляется позиция изменившегося объекта
      process_new_sprites(sprite);
      render_sprites(scene, sc_sprite);
      break;
    default: 
      sprite_thread_offset = sprite->thread_offset;
      sprite = delete_sprite(prev, sprite);
      process_new_sprites(sprite);
      render_sprites(scene, sc_sprite);
    }
    prev = sprite;
    sprite = sprite->next_in_scene;
  }
#ifdef DEBUG
  printf("After rendering:\n");
  dump_sprites();
#endif
}

/// главный цикл рендеринга по сценам
void render_update()
{
  scene_t *s = scene_list_head;
  while (frames_to_skip > frame_num) {
    graphics_sleep();
    palette_update();
    frame_num++;
  }
  frame_num = 0;
  while (1) {
#ifdef DEBUG
    printf("Rendering scene %x flags = %x flags2 = %x\n", (int)((byte *)s - memory), s->flags, s->flags2);
#endif
    if (!(s->flags & SCENE_HIDDEN))
      render_scene(s, sprites + s->scene_sprite);
    if (!s->next)
      break;
    s = (scene_t *)(memory + s->next);
  }
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
