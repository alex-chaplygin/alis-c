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
