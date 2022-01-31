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
};

/** 
 * Новое выражение для добавления результатов
 * Сохраняет предыдущее значение, получает новое,
 * меняет местами строки загрузки и сохранения
 * вызывает подфункцию добавления.
 */
void append()
{
  init_stack();
  save_get();
  switch_string();
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
  add_op[op]();
}

/// добавляет значение byte к переменной word
void add_byte_mem_word()
{
  word w = fetch_word();
  byte *p = seg_read(run_thread->data, w);
  *p += (byte)current_value;
#ifdef DEBUG
  printf("addb varw_%x, %x; %d\n", w, (byte)current_value, (byte)current_value);
#endif
}

/// добавляет значение word к переменной word
void add_word_mem_word()
{
  word w = fetch_word();
  word *p = (word *)seg_read(run_thread->data, w);
  *p += current_value;
#ifdef DEBUG
  printf("addw varw_%x, %x; %d\n", w, current_value, current_value);
#endif
}

/// добавляет строку сохранения к строковой переменной адрес word
void add_string_mem_word()
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
}

/// добавляет значение byte к переменной byte
void add_byte_mem_byte()
{
  byte w = fetch_byte();
  byte *p = seg_read(run_thread->data, w);
  *p += (byte)current_value;
#ifdef DEBUG
  printf("addb varb_%x, %x; %d\n", w, (byte)current_value, (byte)current_value);
#endif
}

/// добавляет значение word к переменной byte
void add_word_mem_byte()
{
  byte w = fetch_byte();
  word *p = (word *)seg_read(run_thread->data, w);
  *p += current_value;
#ifdef DEBUG
  printf("addw varb_%x, %x; %d\n", w, current_value, current_value);
#endif
}

/// добавляет строку сохранения к строковой переменной адрес byte
void add_string_mem_byte()
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
}
