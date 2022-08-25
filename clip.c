/**
 * @file   clip.c
 * @author alex <alex@localhost>
 * @date   Sat Aug  6 07:46:51 2022
 * 
 * @brief  Модуль вычисления отсечений спрайтов
 * 
 */
#include <stdio.h>
#include "clip.h"
#include "res.h"

/// Отладочная печать окна
void clip_print_rec(rectangle_t *rec)
{
  printf("min = (%d %d) max = (%d %d)\n", rec->min_x, rec->min_y, rec->max_x, rec->max_y);
}

/** 
 * Сброс минимальных и максимальных значений прямоугольника
 * 
 * @param r координатное окно
 */
void clip_reset_rec(rectangle_t *r)
{
  r->min_x = r->min_y = 32000;
  r->max_x = r->max_y = -32000;
  r->min_x = r->min_y = 0;
  r->max_x = 319;
  r->max_y = 199;
}

/** 
 * Добавление области вывода спрайта в координатное окно
 * 
 * @param c добавляемый спрайт
 * @param r окно
 */
void clip_update_rec(sprite_t *c, rectangle_t *r)
{
  image_t *im = (image_t *)c->render_image;
#ifdef DEBUG
  printf("prev rec: ");
  clip_print_rec(r);
#endif
  if (c->origin.z < 0)
    return;
  if (c->origin.x < r->min_x)
    r->min_x = c->origin.x;
  int max_x = c->origin.x + im->maxx;
  if (max_x > r->max_x)
    r->max_x = max_x;
  if (c->origin.y < r->min_y)
    r->min_y = c->origin.y;
  int max_y = c->origin.y + im->maxy;
  if (max_y > r->max_y)
    r->max_y = max_y;
#ifdef DEBUG
  printf("image rec: %d %d\n", im->maxx, im->maxy);
  printf("rec: ");
  clip_print_rec(r);
#endif
}

/** 
 * Определяет, попадает ли спрайт в заданное координатное окно и
 * вычисляет пересечение спрайта и окна
 * 
 * @param sc_sprite спрайт
 * @param rec координатное окно, по которому идет отсечение
 * @param out координатное окно, которое получилось в результате 
 * пересечения спрайта и окна
 * @param round если 1, то координаты x выравниваются по сетке 16
 * @return 0, если спрайт полностью выходят за пределы окна, иначе 1
 */
int clip_sprite(sprite_t *sc_sprite, rectangle_t *rec, rectangle_t *out, int round)
{
  // проверка, что регион отрисовки полностью выходит за пределы
  // окна вывода
  if (rec->min_x > sc_sprite->max.x) // для спрайта сцены center - это max
    return 0;
  if (rec->min_y > sc_sprite->max.y)
    return 0;
  if (rec->max_x < sc_sprite->origin.x)
    return 0;
  if (rec->max_y < sc_sprite->origin.y)
    return 0;
  /*  if (round)
    out->min_x = rec->min_x & 0xfffffff0; // округление до 16 в меньшую сторону
    else*/
    out->min_x = rec->min_x;
  // отсечение по окну сцены из спрайта сцены
  if (out->min_x < sc_sprite->origin.x)
    out->min_x = sc_sprite->origin.x;
  out->min_y = rec->min_y;
  if (out->min_y < sc_sprite->origin.y)
    out->min_y = sc_sprite->origin.y;
  /*  if (round)
    out->max_x = rec->max_x | 0xf; // округление до 16 - 1 в большую сторону
    else*/
    out->max_x = rec->max_x;
  if (out->max_x > sc_sprite->max.x)
    out->max_x = sc_sprite->max.x;
  out->max_y = rec->max_y;
  if (out->max_y > sc_sprite->max.y)
    out->max_y = sc_sprite->max.y;
  return 1;
}
