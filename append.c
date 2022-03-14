/**
 * @file   append.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 12:22:33 2022
 * 
 * @brief  Подфункций для добавления данных
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "get.h"
#include "append.h"
#include "threads.h"

/// таблица подфункций для добавления
func add_op[] = {
  no_func,
  no_func,
  no_func,
  add_byte_mem_word, // 6
  add_word_mem_word, // 8
  add_string_mem_word, // a
  nimp, // c
  nimp, // e
  nimp, // 10
  add_byte_mem_byte, // 12
  add_word_mem_byte, // 14 
  add_string_mem_byte, // 16
  nimp, // 18
  nimp, // 1a
  nimp, // 1c
  add_byte_global_word, // 1e
  add_word_global_word, // 20
  nimp, // 22
  nimp, // 24
  nimp, // 26
  nimp, // 28
  nimp, // 2a
  nimp, // 2c
  nimp, // 2e
  nimp, // 30
  nimp, // 32
  nimp, // 34
  nimp, // 36
  add_expression, // 38
};

/// вызов одной подфункции добавления
int op_append()
{
  byte op = fetch_byte();
  if (op & 1) {
    printf("invalid append op %x\n", op);
    exit(1);
  }
  op >>= 1;
  if (op >= sizeof(add_op) / sizeof(add_op[0])) {
    printf("Unknown add op %x\n", op * 2);
    exit(1);
  }
  return add_op[op]();
}

/** 
 * меняет местами строки загрузки и сохранения
 * вызывает подфункцию добавления.
 */
int exchange_strings_append()
{
  switch_string();
  return op_append();
}

/** 
 * Новое выражение для добавления результатов
 * Сохраняет предыдущее значение, получает новое,
 * меняет местами строки загрузки и сохранения
 * вызывает подфункцию добавления.
 */
int append()
{
  init_stack();
  save_get();
  return exchange_strings_append();
}

/// вычитает значение выражения из переменной
int subract()
{
  new_get();
  current_value = -current_value;
#ifdef DEBUG
  printf("subtract: %d\n", current_value);
#endif
  return exchange_strings_append();
}

/// добавляет значение byte к переменной word
int add_byte_mem_word()
{
  word w = fetch_word();
  byte *p = seg_read(run_thread->data, w);
  *p += (char)current_value;
#ifdef DEBUG
  printf("addb varw_%x, %x; %d\n", w, (char)current_value, (char)current_value);
#endif
  return *p;
}

/// добавляет значение word к переменной word
int add_word_mem_word()
{
  word w = fetch_word();
  word *p = (word *)seg_read(run_thread->data, w);
  *p += current_value;
#ifdef DEBUG
  printf("addw varw_%x, %x; %d\n", w, current_value, current_value);
#endif
  return *p;
}

/// добавляет строку сохранения к строковой переменной адрес word
int add_string_mem_word()
{
  word w = fetch_word();
  char *dst = (char *)seg_read(run_thread->data, w);
  char *src = store_string;
  while (*dst++);
  dst--;
  while (*dst++ = *src++);
#ifdef DEBUG
  printf("addw strw_%x; \"%s\"\n", w, (char *)seg_read(run_thread->data, w));
#endif
  return 1;
}

/// добавляет значение byte к переменной byte
int add_byte_mem_byte()
{
  byte w = fetch_byte();
  byte *p = seg_read(run_thread->data, w);
  *p += (char)current_value;
#ifdef DEBUG
  printf("addb varb_%x, %x; %d\n", w, (char)current_value, (char)current_value);
#endif
  return *p;
}

/// добавляет значение word к переменной byte
int add_word_mem_byte()
{
  byte w = fetch_byte();
  word *p = (word *)seg_read(run_thread->data, w);
  *p += current_value;
#ifdef DEBUG
  printf("addw varb_%x, %x; %d\n", w, current_value, current_value);
#endif
  return *p;
}

/// добавляет строку сохранения к строковой переменной адрес byte
int add_string_mem_byte()
{
  byte w = fetch_byte();
  char *dst = (char *)seg_read(run_thread->data, w);
  char *src = store_string;
  while (*dst++);
  dst--;
  while (*dst++ = *src++);
#ifdef DEBUG
  printf("adds strb_%x; \"%s\"\n", w, (char *)seg_read(run_thread->data, w));
#endif
  return 1;
}

/// добавляет значение byte к глобальной переменной word
int add_byte_global_word()
{
  word w = fetch_word();
  char *p = (char *)seg_read(threads_table->thread->data, w);
  *p += (char)current_value;
#ifdef DEBUG
  printf("addb main.varw_%x, %x; %d\n", w, (char)current_value, (char)current_value);
#endif
  exit(1);
  return *p;
}

/// добавляет значение word к глобальной переменной word
int add_word_global_word()
{
  word w = fetch_word();
  short *p = (short *)seg_read(threads_table->thread->data, w);
  *p += current_value;
#ifdef DEBUG
  printf("addw main.varw_%x, %x; %d\n", w, current_value, current_value);
#endif
  return *p;
}

/// выражение для добавления
int add_expression()
{
  init_stack();
  stack_push(&stack, current_value);
#ifdef DEBUG
  printf("add expression: cur = %x; %d\n", current_value, current_value);
#endif
  get_expression();
  return op_append();
}
