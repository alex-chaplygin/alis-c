/**
 * @file   render.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 17:27:56 2022
 * 
 * @brief  Рендеринг сцен
 * 
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

byte frame_num;
rectangle_t sprites_rec;	/**< окно вывода спрайтов */
rectangle_t clip_rec;	/**< окно отсеченных по сцене спрайтов */

/// Сброс минимальных и максимальных значений окна спрайтов
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

/// обновляем минимальные и макисмальные (x, y) координаты окна спрайтов
void update_sprites_rec(sprite_t *c)
{
  image_t *im = (image_t *)c->image;
  if (c->origin.z < 0)
    return;
  if (c->origin.x <= sprites_rec.min_y)
    sprites_rec.min_x = c->origin.x;
  int max_x = c->origin.x + im->maxx;
  if (max_x >= sprites_rec.max_x)
    sprites_rec.max_x = max_x;
  if (c->origin.y <= sprites_rec.min_y)
    sprites_rec.min_y = c->origin.y;
  int max_y = c->origin.y + im->maxy;
  if (max_y >= sprites_rec.max_y)
    sprites_rec.max_y = max_y;
#ifdef DEBUG
  printf("sprites rec: ");
  print_rec(&sprites_rec);
#endif
}

/** 
 * Новый спрайт помещается на новое место в списке отрисовки спрайтов
 * Идет сортировка по убыванию z координаты, затем по порядку (order),
 * по убыванию тегов
 * @param sc холст сцены
 * @param sprite текущий холст
 * @return спрайт перед которым добавляется новый, если новый спрайт в конце списка, то возвращается он
 */
sprite_t *sort_sprite(sprite_t *sc, sprite_t *sprite)
{
  sprite_t *prev;
  sprite_t* c = sc;
#ifdef DEBUG
  printf("sprite to tail: origin(%d %d %d)center(%d %d %d)tag(%d)layer(%d)state(%d)\n", sprite->origin.x, sprite->origin.y, sprite->origin.z, sprite->center.x, sprite->center.y, sprite->center.z, sprite->tag, sprite->layer, sprite->state);
#endif
  do {
    prev = c;
    c = c->next_in_scene;
    if (!c)
      break;
#ifdef DEBUG
    printf("spr: origin(%d %d %d)center(%d %d %d)tag(%d)state(%d)\n", c->origin.x, c->origin.y, c->origin.z, c->center.x, c->center.y, c->center.z, c->tag, c->state);
#endif
    if (c->state < SPRITE_SORTED) // спрайт помещается после новых
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
  sprite->state = SPRITE_SORTED;
  if (c != sc)
    return sc;
  else {
    printf("sort sprites: insert before scene sprite\n");
    exit(1);
    return sprite;
  }
}

/** 
 * Обработка нового спрайта: проецирование, сортировка.
 * 
 * @param sc холст сцены
 * @param c текущий холст
 */
void process_sprite(sprite_t *sc, sprite_t *c)
{
  // устанавливаем левый верхний угол после проецирования
  project_sprite(c, &c->origin);
  // удаление спрайта из списка сцены
  sc->next_in_scene = c->next_in_scene;
  update_sprites_rec(c);
  sort_sprite(sc, c);
#ifdef DEBUG
   dump_sprites();
#endif
}

/** 
 * Обработка вновь добавленных спрайтов: проецирование, сортировка
 * 
 * @param sc_sprite голова списка спрайтов
 */
void process_new_sprites(sprite_t *sc_sprite)
{
  sprite_t *sprite = sc_sprite->next_in_scene;
  while (sprite) {
    printf("sprite: origin(%d %d %d)center(%d %d %d)tag(%d)state(%d)\n", sprite->origin.x, sprite->origin.y, sprite->origin.z, sprite->center.x, sprite->center.y, sprite->center.z, sprite->tag, sprite->state);    
    // thread offset?
    if (sprite->state != SPRITE_SORTED) {
      if (sprite->state < SPRITE_SORTED) {
	process_sprite(sc_sprite, sprite);
	sprite = sc_sprite->next_in_scene;
      } else {
	printf("process_new_sprites: not sprite new\n");
	exit(1);
      }
    } else
      sprite = sprite->next_in_scene;
  }
}

/** 
 * Определяет, попадает ли спрайт в координатное окно и
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

void render_sprite(sprite_t *sp, rectangle_t *clip)
{
  rectangle_t blit;
  image_t *im = (image_t *)sp->image;
#ifdef DEBUG
  printf("Rendering sprite: origin(%d %d %d)size(%d %d)\n", sp->origin.x, sp->origin.y, sp->origin.z, im->maxx, im->maxy);
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
      if (clip_sprite(spr, &clip_rec, &blit_rec, 0)) {
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
	  if (spr->state >= SPRITE_SORTED && spr->origin.z >= 0)
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

void render_sprites(scene_t *scene, sprite_t *spr)
{
  if (scene->flags & SCENE_HIDDEN)
    return;
  if (!clip_sprite(spr, &sprites_rec, &clip_rec, 1))
    return;
#ifdef DEBUG
  printf("Clip rec: ");
  print_rec(&clip_rec);
#endif
  render_all_scenes();
  if (scene->flags2 & SCENE2_NOBLIT) {
    printf("Scene: no blit\n");
    exit(1);
  }
}

/** 
 * Рендеринг сцены
 * если нет новых спрайтов, рендеринг не происходит
 * если появились новые спрайты, то сцена перерисовывается
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
  sprite_t *sc_sprite = sprite;
  sprite = sprite->next_in_scene;
  while (sprite) {
    switch (sprite->state) {
    case SPRITE_SORTED:
      break;
    case SPRITE_NEW:
      reset_sprites_rec();
      process_sprite(sc_sprite, sprite);
      process_new_sprites(sc_sprite);
      render_sprites(scene, sc_sprite);
      break;
    default: 
      printf("sprite state = %d\n", sprite->state);
      exit(1);
    }
    sprite = sprite->next_in_scene;
  }
}

/// главный цикл рендеринга по сценам
void render_update()
{
  scene_t *s = scene_list_head;
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
  exit(1);
}
