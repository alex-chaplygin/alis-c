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
#include <string.h>
#include "file.h"
#include "interpret.h"
#include "get.h"

FILE *handle;			/**< текущий файл с которым идет работа */
char file_name[20];		/**< имя файла */

/// преобразование строки в верхний регистр
void uppercase(char *s)
{
    char *c = s;
    while (*c) {
      *c = toupper(*c);
      c++;
    }
}

/// преобразование строки в нижний регистр
void lowercase(char *s)
{
    char *c = s;
    while (*c) {
      *c = tolower(*c);
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
void file_open(char *name, int mode)
{
  char rw[] = "r+b";
  char *m;
  strcpy(file_name, name);
  lowercase(file_name);
  switch (mode) {
  case 2: // открытие на чтение и запись
    m = rw;
    break;
  default:
    printf("unknown mode: %d\n", mode);
    exit(1);
  }
#ifdef DEBUG
  printf("file open: %s mode = %s\n", file_name, m);
#endif
  handle = fopen(file_name, m);
  if (!handle) {
    uppercase(file_name);
    handle = fopen(file_name, m);
    if (!handle) {
      fprintf(stderr, "File: %s not found\n", file_name);
      exit(1);
    }
  }
#ifdef DEBUG
  printf("file open: %s mode = %s\n", file_name, m);
#endif
}

/** 
 * Чтение из файла с обработкой ошибок
 * 
 * @param buf адрес буфера куда происходит чтение
 * @param size размер буфера
 */
void file_read(void *buf, int size)
{
  int c = fread(buf, size, 1, handle);
  if (c < 0) {
    fprintf(stderr, "File: read error size = %d, read bytes: %d\n", size, c);
    exit(1);
  }
}

/** 
 * Закрытие текущего файла
 */
void file_close()
{
  fclose(handle);
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
  lowercase(s);
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
  char *file;
  int mode;
  if (*current_ip != 0xff) {
    file = current_ip;
    while (*current_ip++) ;
    mode = fetch_word();
  } else {
    current_ip++;
    switch_string_get();
    new_get();
    file = get_string;
    mode = current_value;
  }
  #ifdef DEBUG
  printf("open file '%s' mode = %d\n", file, mode);
  #endif
  file_open(file, mode);
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
  word *buf = (word *)seg_read(run_object->data, adr);
  word *b = buf;
  // проверяем верхнюю границу буфера
  seg_read(run_object->data, adr + count - 1);
  file_read(buf, count);
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

/// Команда - закрытие файла
void op_close_file()
{
#ifdef DEBUG
  printf("close file\n");
  file_close();
#endif
}

/// функция файл не существует
void file_not_exists()
{
  if (!file_exists(get_string))
    current_value = 0;
  else
    current_value = -1;
#ifdef DEBUG
  printf("file not exists: %s %d\n", get_string, current_value);
#endif
  if (!current_value)
    exit(1);
}
