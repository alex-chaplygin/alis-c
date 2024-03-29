/**
 * @file   str.c
 * @author alex <alex@localhost>
 * @date   Thu Aug  4 17:54:28 2022
 * 
 * @brief  Команды - работа со строками
 * 
 */

#include <stdio.h>
#include <string.h>
#include "interpret.h"
#include "get.h"

/*
void str_from_stack()
{
  char *dst = current_string;
  do {
    *dst++ = stack[stack_pos++];
    if (dst - current_string >= MAX_STR) {
      printf("MAX STR\n");
      exit(1);
    }
    if (stack_pos >= MAX_STACK) {
      printf("Stack is empty\n");
      exit(1);
    }
  } while (*(dst - 1) != 0);
}

void str_trim()
{
  str_from_stack();
  printf("str_trim %d %s\n", current_string, current_value & 0xff);
  current_string[current_value & 0xff] = 0;
  printf("res: %s\n", current_string);
}

void str_part()
{
  str_from_stack();
  printf("str_part %d %s\n", current_value, current_string);
  char *src = current_string + strlen(current_string)  - current_value;
  if (src < current_string) src = current_string;
  char *dst = current_string;
  strcpy(dst, src);
  printf("res: %s\n", current_string);
}

void str_pos()
{
  if (stack_pos >= MAX_STACK) {
    printf("Stack id empty\n");
    exit(1);
  }
  char *pos = current_string + stack[stack_pos];
  stack_pos += 2;
  str_from_stack();
  printf("str_pos %d %s\n", current_value, current_string);
  current_value -= 2;
  *current_string = *pos;
  if (*current_string && current_value < 0) *(current_string + 1) = 0;
  printf("res: %s\n", current_string);
  }*/

/// Вычисляет длину строки
void get_string_len()
{
  current_value = strlen(get_string);
#ifdef DEBUG
  printf("get_string_len \"%s\" res = %d\n", get_string, current_value);
#endif
}

/// условие - строки не равны
void str_neq()
{
  switch_string_get();
  printf("str_neq %s %s\n", get_string, text_string);
  if (strcmp(get_string, text_string))
    current_value = 0;
  else
    current_value = -1;
  printf("res: %d\n", current_value);
}

/** 
 * Конвертация числа в строку
 */
void num_to_str()
{
  sprintf(get_string, "%d", current_value);
#ifdef DEBUG
  printf("num_to_str %d '%s'\n", current_value, get_string);
#endif
}

/** 
 * Запись одного символа в строку
 */
void set_char()
{
  sprintf(get_string, "%c", (char)current_value);
#ifdef DEBUG
  printf("set  char: %s\n", get_string);
#endif
}

/** 
 * Запись строки в стек
 */
void str_push()
{
  char *c = get_string + strlen(get_string);
#ifdef DEBUG
  printf("str push: %s\n", get_string);
#endif
  do
    stack_push(&stack, *c--);
  while (c >= get_string);
}

/** 
 * Соединение двух строк
 */
void str_cat()
{
  char *src;
  char *str;
  switch_string_get();
#ifdef DEBUG
  printf("str cat %s %s\n", get_string, text_string);
#endif
  str = text_string + strlen(text_string);
  src = get_string;
  strcpy(str, src);
  str = get_string;
  get_string = text_string;
  text_string = str;
#ifdef DEBUG
  printf("res: %s\n", get_string);
#endif
}

/** 
 * Извлечение строки из стека
 */
void str_pop()
{
  char *str = get_string;
  get_string = text_string;
  text_string = str;
  do {
    *str++ = stack_pop(&stack);
  } while (*(str - 1) != 0);
#ifdef DEBUG
  printf("str pop %s %s\n", get_string, text_string);
#endif
}

/*
void str_char()
{
  current_value = (byte)*get_string;
  printf("str char: %c %d\n", (char)current_value, current_value);
}

void switch_str_get()
{
  char *str = get_string;
  get_string = text_string;
  text_string = str;
  get();
}

void str_eq()
{
  switch_str_get();
  printf("str_eq %s %s\n", get_string, text_string);
  if (!strcmp(get_string, text_string)) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_gt()
{
  switch_str_get();
  printf("str_gt %s %s\n", get_string, text_string);
  if (strcmp(get_string, text_string) > 0) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_lt()
{
  switch_str_get();
  printf("str_lt %s %s\n", get_string, text_string);
  if (strcmp(get_string, text_string) < 0) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_ge()
{
  switch_str_get();
  printf("str_ge %s %s\n", get_string, text_string);
  if (strcmp(get_string, text_string) >= 0) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_le()
{
  switch_str_get();
  printf("str_le %s %s\n", get_string, text_string);
  if (strcmp(get_string, text_string) <= 0) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_to_num()
{
  current_value = atoi(get_string);
  printf("str to num: %s %d\n", get_string, current_value);
}
*/
