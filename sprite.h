#ifndef __SPRITE__
#define __SPRITE__
#include "types.h"
#include "vector.h"
#include "scene.h"

#define SPRITE_CURSOR -2		/**< спрайт курсора */
#define SPRITE_NEW -1		/**< новый спрайт */
#define SPRITE_PROJECTED 0		/**< спрайт после проецирования на экран */
#define SPRITE_REMOVED 1
#define SPRITE_TRANSLATED 2	/**< спрайт после трансформации перемещения */

typedef struct sprite_s {
  vec_t min;			/**< минимальная точка спрайта */
  vec_t max;			/**< максимальная точка спрайта или центр спрайта (в сценарии задаются координаты центра спрайта)*/
  int tag;			/**< тег спрайта */
  int x_flip;			/**< 1 - зеркальное отражение спрайта */
  int state;			/**< состояние спрайта*/
  int f24;
  int f20;
  byte *image;			/**< данные изображения */
  struct sprite_s *next;	/**< следующий в списке потока спрайт */
  struct sprite_s *next_in_scene;	/**< следующий в списке сцены */
  scene_t *scene;	/**< сцена к которой принадлежит спрайт */
} sprite_t;

void sprites_init(int num);
int num_free_sprites();
void set_translate(word *data);
void sprites_translate(word *data);
sprite_t *sprite_find(int tag);
void sprite_set(sprite_t *c, byte *image, int x_flip, vec_t *coord);
void sprite_new_insert(sprite_t *c, int tag, byte *image, int x_flip, vec_t *coord);
sprite_t *sprite_next_on_tag(sprite_t *c, int tag);
sprite_t *sprite_remove(sprite_t *c);
void dump_sprites();
void clear_sprites();

extern sprite_t *sprites;		/**< таблица спрайтов */
extern sprite_t *free_sprite;		/**< последний свободный спрайт из таблицы */

#endif