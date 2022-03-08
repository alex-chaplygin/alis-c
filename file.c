/**
 * @file   file.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 14:07:11 2022
 * 
 * @brief  Работа с файлами
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "file.h"
#include "interpret.h"
#include "get.h"

#define MODE_SEEK 0x800		/**< после открытия файла переместить указатель */

FILE *handle;			/**< текущий файл с которым идет работа */

/// преобразование строки в верхний регистр
void uppercase(char *s)
{
    char *c = s;
    while (*c) {
      *c = toupper(*c);
      c++;
    }
}

/** 
 * Открытие файла.
 * Пытается открыть файл с именем в верхнем регистре
 * @param file_name путь к файлу
 * @param mode режим открытия
 * 
 * @return указатель на файл
 */
FILE *file_open(char *file_name, char *mode)
{
  FILE *f = fopen(file_name, mode);
  if (!f) {
    uppercase(file_name);
    f = fopen(file_name, mode);
    if (!f) {
      fprintf(stderr, "File: %s not found\n", file_name);
      exit(1);
    }
  }
  return f;
}

/** 
 * Чтение из файла с обработкой ошибок
 * 
 * @param f указатель на файл
 * @param buf адрес буфера куда происходит чтение
 * @param size размер буфера
 */
void file_read(FILE *f, void *buf, int size)
{
  int c = fread(buf, size, 1, f);
  if (c < 0) {
    fprintf(stderr, "File: read error size = %d, read bytes: %d\n", size, c);
    exit(1);
  }
}

/** 
 * Проверка на существование файла. 
 * Имя файла преобразуется в верхний регистр при необходимости
 * 
 * @param s имя файла
 * 
 * @return 1 если файл существует, иначе 0
 */
int file_exists(char *s)
{
  FILE *f = fopen(s, "rb");
  if (!f) {
    uppercase(s);
    f = fopen(s, "rb");
    if (!f)
      return 0;
  } 
  fclose(f);
  return 1;
}

/** 
 * Команда - открытие файла
 * имя файла в get_string
 */
void op_open_file()
{
  if (*current_ip != 0xff) {
    printf("open file skip name\n");
    exit(1);
  }
  current_ip++;
  switch_string_get();
  new_get();
  #ifdef DEBUG
  printf("open file '%s' mode = %d buf = %x\n", get_string, current_value, prev_value);
  #endif
  switch (current_value) {
  case 2: // открытие на чтение и запись
    handle = file_open(get_string, "r+b");
    break;
  default:
    printf("unknown mode: %d\n", current_value);
  }
  if (current_value & MODE_SEEK) {
    printf("file seek\n");
    exit(1);
  }
}

/// чтение из файла
void op_read_file()
{
  word adr = fetch_word();
  if (!adr) {
    printf("read file to main mem\n");
    exit(1);
  }
  word count = fetch_word();
  word *buf = (word *)seg_read(run_thread->data, adr);
  word *b = buf;
  // проверяем верхнюю границу буфера
  seg_read(run_thread->data, adr + count - 1);
  file_read(handle, buf, count);
  if (*((byte *)buf - 2) == 2) { // если это массив слов
    for (int i = 0; i < count / 2; i++) {// преобразуем в big endian
      *b = ((*b & 0xff) << 8) + (*b >> 8);
      b++;
    }
  }
#ifdef DEBUG
  printf("read from file: adr = %x count = %d\n", adr, count);
  if (adr == 0x1978 && count == 140)
    ASSERT(*(word *)(buf + 16 * 2), 0xb)
#endif
}
