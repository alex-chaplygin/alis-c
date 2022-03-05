/**
 * @file   text.c
 * @author alex <alex@localhost>
 * @date   Sat Mar  5 18:39:01 2022
 * 
 * @brief  Команды - работа с текстом
 * 
 */

#include <stdio.h>
#include "interpret.h"
#include "get.h"
#include "vector.h"

vec_t text_pos;
int text_tag;

/** 
 * Установка параметров текста
 */
void set_text_params()
{
  new_get();
  int pr = current_value;
  new_get();
  int pr2 = current_value;
  new_get();
  int size = (byte)current_value;
  new_get();
  int h = (byte)current_value;
  new_get();
  int pr3 = current_value;
#ifdef DEBUG
  printf("set text params %d %d %d %d %d\n", pr, pr2, size, h, pr3);
#endif
}

/** 
 * Установка позиции для вывода текста
 * Установка номера объекта текста
 */
void set_text_pos()
{
  new_get();
  text_pos.x = current_value;
  new_get();
  text_pos.y = current_value;
  new_get();
  text_pos.z = current_value;
  new_get();
  text_tag = (byte)current_value;
#ifdef DEBUG
  printf("set text pos (%d %d %d) tag = %x\n", text_pos.x, text_pos.y, text_pos.z, text_tag);
#endif
}
