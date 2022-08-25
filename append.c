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
#include "objects.h"
#include "array.h"

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
  add_string_array_byte, // 18
  add_byte_array_byte, // 1a
  add_word_array_byte, // 1c
  add_byte_global_word, // 1e
  add_word_global_word, // 20
  add_string_global_word, // 22
  nimp, // 24
  add_byte_global_array_word, // 26
  add_word_global_array_word, // 28
  nimp, // 2a
  add_word_object_word, //2c
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
#ifdef DEBUG
  printf("append\n");
#endif
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
  char *p = (char *)seg_read(run_object->data, w);
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
  short *p = (short *)seg_read(run_object->data, w);
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
  char *dst = (char *)seg_read(run_object->data, w);
  char *src = store_string;
  while (*dst++);
  dst--;
  while (*dst++ = *src++);
#ifdef DEBUG
  printf("addw strw_%x; \"%s\"\n", w, (char *)seg_read(run_object->data, w));
#endif
  return 1;
}

/// добавляет значение byte к переменной byte
int add_byte_mem_byte()
{
  byte w = fetch_byte();
  char *p = seg_read(run_object->data, w);
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
  short *p = (short *)seg_read(run_object->data, w);
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
  char *dst = (char *)seg_read(run_object->data, w);
  char *src = store_string;
  while (*dst++);
  dst--;
  while (*dst++ = *src++);
#ifdef DEBUG
  printf("adds strb_%x; \"%s\"\n", w, (char *)seg_read(run_object->data, w));
#endif
  return 1;
}

/// добавляет значение byte к глобальной переменной word
int add_byte_global_word()
{
  word w = fetch_word();
  char *p = (char *)seg_read(objects_table->object->data, w);
  *p += (char)current_value;
#ifdef DEBUG
  printf("addb main.varw_%x, %x; %d\n", w, (char)current_value, (char)current_value);
#endif
  return *p;
}

/// добавляет значение word к глобальной переменной word
int add_word_global_word()
{
  word w = fetch_word();
  short *p = (short *)seg_read(objects_table->object->data, w);
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

/** 
 * Добавление байта в глобальный массив
 * current_value - первый индекс в массиве
 * остальные индексы - в стеке
 */
void add_byte_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  byte *b = array_pos(seg_read(objects_table->object->data, w), 0, 1);
  current_value = stack_pop(&stack);
#ifdef DEBUG
  printf("add_b main.arrw_%x[%d] %x; %d\n", w, idx, (char)current_value, (char)current_value);
  printf("was value = %x\n", *b);
#endif
  *b += (byte)current_value;
#ifdef DEBUG
  printf("after add = %x\n", *b);
#endif
}

/// добавление значения в массив byte по адресу byte
void add_byte_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_object->data, w), 0, 1);
  current_value = (word)stack_pop(&stack);
#ifdef DEBUG
  printf("add_b arrb_%x[%d], %x; %d\n", w, idx, (byte)current_value, (byte)current_value);
  printf("was value = %x\n", *pos);
#endif
  *pos += (byte)current_value;
#ifdef DEBUG
  printf("after add = %x\n", *pos);
#endif
}

/// добавление строки в массив по адресу byte
void add_string_array_byte()
{
  byte w = fetch_byte();
  byte *pos = array_pos(seg_read(run_object->data, w), 1, 1);
  char *res = pos;
  byte *src = store_string;
#ifdef DEBUG
  printf("add_str arrb_%x (%s), %s\n", w, (char *)pos, store_string);
#endif
  while (*pos++);
  pos--;
  while (*pos++ = *src++);
#ifdef DEBUG
  printf("res = %s\n", res);
#endif
}

/// добавление слова в глобальный массив по адресу word
void add_word_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  short *b = (short *)array_pos(seg_read(objects_table->object->data, w), 0, 2);
  current_value = stack_pop(&stack);
#ifdef DEBUG
  printf("add_w main.arrw_%x[%d] %x; %d\n", w, idx, current_value, current_value);
  printf("was value = %x\n", *b);
#endif
  *b += current_value;
#ifdef DEBUG
  printf("after add = %x\n", *b);
#endif
}

/// добавление в массив word по адресу byte
void add_word_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  word *pos = (word *)array_pos(seg_read(run_object->data, w), 0, 2);
  current_value = (short)stack_pop(&stack);
#ifdef DEBUG
  printf("add_w arrb_%x[%d], %x; %d\n", w, idx, current_value, current_value);
  printf("was value = %x\n", *pos);
#endif
  *pos += current_value;
#ifdef DEBUG
  printf("after add = %x\n", *pos);
#endif
}

/// добавляет строку сохранения к глобальной строковой переменной адрес word
int add_string_global_word()
{
  word w = fetch_word();
  char *dst = (char *)seg_read(objects_table->object->data, w);
  char *src = store_string;
  while (*dst++);
  dst--;
  while (*dst++ = *src++);
#ifdef DEBUG
  printf("addw main.strw_%x; \"%s\"\n", w, (char *)seg_read(objects_table->object->data, w));
#endif
  return 1;
}

/// добавление значения в свойство объекта по адресу word
int add_word_object_word()
{
  int thr = fetch_word();
  thr = *(word *)seg_read(run_object->data, thr);
  if (thr % 6 != 0) {
    printf("store word object word: invalid object\n");
    exit(1);
  }
  object_t *t = objects_table[thr / 6].object;
  word adr = fetch_word();
  short *pos = (short *)(t->data->data + adr);
#ifdef DEBUG
  printf("add_word object: class = %x var_%x: %x; %d\n", t->id, adr, current_value, current_value);
  printf("was value: %x\n", *pos);
#endif
  *pos += current_value;
#ifdef DEBUG
  printf("new value: %x\n", *pos);
#endif
  return *pos;
}
