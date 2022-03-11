#include <stdio.h>
#include <string.h>
#include "interpret.h"

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
/*
void str_char()
{
  current_value = (byte)*current_string;
  printf("str char: %c %d\n", (char)current_value, current_value);
}

void num_to_str()
{
  sprintf(current_string, "%d", current_value);
  printf("num_to_str %d %s\n", current_value, current_string);
}

void switch_str_get()
{
  char *str = current_string;
  current_string = prev_string;
  prev_string = str;
  get();
}

void str_cat()
{
  char *src;
  char *str;
  switch_str_get();
  printf("str_cat %s %s\n", current_string, prev_string);
  str = prev_string + strlen(prev_string);
  src = current_string;
  strcpy(str, src);
  str = current_string;
  current_string = prev_string;
  prev_string = str;
  printf("res: %s\n", current_string);
}

void str_neq()
{
  switch_str_get();
  printf("str_neq %s %s\n", current_string, prev_string);
  if (strcmp(current_string, prev_string)) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_eq()
{
  switch_str_get();
  printf("str_eq %s %s\n", current_string, prev_string);
  if (!strcmp(current_string, prev_string)) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_gt()
{
  switch_str_get();
  printf("str_gt %s %s\n", current_string, prev_string);
  if (strcmp(current_string, prev_string) > 0) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_lt()
{
  switch_str_get();
  printf("str_lt %s %s\n", current_string, prev_string);
  if (strcmp(current_string, prev_string) < 0) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_ge()
{
  switch_str_get();
  printf("str_ge %s %s\n", current_string, prev_string);
  if (strcmp(current_string, prev_string) >= 0) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_le()
{
  switch_str_get();
  printf("str_le %s %s\n", current_string, prev_string);
  if (strcmp(current_string, prev_string) <= 0) current_value = 0;
  else current_value = -1;
  printf("res: %d\n", current_value);
}

void str_to_stack()
{
  char *c = current_string + strlen(current_string);
  printf("str to stack: %s\n", current_string);
  do {
    stack_pos--;
    if (stack_pos == 0) {
      printf("Stack is full\n");
      exit(1);
    }
    stack[stack_pos] = *c--;
  } while (c >= current_string);
}

void prev_from_stack()
{
  char *str = current_string;
  current_string = prev_string;
  prev_string = str;
  do {
    if (stack_pos == MAX_STACK) {
      printf("Max stack\n");
      exit(1);
    }
    *str++ = stack[stack_pos++];
  } while (*(str - 1) != 0);
  printf("prev from stack %s %s\n", current_string, prev_string);
}

void str_to_num()
{
  current_value = atoi(current_string);
  printf("str to num: %s %d\n", current_string, current_value);
}

void set_char()
{
  *current_string = (char)(current_value & 0xff);
  *(current_string + 1) = 0;
  printf("set  char %s %x\n", current_string, (current_value & 0xff));
}
*/
