#ifndef SPRITES
#define SPRITES

#include "types.h"

typedef struct sprite_s {
  struct sprite_s *next;	/**< указатель на следующий в списке (список потока или список свободных ячеек) */
} sprite_t;

int sprites_free_num();
void sprites_set_translate(word *vec);
void sprites_translate(sprite_t *list, word *vec);

#endif
