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

/** 
 * Открытие или создание файла.
 * Пытается открыть файл с именем в верхнем регистре
 * @param file_name путь к файлу
 * @param mode режим открытия
 * 
 * @return указатель на файл
 */
void file_open(char *name, int mode)
{
  char rw[] = "r+b";
  char cr[] = "w+b";
  char ap[] = "a+b";
  char *m;
  strcpy(file_name, name);
  if (mode & FILE_CREATE)
    m = cr;
  else if (mode & FILE_APPEND)
    m = ap;
  else
    m = rw;
  handle = fopen(file_name, m);
  if (!handle) {
    uppercase(file_name);
    handle = fopen(file_name, m);
    if (!handle) {
      fprintf(stderr, "File: %s not found\n", file_name);
      exit(1);
    }
  }
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
 * Запись в файл с обработкой ошибок
 * 
 * @param buf адрес буфера который записывается
 * @param size размер буфера
 */
void file_write(void *buf, int size)
{
  int c = fwrite(buf, size, 1, handle);
  if (c < 0) {
    fprintf(stderr, "File: write error size = %d, read bytes: %d\n", size, c);
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
