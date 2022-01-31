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

#define TAG_FLAG1 (1 << 1)
#define TAG_FLAG2 (1 << 2)
#define TAG_3D (1 << 7)

/// координатное окно
typedef struct {
  int min_x;			/**< левый верхний угол */
  int min_y;
  int max_x;			/**< правый нижний угол */
  int max_y;
} rectangle_t;

byte frame_num;
rectangle_t sprites_rec;	/**< окно вывода спрайтов */
rectangle_t clip_rec;	/**< окно отсеченных по сцене спрайтов */
vec_t origin;			/**< оконные координаты левого верхнего угла объекта */
int draw_region_updated;

/** 
 * Перемещает начало координат сцены
 * 
 * @param scene сцена
 * @param c спрайт сцены
 */
void scene_apply_delta(scene_t *scene, sprite_t *c)
{
  scene->origin_x += scene->delta_x;
  scene->origin_y += scene->delta_y;
  scene->origin_z += scene->delta_z;
  scene->delta_x = scene->delta_y = scene->delta_z = 0;
  c = c->next_in_scene;
  while (c) {
    if (c->state == SPRITE_PROJECTED)
      c->state = SPRITE_TRANSLATED;
    c = c->next_in_scene;
  }
  scene->flags &= ~SCENE_NOTUPDATED;
#ifdef DEBUG
  printf("scene apply delta: (%d %d %d) flags = %x\n", scene->origin_x, scene->origin_y, scene->origin_z, scene->flags);
#endif  
}

/// установка начальных значений минимума и максимума спрайтов
void reset_min_max()
{
  sprites_rec.min_x = sprites_rec.min_y = 32000;
  sprites_rec.max_x = sprites_rec.max_y = -32000;
}

void print_rec(rectangle_t sprites_rec)
{
  printf("min = (%d %d) max = (%d %d)\n", sprites_rec.min_x, sprites_rec.min_y, sprites_rec.max_x, sprites_rec.max_y);
}

/// обновляем минимальные и макисмальные (x, y) координаты спрайтов
void update_min_max(sprite_t *c)
{
  image_t *im = (image_t *)c->image;
  if (c->min.z < 0)
    return;
  if (c->min.x <= sprites_rec.min_y)
    sprites_rec.min_x = c->min.x;
  int max_x = c->min.x + im->maxx;
  if (max_x >= sprites_rec.max_x)
    sprites_rec.max_x = max_x;
  if (c->min.y <= sprites_rec.min_y)
    sprites_rec.min_y = c->min.y;
  int max_y = c->min.y + im->maxy;
  if (max_y >= sprites_rec.max_y)
    sprites_rec.max_y = max_y;
#ifdef DEBUG
  printf("sprites rec: ");
  print_rec(sprites_rec);
#endif
}

/// арифметический сдвиг вправо
int sar(int a, int c)
{
  int sign = a & (1 << 31);
  int res = a;
  for (int i = 0; i < c; i++)
    res = (res >> 1) | sign;
  return res;
}

/** 
 * Преобразование из мировых в оконные координаты
 * 
 * @param c спрайт
 */
void project_sprite(sprite_t *c)
{
  scene_t *sc = c->scene;
  vec_t *v = vec_new(c->max.x - sc->origin_x, c->max.y - sc->origin_y, c->max.z - sc->origin_z);
#ifdef DEBUG
  printf("projection: vec (%d %d %d)\n", (*v).x, (*v).y, (*v).z);
#endif
  if (sc->tag & TAG_3D) {
    printf("TAG 3D\n");
    exit(1);
  }
  int y = (*v).y;
  // оси y и z меняются местами, z направлена снизу вверх
  (*v).y = sc->ay * (*v).y - (*v).z;
  (*v).z = y;
#ifdef DEBUG
  //  printf("after matrix: (%d %d %d)\n", (*v).x, (*v).y, (*v).z);
#endif
  int vec_z = 0;
  int vec_zz = sc->f24;
  if (vec_zz < 0) {
    // перемещение через матрицу проекции
    (*v).x = sar((*v).x, sc->f23) + sc->f5;
    (*v).y = sar((*v).y, sc->f23) + sc->f7;
#ifdef DEBUG
    //  printf("sar: f23 = %d f5 = %d f7 = %d (%d %d %d)\n", sc->f23, sc->f5, sc->f7, (*v).x, (*v).y, (*v).z);
#endif
    image_t *im = (image_t *)c->image;
    // проверка типа изображения
    // если тип не 3
    if (im->type == 3) {
      printf("image type 3\n");
      exit(1);
    }
    // проверка c->f24 == 0
    if (c->f24 != 0) {
      printf("c->f24 != 0 %x\n", c->f24);
      exit(1);
    }
    // был задан центр изображения
    (*v).x -= im->maxx / 2; 
    (*v).y -= im->maxy / 2; 
  } else {
    printf("vec_zz >= 0 not implemented\n");
    exit(1);
  }
  memcpy(&origin, v, sizeof(vec_t));
  vec_delete(v);
#ifdef DEBUG
  // 0 -10 40
  printf("origin: (%d %d %d)\n", origin.x, origin.y, origin.z);
#endif
}

/** 
 * Новый спрайт помещается на новое место в списке отрисовки спрайтов
 * Идет сортировка по убыванию z координаты, затем по порядку (order),
 * по убыванию тегов
 * @param sc холст сцены
 * @param sprite текущий холст
 */
void sort_sprite(sprite_t *sc, sprite_t *sprite)
{
  sprite_t *prev;
  sprite_t* c = sc;
#ifdef DEBUG
  printf("sprite to tail: min(%d %d %d)max(%d %d %d)tag(%d)state(%d)\n", sprite->min.x, sprite->min.y, sprite->min.z, sprite->max.x, sprite->max.y, sprite->max.z, sprite->tag, sprite->state);
#endif
  do {
    prev = c;
    c = c->next_in_scene;
    if (!c)
      break;
#ifdef DEBUG
    printf("spr: min(%d %d %d)max(%d %d %d)tag(%d)state(%d)\n", c->min.x, c->min.y, c->min.z, c->max.x, c->max.y, c->max.z, c->tag, c->state);
#endif
    if (c->state < SPRITE_PROJECTED) // спрайт помещается после новых
      continue;
    if (sprite->min.z < c->min.z) // сортировка по z с уменьшением z
      continue;
    if (sprite->min.z > c->min.z)
      break;
    if (sprite->layer < c->layer) // сортировка по слою по убыванию
      continue;
    if (sprite->layer > c->layer)
      break;
    if (sprite->tag < c->tag) // сортировка по тегам по убыванию
      continue;
    if (sprite->tag > c->tag)
      break;
    if (c->state == SPRITE_NEW)
      break;
  } while (c);
  sprite->next_in_scene = c;
  prev->next_in_scene = sprite;
  sprite->state = SPRITE_PROJECTED;
}

/** 
 * Обработка нового спрайта: проецирование, сортировка.
 * 
 * @param sc холст сцены
 * @param c текущий холст
 */
void process_sprite(sprite_t *sc, sprite_t *c)
{
  project_sprite(c);
  // удаление спрайта из списка сцены
  sc->next_in_scene = c->next_in_scene;
  // устанавливаем левый верхний угол после проецирования
  c->min.x = origin.x;
  c->min.y = origin.y;
  c->min.z = origin.z;
  update_min_max(c);
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
    printf("sprite: min(%d %d %d)max(%d %d %d)tag(%d)state(%d)\n", sprite->min.x, sprite->min.y, sprite->min.z, sprite->max.x, sprite->max.y, sprite->max.z, sprite->tag, sprite->state);    
    // thread offset?
    if (sprite->state != SPRITE_PROJECTED) {
      if (sprite->state < SPRITE_PROJECTED) {
	process_sprite(sc_sprite, sprite);
	sprite = sc_sprite->next_in_scene;
      } else {
	printf("render loop not sprite new\n");
	exit(1);
      }
    } else
      sprite = sprite->next_in_scene;
  }
}

/** 
 * Вычисление области отсечения по экранной области из спрайта сцены
 * 
 * @param sc_sprite спрайт сцены
 * 
 * @return 0, если спрайты полностью выходят за пределы окна, иначе 1
 */
int clip_sprites(sprite_t *sc_sprite)
{
  // проверка, что регион отрисовки полностью выходит за пределы
  // окна вывода
  if (sprites_rec.min_x > sc_sprite->max.x)
    return 0;
  if (sprites_rec.min_y > sc_sprite->max.y)
    return 0;
  if (sprites_rec.max_x < sc_sprite->min.x)
    return 0;
  if (sprites_rec.max_y < sc_sprite->min.y)
    return 0;
  clip_rec.min_x = sprites_rec.min_x & 0xfff0; // округление до 16 в меньшую сторону
  // отсечение по окну сцены из спрайта сцены
  if (clip_rec.min_x < sc_sprite->min.x)
    clip_rec.min_x = sc_sprite->min.x;
  clip_rec.min_y = sprites_rec.min_y;
  if (clip_rec.min_y < sc_sprite->min.y)
    clip_rec.min_y = sc_sprite->min.y;
  clip_rec.max_x = sprites_rec.max_x | 0xf; // округление до 16 - 1 в большую сторону
  if (clip_rec.max_x > sc_sprite->max.x)
    clip_rec.max_x = sc_sprite->max.x;
  clip_rec.max_y = sprites_rec.max_y;
  if (clip_rec.max_y > sc_sprite->max.y)
    clip_rec.max_y = sc_sprite->max.y;
  return 1;
}

void render_sprites(scene_t *scene, sprite_t *spr)
{
  if (scene->flags & SCENE_HIDDEN)
    return;
  if (!draw_region_updated) {
    printf("draw region updated = 0\n");
    exit(1);
  }
  if (!clip_sprites(spr))
    return;
#ifdef DEBUG
  printf("Clip rec: ");
  print_rec(clip_rec);
#endif
  exit(1);
}

/** 
 * Рендеринг сцены, обход холстов
 * 
 * @param scene сцена
 * @param sprite голова списка холстов
 */
void render_scene(scene_t *scene, sprite_t *sprite)
{
  int tag_flag2;
  if (scene->flags & SCENE_NOTUPDATED)
    scene_apply_delta(scene, sprite);
  if (scene->tag & TAG_FLAG2)
    tag_flag2 = 1;
  if (scene->tag & TAG_FLAG1) {
    printf("layer flag1\n");
    exit(1);
  }
  sprite_t *sc_sprite = sprite;
  sprite = sprite->next_in_scene;
  if (!sprite)
    return;
  if (sprite->state != SPRITE_NEW) {
    printf("sprite state not new = %d\n", sprite->state);
    exit(1);
  }
  draw_region_updated = 0;
  reset_min_max();
  process_sprite(sc_sprite, sprite);
  process_new_sprites(sc_sprite);
  draw_region_updated = 1;
  render_sprites(scene, sc_sprite);
}

/// главный цикл рендеринга по сценам
void render_update()
{
  scene_t *s = scene_list_head;
  while (1) {
#ifdef DEBUG
    printf("Rendering scene %x flags = %x\n", (int)((byte *)s - memory), s->flags);
#endif
    if (!(s->flags & SCENE_HIDDEN))
      render_scene(s, sprites + s->scene_sprite);
    if (!s->next)
      break;
    s = (scene_t *)(memory + s->next);
  }
  exit(1);
}
