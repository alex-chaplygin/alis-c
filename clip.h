#pragma once
#include "sprite.h"
/// координатное окно
typedef struct {
  int min_x;			/**< левый верхний угол */
  int min_y;
  int max_x;			/**< правый нижний угол */
  int max_y;
} rectangle_t;

void clip_print_rec(rectangle_t *rec);
void clip_reset_rec(rectangle_t *r);
void clip_update_rec(sprite_t *c, rectangle_t *r);
int clip_sprite(sprite_t *sc_sprite, rectangle_t *rec, rectangle_t *out, int round);
