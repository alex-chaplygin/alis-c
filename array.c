/**
 * @file   array.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 12:28:04 2022
 * 
 * @brief  Работа с массивами
 * 
 *  Массив:
 * размер_n(2байта), ..., размер_1(2 байта), число измерений - 1 (1 байт) (здесь указатель массива) данные 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpret.h"
#include "memory.h"

/** 
 * Создание нового массива
 */
void array_new()
{
  word adr = fetch_word();
  byte *pos = seg_read(run_object->data, adr) - 1;
  byte dim = fetch_byte();
  *pos-- = dim;
  byte d = fetch_byte();
  *pos = d;
  word *pos2 = (word *)pos;
  pos2--;
  for (int i = 0; i < dim; i++)
    *pos2-- = fetch_word();
#ifdef DEBUG
  printf("new array var_%x[%d", adr, d);
  pos2 = (word *)pos;
  pos2--;
  for (int i = 0; i < dim; i++)
    printf(", %d", *pos2--);
  printf("]\n");
#endif
}

/** 
 * Получает позицию данных в массиве
 * Индекс массива - в аккумуляторе (другие индексы если есть - в стеке)
 * @param arr указатель массива
 * @param str 1 - если для строки
 * @param size размер элемента массива
 * 
 * @return адрес строки
 */
byte *array_pos(byte *arr, int str, int size)
{
  byte *pos = arr;
  byte dim_count = *(arr - 1);
  word *dim = (word *)(arr - 2);
  word idx;
  if (str) {
    idx = *(byte *)dim;
    pos += idx * current_value;
  } else
    pos += current_value * size;
  if (dim_count)
    do {
      idx = stack_pop(&stack);
     --dim;
     idx *= *dim;
     pos += idx;
    } while (--dim_count);
  return pos;
}

/** 
 * Заполняет массив константными значениями
 * 
 */
void array_set()
{
  word arr = fetch_word();
  word count = fetch_word() - 1;
  memcpy(seg_read(run_object->data, arr), current_ip, count);
  current_ip += count;
#ifdef DEBUG
  printf("array set %x %x\n", arr, count);
  dump_mem(seg_read(run_object->data, arr), count);
#endif
}
