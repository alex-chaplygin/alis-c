/**
 * @file   var.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 14:16:49 2022
 * 
 * @brief  Интерпретатор - работа с переменными
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "memory.h"
#include "get.h"
#include "threads.h"
#include "array.h"

/// чтение переменной типа byte по адресу word
void get_byte_mem_word()
{
  word w = fetch_word();
  current_value = (char)*seg_read(run_thread->data, w);
#ifdef DEBUG
  printf("get byte varw_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение переменной типа word по адресу word
void get_word_mem_word()
{
  word w = fetch_word();
  if ((short)w == -40) // чтение номера родительского потока
    current_value = thread_num(run_thread->parent);
  else
    current_value = (short)*(word *)seg_read(run_thread->data, w);
#ifdef DEBUG
  printf("get word varw_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение строки по адресу word
void get_string_mem_word()
{
  word w = fetch_word();
  byte *ip = current_ip;
  current_ip = seg_read(run_thread->data, w);
  read_string();
#ifdef DEBUG
  printf("get string strw_%x: %s\n", w, get_string);
#endif
  current_ip = ip;
}

/// чтение переменной типа byte по адресу byte
void get_byte_mem_byte()
{
  byte w = fetch_byte();
  current_value = (char)*seg_read(run_thread->data, w);
#ifdef DEBUG
  printf("get byte varb_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение переменной типа word по адресу byte
void get_word_mem_byte()
{
  byte w = fetch_byte();
  current_value = (short)*(word *)seg_read(run_thread->data, w);
#ifdef DEBUG
  printf("get word varb_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение строки по адресу byte
void get_string_mem_byte()
{
  byte w = fetch_byte();
  byte *ip = current_ip;
  current_ip = seg_read(run_thread->data, w);
  read_string();
#ifdef DEBUG
  printf("get string strb_%x: %s\n", w, get_string);
#endif
  current_ip = ip;
}

/// чтение переменной byte по адресу word из другого потока
void get_byte_from_thread()
{
  int thr = fetch_word();
  thread_t *t = threads_table[thr / 6].thread;
  word adr = fetch_word();
  current_value =(char)*seg_read(t->data, adr);
  #ifdef DEBUG
  printf("get byte thread: %x var_%x: %x; %d\n", thr, adr, current_value, current_value);
  #endif
  exit(1);
}

/// чтение глобальной переменной типа byte по адресу word
void get_byte_global_word()
{
  word w = fetch_word();
  current_value = (char)*seg_read(threads_table->thread->data, w);
#ifdef DEBUG
  printf("get byte main.varw_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение глобальной переменной типа word по адресу word
void get_word_global_word()
{
  word w = fetch_word();
  current_value = (char)*seg_read(threads_table->thread->data, w);
#ifdef DEBUG
  printf("get word main.varw_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение из массива слов
void get_word_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  current_value = *(short *)array_pos(seg_read(run_thread->data, w), 0, 2);
#ifdef DEBUG
  printf("get word arrw_%x[%d] %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// чтение слова из массива по адресу word
void get_word_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  current_value = *(short *)array_pos(seg_read(threads_table->thread->data, w), 0, 2);
#ifdef DEBUG
  printf("get word main.arrw_%x[%d] %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// чтение слова из массива по адресу word
void get_byte_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  current_value = *(char *)array_pos(seg_read(threads_table->thread->data, w), 0, 1);
#ifdef DEBUG
  printf("get byte main.arrw_%x[%d] %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// чтение строки из глобального массива word
void get_string_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  char *src = (char *)array_pos(seg_read(threads_table->thread->data, w), 1, 1);
  char *dst = get_string;
  while (*dst++ = *src++) ;
#ifdef DEBUG
  printf("get string main.arrsw_%x[%d] \"%s\"\n", w, idx, get_string);
#endif
}
