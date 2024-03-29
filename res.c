/**
 * @file   res.c
 * @author alex <alex@localhost>
 * @date   Thu Aug  4 12:07:19 2022
 * 
 * @brief  Работа с ресурсами io файлов
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "get.h"

#pragma pack(1)

/// таблица ресурсов
typedef struct {
  dword image_table;		/**< смещение таблицы изображений */
  word image_count;		/**< число изображений */
  dword form_table;
  word form_count;
  dword sound_table;		/**< смещение таблицы звуков */
  word sound_count;		/**< число звуков */
} resource_table_t;

int load_main_res = 0;	/**< флаг загрузки ресурса из главного потока */

/** 
 * Получение данных изображения из таблицы ресурсов
 * 
 * @param num номер изображения
 * 
 * @return указатель на изображение
 */
byte *res_get_image(int num)
{
  byte *class = run_object->class;
  if (load_main_res)
    class = main_object->class;
  program_t *h = (program_t *)class;
  resource_table_t *r = (resource_table_t *)(class + h->resources);
#ifdef DEBUG
    printf("res_get_image: main %d num %d\n", load_main_res, num);
#endif
  if (num >= r->image_count) {
    printf("res_get_image: main %d num %d > total %d\n", load_main_res, num, r->image_count);
    exit(1);
  }
  byte *pos = (byte *)r + r->image_table + num * sizeof(dword);
  return pos + *(dword *)pos;
}

/** 
 * Загрузка звука из ресурсов
 * 
 * @param num номер звука
 * 
 * @return буфер звука
 */
byte *res_get_sound(int num)
{
  byte *class = run_object->class;
  if (load_main_res)
    class = main_object->class;
  program_t *h = (program_t *)class;
  resource_table_t *r = (resource_table_t *)(class + h->resources);
#ifdef DEBUG
    printf("res_get_sound: main %d num %d\n", load_main_res, num);
#endif
  if (num >= r->sound_count) {
    printf("res_get_sound: main %d num %d > total %d\n", load_main_res, num, r->image_count);
    exit(1);
  }
  byte *pos = (byte *)r + r->sound_table + num * sizeof(dword);
  pos += *(dword *)pos;
#ifdef DEBUG
  dump_mem(pos, 16);
#endif
  return pos;
}

/** 
 * Загрузка формы из ресурсов
 * 
 * @param class образ класса
 * @param num номер формы
 * 
 * @return загруженная форма
 */
byte *res_get_form(byte *class, int num)
{
  program_t *h = (program_t *)class;
  resource_table_t *r = (resource_table_t *)(class + h->resources);
#ifdef DEBUG
  printf("load form: num %d count %d \n", num, r->form_count);
#endif
  if (num >= r->form_count) {
    printf("load_form: num %d > total %d\n", num, r->form_count);
    exit(1);
  }
  byte *pos = (byte *)r + r->form_table;
  pos += *(word *)&pos[num * sizeof(word)];
#ifdef DEBUG
  dump_mem(pos, 16);
#endif
  return pos;
}

/** 
 * Удаление изображения
 * 
 */
void delete_image()
{
  new_get();
  switch_get();
#ifdef DEBUG
  printf("delete image %d %d\n", prev_value, current_value);
#endif
}
