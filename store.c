/**
 * @file   store.c
 * @author alex <alex@localhost>
 * @date   Mon Jan 31 09:22:17 2022
 * 
 * @brief  Интерпретатор - функции записи в переменные
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "store.h"
#include "array.h"
#include "get.h"

/// таблица опреаций присваивания переменных
func set_op[] = {
  no_func, //0
  no_func, // 2
  no_func, // 4
  set_byte_mem_word,//6
  set_word_mem_word,//8
  set_string_mem_word,//a
  set_string_array_word,//c
  set_byte_array_word,//e
  set_word_array_word,//10
  set_byte_mem_byte,//12
  set_word_mem_byte,//14
  set_string_mem_byte,//16
  set_string_array_byte,//18
  set_byte_array_byte,//1a
  set_word_array_byte,//1c
  set_byte_global_word, //1e
  nimp,//set_word_global, //20
  nimp,//set_string_global, //22
  nimp,//set_string_global_array, //24
  nimp,//set_byte_global_array,//26
  nimp,//set_word_global_array,//28
  store_byte_thread_word, //2a
  nimp,//set_word_far,//2c
  nimp,//set_string_far,//2e
  nimp,//set_string_far_array,//30
  nimp,//set_byte_far_array,//32
  nimp,//set_word_far_array,//34
  nimp,//get_push,//36
  store_expression,//38
  null_op, //3a - end of store expression
};

/// операция присваивания переменной
void store()
{
  byte op = fetch_byte();
  if (op & 1) {
    printf("invalid store op %x\n", op);
    exit(1);
  }
  op >>= 1;
  if (op >= sizeof(set_op) / sizeof(set_op[0])) {
    printf("Unknown set op %x\n", op * 2);
    exit(1);
  }
  set_op[op]();
}

/// присваивание выражения
void store_expression()
{
  byte op;
  
#ifdef DEBUG
  printf("store expression\n");
#endif
  init_stack();
  stack_push(&stack, current_value);
  get_expression();
#ifdef DEBUG
  printf("%04x:\t\t", (int)(current_ip - run_thread->script));
#endif
  store();
}

/** 
 * Строки чтения и записи меняются местами.
 * Очищается стек и происходит запись.
 */
void exchange_strings_store()
{
  char *s = get_string;
  get_string = store_string;
  store_string = s;
  init_stack();
  store();
}

/// запись переменной byte по адресу word
void set_byte_mem_word()
{
  word w = fetch_word();
  short ww = (short)w;
  if (ww == -2) {
#ifdef DEBUG
  printf("store_b frames_to_skip, %x; %d\n", (byte)current_value, (byte)current_value);
#endif
    run_thread->frames_to_skip = (byte)current_value;
  } else {
    seg_write_byte(run_thread->data, w, (byte)current_value);
#ifdef DEBUG
  printf("store_b var_%x, %x; %d\n", w, (byte)current_value, (byte)current_value);
#endif
  }
}

/// запись переменной word по адресу word
void set_word_mem_word()
{
  word w = fetch_word();
  seg_write_word(run_thread->data, w, current_value);
#ifdef DEBUG
  printf("store_w var_%x, %x; %d\n", w, current_value, current_value);
#endif
}

/// запись строки по адресу word
void set_string_mem_word()
{
  word w = fetch_word();
  byte *pos = seg_read(run_thread->data, w);
  byte *s = store_string;
  while (*pos++ = *s++);
#ifdef DEBUG
  printf("store_str var_%x, \"%s\"\n", w, (char *)seg_read(run_thread->data, w));
#endif
}

/// запись переменной byte по адресу byte
void set_byte_mem_byte()
{
  byte w = fetch_byte();
  seg_write_byte(run_thread->data, w, (byte)current_value);
#ifdef DEBUG
  printf("store_b var_%x, %x; %d\n", w, (byte)current_value, (byte)current_value);
#endif
}

/// запись переменной word по адресу byte
void set_word_mem_byte()
{
  byte w = fetch_byte();
  seg_write_word(run_thread->data, w, current_value);
#ifdef DEBUG
  printf("store_w var_%x, %x; %d\n", w, current_value, current_value);
#endif
}

/// запись в массив byte по адресу word
void set_byte_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_thread->data, w), 0, 1);
  current_value = (word)stack_pop(&stack);
  *pos = current_value;
#ifdef DEBUG
  printf("store_b array_%x[%d], %x; %d\n", w, idx, (byte)current_value, (byte)current_value);
#endif
}

/// запись в массив word по адресу word
void set_word_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_thread->data, w), 0, 2);
  current_value = (word)stack_pop(&stack);
  *(word *)pos = current_value;
#ifdef DEBUG
  printf("store_w array_%x[%d], %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// запись в массив byte по адресу byte
void set_byte_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_thread->data, w), 0, 1);
  current_value = (word)stack_pop(&stack);
  *(word *)pos = current_value;
  *pos = (byte)current_value;
#ifdef DEBUG
  printf("store_b array_%x[%d], %x; %d\n", w, idx, (byte)current_value, (byte)current_value);
#endif
}

/// запись в массив word по адресу byte
void set_word_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_thread->data, w), 0, 2);
  current_value = (word)stack_pop(&stack);
  *pos = (byte)current_value;
#ifdef DEBUG
  printf("store_w array_%x[%d], %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// запись в массив строк по адресу word
void set_string_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_thread->data, w), 1, 1);
  byte *p = pos;
  byte *s = store_string;
  while (*p++ = *s++);
#ifdef DEBUG
  printf("store_str array_%x[%d], \"%s\"\n", w, idx, (char *)pos);
#endif
}

/// запись строки по адресу byte
void set_string_mem_byte()
{
  byte w = fetch_byte();
  byte *pos = seg_read(run_thread->data, w);
  byte *s = store_string;
  while (*pos++ = *s++);
#ifdef DEBUG
  printf("store_str var_%x, \"%s\"\n", w, (char *)seg_read(run_thread->data, w));
#endif
}

/// запись в массив строк по адресу byte
void set_string_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_thread->data, w), 1, 1);
  byte *p = pos;
  byte *s = store_string;
  while (*p++ = *s++);
#ifdef DEBUG
  printf("store_str array_%x[%d], \"%s\"\n", w, idx, (char *)pos);
#endif
}

/// запись переменной byte по адресу word из другого потока
void store_byte_thread_word()
{
  int thr = fetch_word();
  thr = *(word *)seg_read(run_thread->data, thr);
  thread_t *t = threads_table[thr / 6].thread;
  word adr = fetch_word();
  seg_write_byte(t->data, adr, (byte)current_value);
  #ifdef DEBUG
  printf("store_byte thread: %x var_%x: %x; %d\n", thr, adr, current_value, current_value);
  #endif
}

/// запись переменной byte по адресу word
void set_byte_global_word()
{
  word w = fetch_word();
  seg_write_byte(threads_table->thread->data, w, (byte)current_value);
#ifdef DEBUG
  printf("store_b glob_varw_%x, %x; %d\n", w, (byte)current_value, (byte)current_value);
#endif
}
