#ifndef __VECTOR__
#define __VECTOR__

/**< вектор - 3 компонента */
typedef struct {
  int x;
  int y;
  int z;
} vec_t;

vec_t *vec_new(int x, int y, int z);
void vec_delete(vec_t *v);
void vec_add(vec_t *v1, vec_t *v2, vec_t *v3);
void vec_sub(vec_t *v1, vec_t *v2, vec_t *v3);

#endif
