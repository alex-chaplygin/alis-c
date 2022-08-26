/**
 * @file   collision.c
 * @author alex <alex@localhost>
 * @date   Wed Aug  3 16:40:52 2022
 * 
 * @brief  Модуль поиска объектов, которые пересекаются с заданным.
 *
 * Форма объекта задается минимальным и максимальным значением
 * координат относительно центра объекта (параллепипед).
 * Формы могут состоять из списка паралепипедов.
 */
#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "get.h"
#include "res.h"
#include "objects.h"
#include "store.h"

/** 
 * Устанавливает номер формы для текущего объекта
 */
void set_form()
{
  new_get();
  run_object->form = current_value;
#ifdef DEBUG
  printf("obj set form = %x\n", current_value);
#endif
}

/** 
 * Очищает форму у текущего объекта
 */
void clear_form()
{
  run_object->form = -1;
#ifdef DEBUG
  printf("obj clear form\n");
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

/** 
 * Проверка пересечения двух объектов
 * 
 * @param origin позиция первого объекта
 * @param origin2 позиция второго объекта
 * @param flip отражение первого объекта
 * @param flip2 отражение второго объекта
 * @param f1 форма первого объекта
 * @param f2 форма второго объекта
 * @param mask маска для отбора объектов (с какими искать пересечение)
 * 
 * @return 1 - объекты пересекаются, 0 - не пересекаются
 */
int objects_intersection(short *origin, short *origin2, int flip, int flip2, form_t *f1, form_t *f2, word mask)
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
    return 1;
  }
  return 0;
}

/** 
 * Поиск объектов, которые пересекаются с заданным объектом
 * 
 * @param origin позиция заданного объекта
 * @param mask маска, какие объекты учитывать в поиске
 * @param form форма исходного объекта
 */
void find_intersection_list(short *origin, word mask, int form)
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
	if (objects_intersection(origin, (short *)t->data->data, run_object->x_flip,
		       t->x_flip, f, f2, mask)) {
	  printf("check_bbox = true\n");
	  *objects_list_pos = object_num(t);
	  objects_list_pos[256] = f2->mask;
	  objects_list_pos++;
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
 * Нахождение списка объектов, которые пересекаются с текущим 
 * объектом. Первый найденный объект сохраняется в переменную.
 * На входе: маска (какие объекты искать) и форма (bbox).
 */
void find_intersection_list_cur_obj()
{
  short *origin = (short *)run_object->data->data;
  new_get();
  short mask = current_value;
  new_get();
  short form = current_value;
#ifdef DEBUG
  printf("find intersection list cur obj: origin (%x %x %x), mask (%x), form(%x)\n", origin[0], origin[1], origin[2], mask, form);
#endif
  find_intersection_list(origin, mask, form);
  objects_list_pos = objects_list;
  object_store_next();
}

/** 
 * Сохраняет в переменную маску последнего найденного объекта
 */
void get_last_object_mask()
{
  current_value = objects_list_pos[255];
#ifdef DEBUG
  printf("get last mask: %x\n", current_value);
#endif
  switch_string_store();
}

/** 
 * Нахождение списка объектов, которые пересекаются с формой
 * в заданной точке. Первый найденный объект сохраняется в переменную.
 * На входе: точка, маска (какие объекты искать) и форма (bbox).
 */
void find_intersection_list_point()
{
  short origin[3];
  new_get();
  origin[0] = current_value;
  new_get();
  origin[1] = current_value;
  new_get();
  origin[2] = current_value;
  new_get();
  short mask = current_value;
  new_get();
  short form = current_value;
#ifdef DEBUG
  printf("find intersection list point: origin (%x %x %x), mask (%x), form(%x)\n", origin[0], origin[1], origin[2], mask, form);
#endif
  find_intersection_list(origin, mask, form);
  objects_list_pos = objects_list;
  object_store_next();
}

/** 
 * Нахождение списка объектов, которые пересекаются с формой
 * на заданном векторе относительно текущего объекта. 
 * Первый найденный объект сохраняется в переменную.
 * На входе: вектор, маска (какие объекты искать) и форма (bbox).
 */
void find_intersection_list_vector()
{
  short origin[3];
  short *org = (short *)run_object->data->data;
  new_get();
  origin[0] = current_value + org[0];
  new_get();
  origin[1] = current_value + org[1];
  new_get();
  origin[2] = current_value + org[2];
  new_get();
  short mask = current_value;
  new_get();
  short form = current_value;
#ifdef DEBUG
  printf("find intersection list vector: origin (%x %x %x), mask (%x), form(%x)\n", origin[0], origin[1], origin[2], mask, form);
#endif
  find_intersection_list(origin, mask, form);
  objects_list_pos = objects_list;
  object_store_next();
}

/** 
 * Нахождение списка объектов, которые пересекаются с формой объекта
 * на заданном векторе относительно текущего объекта. 
 * Первый найденный объект сохраняется в переменную.
 * На входе: вектор, маска (какие объекты искать)
 */
void find_intersection_list_vector_cur_form()
{
  short origin[3];
  short *org = (short *)run_object->data->data;
  new_get();
  origin[0] = current_value + org[0];
  new_get();
  origin[1] = current_value + org[1];
  new_get();
  origin[2] = current_value + org[2];
  new_get();
  short mask = current_value;
  short form = run_object->form;
#ifdef DEBUG
  printf("find intersection list vector cur_form: origin (%x %x %x), mask (%x), form(%x)\n", origin[0], origin[1], origin[2], mask, form);
#endif
  find_intersection_list(origin, mask, form);
  objects_list_pos = objects_list;
  object_store_next();
}

/** 
 * Нахождение списка объектов, которые пересекаются с формой
 * на заданном векторе формы относительно текущего объекта. 
 * Первый найденный объект сохраняется в переменную.
 * На входе: форма, маска (какие объекты искать) и форма (bbox).
 */
void find_intersection_list_form()
{
  short origin[3];
  short *org = (short *)run_object->data->data;
  new_get();
  form_t *f = (form_t *)res_get_form(run_object->class, current_value);
  if (f->form_type == 0) {
    origin[0] = f->rect0[0] + org[0];
    origin[1] = f->rect0[1] + org[1];
    origin[2] = f->rect0[2] + org[2];
  } else if (f->form_type == 1) {
    origin[0] = f->rect1[0] + org[0];
    origin[1] = f->rect1[1] + org[1];
    origin[2] = f->rect1[2] + org[2];
  }
  new_get();
  short mask = current_value;
  new_get();
  short form = current_value;
#ifdef DEBUG
  printf("find intersection list form: origin (%x %x %x), mask (%x), form(%x)\n", origin[0], origin[1], origin[2], mask, form);
#endif
  find_intersection_list(origin, mask, form);
  objects_list_pos = objects_list;
  object_store_next();
}
