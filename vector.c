/**
 * @file   vector.c
 * @author alex <alex@localhost>
 * @date   Mon Jan 31 09:44:55 2022
 * 
 * @brief  Функции для работы с 3-х компонентным вектором
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "vector.h"
#include "memory.h"

vec_t *vec_new(int x, int y, int z)
{
  vec_t *temp_vec = xmalloc(sizeof(vec_t));
  temp_vec->x = x;
  temp_vec->y = y;
  temp_vec->z = z;
  return temp_vec;
}

void vec_delete(vec_t *v)
{
  if (!v) {
    printf("vec_delete: NULL\n");
    exit(1);
  }
  free(v);
}

/** 
 * Сложение векторов v3 = v1 + v2
 */
void vec_add(vec_t *v1, vec_t *v2, vec_t *v3)
{
  v3->x = v1->x + v2->x;
  v3->y = v1->y + v2->y;
  v3->z = v1->z + v2->z;
}

/** 
 * Вычитание векторов v3 = v1 - v2
 */
void vec_sub(vec_t *v1, vec_t *v2, vec_t *v3)
{
  v3->x = v1->x - v2->x;
  v3->y = v1->y - v2->y;
  v3->z = v1->z - v2->z;
}
