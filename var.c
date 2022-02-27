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
