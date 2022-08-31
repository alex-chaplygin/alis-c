/**
 * @file   path.c
 * @author alex <alex@localhost>
 * @date   Mon Aug 29 13:54:43 2022
 * 
 * @brief  Модуль поиска пути
 */

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "interpret.h"
#include "get.h"
#include "objects.h"
#include "vector.h"

/** 
 * Находит вектор между двумя точками
 * 
 * @param p1 точка 1
 * @param p2 точка 2
 * @param out вектор
 */
void make_vec(short *p1, short *p2, vec_t *out)
{
#ifdef DEBUG
  printf("p1 (%x %x %x) p2 (%x %x %x)\n", p1[0], p1[1], p1[2], p2[0], p2[1], p2[2]);
#endif
  out->x = p2[0] - p1[0];
  out->y = p2[1] - p1[1];
  out->z = p2[2] - p1[2];
}

/** 
 * Скалярное произведение векторов
 * 
 * @param v1 вектор 1
 * @param v2 вектор 2
 * 
 * @return скалярное произведение
 */
int dot_product(vec_t *v1, char *v2)
{
  return v1->x * v2[0] + v1->y * v2[1] + v1->z * v2[2];
}

/** 
 * Находит кратчайший путь от заданного объекта до указанного объекта
 * устанавливает вектор скорости текущего объекта
 */
void path_find_shortest()
{
  vec_t vec;
  char *min_path;
  char *cur_path;
  new_get();
  int obj_num = current_value;

#ifdef DEBUG
  printf("find shortest path to object num = %x\n", obj_num);
#endif
  if (obj_num == -1)
    return;
  if (obj_num % 6 != 0)
    exit(1);
  object_t *obj = objects_table[obj_num / 6].object;
  make_vec((short *)run_object->data->data, (short *)obj->data->data, &vec);
#ifdef DEBUG
  printf("vector (%x %x %x)\n", vec.x, vec.y, vec.z);
#endif
  if (!run_object->path_table) {
    printf("path table = 0\n");
    exit(1);
  }
  char *path = run_object->class + run_object->path_table;
  byte num = *path++ + 1;
#ifdef DEBUG
  printf("num paths = %x\n", num);
#endif
  run_object->data->data[8] = num;
  int max_dot = 0x80000000;
  cur_path = path;
  for (int i = 0; i < num; i++) {
    int dot = dot_product(&vec, cur_path);
    if (max_dot < dot) {
      max_dot = dot;
      min_path = cur_path;
    }
#ifdef DEBUG
    printf("path (%d %d %d)(%x %x %x) dot (%d, %x) max (%d, %x)\n", cur_path[0], cur_path[1], cur_path[2], cur_path[0], cur_path[1], cur_path[2], dot, dot, max_dot, max_dot);
#endif
    cur_path += 3;
  }
  run_object->data->data[8] = (min_path - path) / 3;
  run_object->data->data[9] = min_path[0];
  run_object->data->data[10] = min_path[1];
  run_object->data->data[11] = min_path[2];
#ifdef DEBUG
  printf("found path num = %x\n", run_object->data->data[8]);
  printf("min path = (%d %d %d) (%x %x %x)\n", min_path[0], min_path[1], min_path[2], min_path[0], min_path[1], min_path[2]);
#endif
}

/** 
 * Устанавливает путь по заданному номеру
 */
void path_set()
{
  new_get();
  if (!run_object->path_table) {
    printf("set path path table = 0\n");
    exit(1);
  }
  char *path = run_object->class + run_object->path_table + 1 + current_value * 3;
#ifdef DEBUG
  printf("path set (%x %x %x)\n", path[0], path[1], path[2]);
#endif
  run_object->data->data[9] = path[0];
  run_object->data->data[10] = path[1];
  run_object->data->data[11] = path[2];
}
