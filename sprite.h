#ifndef __SPRITE__
#define __SPRITE__
#include "types.h"
#include "vector.h"
#include "window.h"

/// состояния спрайтов
enum sprite_state_e {
  SPRITE_CURSOR = -2,		/**< спрайт курсора */
  SPRITE_NEW = -1,		/**< новый спрайт */
  SPRITE_READY = 0,		/**< спрайт готов к отрисовке */
  SPRITE_REMOVED = 1,		/**< спрайт будет удален при отрисовке */
  SPRITE_UPDATED = 2,	/**< спрайт после обновления координат или изображения */
};
  
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
  int f1c;
  int object_offset;		/**< поток, которому принадлежит спрайт */
  int layer;			/**< слой определяет порядок отрисовки при одинаковой z координате, меньшее рисуется позднее */
  byte *image;			/**< установленное изображение спрайта */
  byte *render_image;			/**< изображение которое будет отрисовано */
  struct sprite_s *next;	/**< следующий в списке потока спрайт */
  struct sprite_s *next_in_window;	/**< следующий в списке отрисовки */
  window_t *window;	/**< сцена к которой принадлежит спрайт */
} sprite_t;

void sprites_init(int num);
int num_free_sprites();
void set_translate(word *data);
void sprites_translate(word *data);
int sprite_find(int tag, sprite_t **c);
void sprite_set(sprite_t *c, byte *image, int x_flip, vec_t *coord);
void sprite_new_insert(sprite_t *c, int tag, byte *image, int x_flip, vec_t *coord);
int sprite_next_on_tag(sprite_t *c, int tag, sprite_t **c2);
sprite_t *sprite_remove(sprite_t *c, int remove);
void dump_sprites(window_t *sc);
void clear_sprites_tag();
void window_translate(window_t *window, sprite_t *c);
void remove_all_sprites(sprite_t *sp, int remove);
void clear_sprites_from_window();
void set_coord_origin();
void move_coord_origin();
void show_sprite();
void show_sprite_flipped();
void set_tag();
void clear_all_sprites();
void show_sprite_0();
void clear_all_sprites2();

extern sprite_t *sprites;		/**< таблица спрайтов */
extern sprite_t *free_sprite;		/**< последний свободный спрайт из таблицы */
extern sprite_t *cursor_sprite;	/**< спрайт курсора мыши */
extern int remove_from_window;	/**< нужно ли удалять из окна */

#endif
