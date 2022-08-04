/**
 * @file   res.c
 * @author alex <alex@localhost>
 * @date   Thu Aug  4 12:07:19 2022
 * 
 * @brief  Работа с ресурсами io файлов
 * 
 */
#include "interpret.h"

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
  byte *script = run_thread->script;
  if (load_main_res)
    script = main_thread->script;
  script_t *h = (script_t *)script;
  resource_table_t *r = (resource_table_t *)(script + h->resources);
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
 * Загрузка формы из ресурсов
 * 
 * @param script образ класса
 * @param num номер формы
 * 
 * @return загруженная форма
 *//*
byte *res_get_form(byte *script, int num)
{
  resource_table_t *r = (resource_table_t *)(script + h->resources);
#ifdef DEBUG
    printf("load form: num %d\n", num);
#endif
  if (num >= r->form_count) {
    printf("load_form: num %d > total %d\n", num, r->form_count);
    exit(1);
  }
  byte *pos = (byte *)r + r->form_table + num * sizeof(word);
  return (form_t *)pos;
  }*/
