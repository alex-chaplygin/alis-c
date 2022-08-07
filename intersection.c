/**
 * @file   collision.c
 * @author alex <alex@localhost>
 * @date   Wed Aug  3 16:40:52 2022
 * 
 * @brief  Модуль проверки столкновений, пересечений
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "get.h"
#include "res.h"
#include "objects.h"
#include "store.h"

/** 
 * Устанавливает номер формы для объекта
 */
void set_form()
{
  new_get();
  run_object->form = current_value;
#ifdef DEBUG
  printf("obj set form = %x\n", current_value);
#endif
}

#define SWAP(x, y)\
  {\
  c = (x);\
  (x) = (y);\
  (y) = c;\
  }

/** 
 * Создает ограничивающий прямоугольник из формы с учетом
 * позиции объекта и отражения объекта
 * @param origin позиция объекта
 * @param flip отражение
 * @param f форма объекта
 * @param min полученный минимум
 * @param max полученный максимум
 */
void create_bbox(short *origin, int flip, form_t *f, vec_t *min, vec_t *max)
{
  vec_t org = {origin[0], origin[1], origin[2]};
  vec_t dist;
  int c;
  if (f->form_type == 0) {
    min->x = f->rect0[0];
    min->y = f->rect0[1];
    min->z = f->rect0[2];
    dist.x = f->rect0[3];
    dist.y = f->rect0[4];
    dist.z = f->rect0[5];
  } else if (f->form_type == 1) {
    min->x = f->rect1[0];
    min->y = f->rect1[1];
    min->z = f->rect1[2];
    dist.x = f->rect1[3];
    dist.y = f->rect1[4];
    dist.z = f->rect1[5];
  }
  if (flip) {
    min->x = -min->x;
    dist.x = -dist.x;
  }
#ifdef DEBUG
  printf("read from form (%x %x %x) (%x %x %x)\n", min->x, min->y, min->z,
	 dist.x, dist.y, dist.z);
#endif
  vec_add(min, &org, min);
#ifdef DEBUG
  printf("after add org (%x %x %x) \n", min->x, min->y, min->z);
#endif
  vec_add(min, &dist, max);
#ifdef DEBUG
  printf("max (%x %x %x) \n", max->x, max->y, max->z);
#endif
  if (min->x > max->x)
    SWAP(min->x, max->x);
  if (min->y > max->y)
    SWAP(min->y, max->y);
  if (min->z > max->z)
    SWAP(min->z, max->z);
#ifdef DEBUG
  printf("create bbox (%x %x %x) (%x %x %x)\n", min->x, min->y, min->z, max->x, max->y, max->z);
#endif
}

int check_bbox(short *origin, short *origin2, int flip, int flip2, form_t *f1, form_t *f2, word mask)
{
  vec_t min;
  vec_t max;
  vec_t min2;
  vec_t max2;  
  create_bbox(origin, flip, f1, &min, &max);
  if (f2->form_type < 0) {
    printf("f2 type -1\n");
    exit(1);
  }
  if (f2->mask & mask) {
    create_bbox(origin2, flip2, f2, &min2, &max2);
#ifdef DEBUG
    printf("f2->mask & mask != 0\n");
#endif
    if (max2.y < min.y || min2.y > max.y ||
	max2.z < min.z || min2.z > max.z ||
	max2.x < min.x || min2.x > max.x)
      return 0;
    else
      return 1;
  }
  return 0;
}

/** 
 * Загрузка формы. Нахождение столкновения.
 * 
 * @param angle угол
 * @param form номер формы
 */
void find_collision_list(short *origin, word mask, int form)
{
  objects_list_pos = objects_list;
  if (form < 0)
    goto end;
  form_t *f = (form_t *)res_get_form(run_object->class, form);
  form_t *f2;
  object_table_t *tab = objects_table;
  object_t *t;
  while (tab) {
    t = tab->object;
    int current = *(short *)seg_read(run_object->data, 6);
    int obj = *(short *)seg_read(t->data, 6);
#ifdef DEBUG
    printf("checking obj %x, form: %x\n", object_num(t), t->form);
#endif
    if (current == obj && t->form >= 0 && t != run_object) {
      f2 = (form_t *)res_get_form(t->class, t->form); // форма проверяемого объекта
      if ((char)f->form_type < 0) {
	printf("form type ff\n");
	exit(1);
      } else {
#ifdef DEBUG
	printf("check bbox, form type = %d\n", f->form_type);
#endif
	if (check_bbox(origin, (short *)t->data->data, run_object->x_flip,
		       t->x_flip, f, f2, mask)) {
	  printf("check_bbox = true\n");
	  *objects_list_pos++ = object_num(t);
	  if (!find_all_objects)
	    goto end;
	} else
	  printf("check_bbox = false\n");	  
      }
    }
    tab = tab->next;
  }
 end:  
  *objects_list_pos++ = -1;
}

/** 
 * Нахождение списка объектов, которые пересекаются с формой.
 * На входе: координаты объекта, угол и форма.
 */
void find_collision()
{
  short *origin = (short *)run_object->data->data;
  new_get();
  short mask = current_value;
  new_get();
  short form = current_value;
#ifdef DEBUG
  printf("find collision: origin (%x %x %x), mask (%x), form(%x)\n", origin[0], origin[1], origin[2], mask, form);
#endif
  find_collision_list(origin, mask, form);
  objects_list_pos = objects_list;
  object_store_next();
}
