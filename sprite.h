#ifndef __SPRITE__
#define __SPRITE__
#include "types.h"
#include "vector.h"
#include "scene.h"

/// состояния спрайтов
#define SPRITE_CURSOR -2		/**< спрайт курсора */
#define SPRITE_NEW -1		/**< новый спрайт */
#define SPRITE_READY 0		/**< спрайт готов к отрисовке */
#define SPRITE_REMOVED 1
#define SPRITE_UPDATED 2	/**< спрайт после обновления координат или изображения */

typedef struct sprite_s {
  vec_t origin;			/**< координаты левого верхнего угла спрайта */
  union {
  vec_t center;			/**< координаты центра спрайта */
  vec_t max;			/**< для спрайта сцены координаты правого нижнего угла окна сцены */
  };
  union {
    int tag;			/**< тег спрайта */
    int flags2;			/**< для спрайта сцены - флаги */
  };
  int x_flip;			/**< 1 - зеркальное отражение спрайта */
  int state;			/**< состояние спрайта*/
  int f24;
  int layer;			/**< слой определяет порядок отрисовки при одинаковой z координате, меньшее рисуется позднее */
  byte *image;			/**< установленное изображение спрайта */
  byte *render_image;			/**< изображение которое будет отрисовано */
  struct sprite_s *next;	/**< следующий в списке потока спрайт */
  struct sprite_s *next_in_scene;	/**< следующий в списке отрисовки */
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
void clear_object();
void scene_translate(scene_t *scene, sprite_t *c);

extern sprite_t *sprites;		/**< таблица спрайтов */
extern sprite_t *free_sprite;		/**< последний свободный спрайт из таблицы */

#endif
