#include "res.h"

/// координатное окно
typedef struct {
  int min_x;			/**< левый верхний угол */
  int min_y;
  int max_x;			/**< правый нижний угол */
  int max_y;
} rectangle_t;

void draw_image(vec_t *origin, image_t *im, int x_flip, rectangle_t *blit);
