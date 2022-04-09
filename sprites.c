/**
 * @file   sprites.c
 * @author alex <alex@localhost>
 * @date   Fri Apr  8 16:00:53 2022
 * 
 * @brief  Модуль для работы со спрайтами
 * 
 */
#include "sprites.h"

sprite_t *free_sprites;		/**< голова списка своболных спрайтов */

/** 
 * Возвращает свободное число ячеек в таблице спрайтов
 */
int sprites_free_num()
{
  int num = 0;
  sprite_t *s = free_sprites;
  while (s) {
    num++;
    s = s->next;
  }
  return num;
}

void sprites_set_translate(word *vec)
{
}

void sprites_translate(sprite_t *list, word *vec)
{
}
