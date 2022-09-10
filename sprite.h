#ifndef __SPRITE__
#define __SPRITE__
#include "types.h"
#include "vector.h"
#include "view.h"

/// состояния спрайтов
enum sprite_state_e {
  SPRITE_CURSOR = -2,		/**< спрайт курсора */
  SPRITE_NEW = -1,		/**< новый спрайт */
  SPRITE_READY = 0,		/**< спрайт готов к отрисовке */
  SPRITE_REMOVED = 1,		/**< спрайт будет удален при отрисовке */
  SPRITE_UPDATED = 2,	/**< спрайт после обновления координат или изображения */
};

/// структура спрайта
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
  int object;		/**< объект, которому принадлежит спрайт */
  int layer;			/**< слой определяет порядок отрисовки при одинаковой z координате, меньшее рисуется позднее */
  byte *image;			/**< установленное изображение спрайта */
  byte *render_image;			/**< изображение которое будет отрисовано */
  int image_res_num;		/**< номер изображения из ресурса */
  int class;			/**< класс объекта, который добавил спрайт */
  struct sprite_s *next;	/**< следующий в списке объекта спрайт */
  struct sprite_s *next_in_view;	/**< следующий спрайт в списке отображения */
  view_t *view;	/**< отображение, к которому принадлежит спрайт */
} sprite_t;

void sprites_init(int num);
int num_free_sprites();
void sprites_translate(vec_t *delta);
void set_sprites_object();
void set_sprites_layer();
void sprites_clear_with_tag();
void sprites_clear_with_tag_view();
void remove_all_sprites(sprite_t *sp, int remove);
void sprite_add(int num, vec_t *origin, int x_flip, int is_object, int tag);
void sprite_show();
void sprite_show_flipped();
void set_tag();
void sprites_clear_all_view();
void sprite_show_0();
void sprites_clear_all();
void dump_sprites();
void view_translate(view_t *view, sprite_t *c);
void sprite_show0_main();

extern sprite_t *sprites;		/**< таблица спрайтов */
extern sprite_t *free_sprite;		/**< последний свободный спрайт из таблицы */
extern sprite_t *cursor_sprite;	/**< спрайт курсора мыши */
extern vec_t current_origin;
extern int remove_from_view;	/**< нужно ли удалять из окна */

#endif
