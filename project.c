/**
 * @file   project.c
 * @author alex <alex@localhost>
 * @date   Sun Feb 13 09:15:43 2022
 * 
 * @brief  Проецирование мировых координат в экранные
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "vector.h"
#include "sprite.h"
#include "scene.h"
#include "image.h"

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
 * @param origin спроецированные координаты левого верхнего угла спрайта
 */
void project_sprite(sprite_t *c, vec_t *origin)
{
  scene_t *sc = c->scene;
  vec_t *v = vec_new(c->center.x - sc->origin_x, c->center.y - sc->origin_y, c->center.z - sc->origin_z);
#ifdef DEBUG
  printf("projection: vec (%d %d %d)\n", (*v).x, (*v).y, (*v).z);
#endif
  if (sc->flags2 & SCENE2_3D) {
    printf("SCENE 3D\n");
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
  memcpy(origin, v, sizeof(vec_t));
  vec_delete(v);
#ifdef DEBUG
  // 0 -10 40
  printf("origin: (%d %d %d)\n", origin->x, origin->y, origin->z);
#endif
}
