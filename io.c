/**
 * @file   io.c
 * @author alex <alex@localhost>
 * @date   Tue Apr  5 07:30:44 2022
 * 
 * @brief  Работа с IO файлом, ресурсы: графика, звук
 * 
 */

#include <stdlib.h>
#include <string.h>

#include "io.h"

/// таблица ресурсов
typedef struct {
  dword image_table;		/**< смещение таблицы изображений */
  word image_count;		/**< число изображений */
  dword anim_table;
  word anim_count;
  dword sound_table;		/**< смещение таблицы звуков */
  word sound_count;		/**< число звуков */
} resource_table_t;

enum {
  SOUND_TYPE1 = 1,
  SOUND_TYPE2 = 2,
};

typedef struct {
  byte type;			/**< тип звука */
  byte f1;
  word size;			/**< размер данных */
  word f4;
  byte packed;			/**< если 1 то звук упакован */
  byte pad[9];
} sound_header_t;

int total_io_files;		/**< максимальное число загруженных io файлов */
int loaded_io_files;		/**< число загруженных io файлов */
/// таблица смещений в данных звука
char sound_offset_table[] = {-34, -21, -13, -8, -5, -3, -2, -1, 1, 2, 3, 5, 8, 13, 21, 34};

/** 
 * Распаковка звуковых данных в io файле
 * Звуковые данные упакованы как закодированные (по 4 бит) смещения
 * @param io буфер данных io файла
 */
void io_unpack_sound(byte *io)
{
  io_header_t *h = (io_header_t *)io;
  resource_table_t *res = (resource_table_t *)(io + h->resources);
  dword *tab = (dword *)((byte *)res + res->sound_table);
  sound_header_t *data;
  byte *temp;
  byte *src;
  byte *dst;
  char ofs;
  int count;
  for (int i = 0; i < res->sound_count; i++) {
    data = (sound_header_t *)((byte *)tab + *tab);
    if (data->type == SOUND_TYPE2 || data->type == SOUND_TYPE1)
      if (data->packed == 1) {
	count = data->size - sizeof(sound_header_t) >> 1;
	temp = malloc(count);
	memcpy(temp, data + 1, count);
	src = temp;
	dst = (byte *)(data + 1);
	ofs = *(char *)src++;
	*dst++ = ofs;
	for (int j = 0; j < count - 1; j++) {
	  ofs += sound_offset_table[*src >> 4];
	  *dst++ = ofs;
	  ofs += sound_offset_table[*src++ & 0xf];
	  *dst++ = ofs;
	}
	dst[-1] = dst[-2] = 0;
	data->size -= 2;
	free(temp);
      }
    ++tab;
  }
}

/** 
 * Возвращает число свободных ячеек из таблицы загруженных io файлов
 */
int io_num_free_files()
{
  return total_io_files - loaded_io_files;
}
