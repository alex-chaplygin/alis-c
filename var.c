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
#include "objects.h"
#include "array.h"

/// чтение переменной типа byte по адресу word
void get_byte_mem_word()
{
  word w = fetch_word();
  if ((short)w == -3)
    current_value = run_object->x_flip;
  else if ((short)w < 0) {
    printf("get byte mem word < 0 %d\n", (short)w);
    exit(1);
  } else
    current_value = (char)*seg_read(run_object->data, w);
#ifdef DEBUG
  printf("get byte varw_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение переменной типа word по адресу word
void get_word_mem_word()
{
  word w = fetch_word();
  if ((short)w == -40) { // чтение номера родительского потока
    if (run_object->parent == -1 || run_object->parent == -2)
      current_value = run_object->parent;
    else
      current_value = object_num(run_object->parent);
  } else if ((short)w == -14)
    current_value = object_num(run_object);
  else if ((short)w < 0) {
    printf("get word mem word < 0 = %d\n", (short)w);
    exit(1);
  } else
    current_value = *(short *)seg_read(run_object->data, w);
#ifdef DEBUG
  printf("get word varw_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение строки по адресу word
void get_string_mem_word()
{
  word w = fetch_word();
  byte *ip = current_ip;
  current_ip = seg_read(run_object->data, w);
  read_string();
#ifdef DEBUG
  printf("get string strw_%x: %s\n", w, get_string);
#endif
  current_ip = ip;
#ifdef DEBUG
  printf("new ip = %x\n", (int)(current_ip - run_object->class));
#endif
}

/// чтение глобальной строки по адресу word
void get_string_global_word()
{
  word w = fetch_word();
  byte *ip = current_ip;
  current_ip = seg_read(objects_table->object->data, w);
  read_string();
#ifdef DEBUG
  printf("get string main.strw_%x: %s\n", w, get_string);
#endif
  current_ip = ip;
}

/// чтение переменной типа byte по адресу byte
void get_byte_mem_byte()
{
  byte w = fetch_byte();
  current_value = (char)*seg_read(run_object->data, w);
#ifdef DEBUG
  printf("get byte varb_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение переменной типа word по адресу byte
void get_word_mem_byte()
{
  byte w = fetch_byte();
  current_value = *(short *)seg_read(run_object->data, w);
#ifdef DEBUG
  printf("get word varb_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение строки по адресу byte
void get_string_mem_byte()
{
  byte w = fetch_byte();
  byte *ip = current_ip;
  current_ip = seg_read(run_object->data, w);
  read_string();
#ifdef DEBUG
  printf("get string strb_%x: %s\n", w, get_string);
#endif
  current_ip = ip;
}

/** 
 * чтение переменной byte по адресу word из другого потока
 * номер потока берется из переменной word
 */
void get_byte_from_object()
{
  object_t *t;
  word thr = fetch_word();
  if ((short)thr == -40)
    t = run_object->parent;
  else if ((short)thr < 0) {
    printf("get byte from object: %x\n", thr);
    exit(1);
  } else {
    thr = *(word *)seg_read(run_object->data, thr);
    if (thr % 6 != 0) {
      printf("get byte object word: invalid object: %x\n", thr);
      exit(1);
    }
    t = objects_table[thr / 6].object;
  }
  word adr = fetch_word();
  if ((short)adr == -3)
    current_value = t->x_flip;
  else if ((short)adr < 0) {
    printf("get byte from object adr: %x\n", adr);
    exit(1);
  } else
    current_value =*(char *)seg_read(t->data, adr);
#ifdef DEBUG
  printf("get byte object: %x var_%x: ", *t->class, adr);
#endif
#ifdef DEBUG
  printf("%x; %d\n", current_value, current_value);
#endif
}

/** 
 * чтение переменной word по адресу word из другого потока
 * номер потока берется из переменной word
 */
void get_word_from_object()
{
  object_t *t;
  word thr = fetch_word();
  if ((short)thr < 0) {
    printf("get word from object: %x\n", thr);
    exit(1);
  } else {
    thr = *(word *)seg_read(run_object->data, thr);
    if (thr % 6 != 0) {
      printf("get word object word: invalid object\n");
      exit(1);
    }
    t = objects_table[thr / 6].object;
  }
  word adr = fetch_word();
  if ((short)adr == -40) {
#ifdef DEBUG
    printf("get word object: %x parent obj class = %x\n", thr, t->id);
#endif
    current_value = object_num(t->parent);
  } else if ((short)adr < 0) {
    printf("get word from object adr: %x\n", adr);
    exit(1);
  } else {
    current_value = *(short *)seg_read(t->data, adr);
#ifdef DEBUG
    printf("get word object: %x var_%x: %x; %d\n", thr, adr, current_value, current_value);
#endif
  }
}

/// чтение глобальной переменной типа byte по адресу word
void get_byte_global_word()
{
  word w = fetch_word();
  current_value = (char)*seg_read(objects_table->object->data, w);
#ifdef DEBUG
  printf("get byte main.varw_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение глобальной переменной типа word по адресу word
void get_word_global_word()
{
  word w = fetch_word();
  current_value = *(short *)seg_read(objects_table->object->data, w);
#ifdef DEBUG
  printf("get word main.varw_%x: %x; %d\n", w, current_value, current_value);
#endif
}

/// чтение слова из массива по адресу word
void get_word_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  current_value = *(short *)array_pos(seg_read(run_object->data, w), 0, 2);
#ifdef DEBUG
  printf("get word arrw_%x[%d] %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// чтение байта из массива по адресу word
void get_byte_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  current_value = *(char *)array_pos(seg_read(run_object->data, w), 0, 1);
#ifdef DEBUG
  printf("get byte arrw_%x[%d] %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// чтение слова из массива по адресу byte
void get_word_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  current_value = *(short *)array_pos(seg_read(run_object->data, w), 0, 2);
#ifdef DEBUG
  printf("get word arrb_%x[%d] %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// чтение байта из массива по адресу byte
void get_byte_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  current_value = *(char *)array_pos(seg_read(run_object->data, w), 0, 1);
#ifdef DEBUG
  printf("get byte arrb_%x[%d] %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// чтение слова из массива по адресу word
void get_word_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  current_value = *(short *)array_pos(seg_read(objects_table->object->data, w), 0, 2);
#ifdef DEBUG
  printf("get word main.arrw_%x[%d] %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// чтение слова из массива по адресу word
void get_byte_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  current_value = *(char *)array_pos(seg_read(objects_table->object->data, w), 0, 1);
#ifdef DEBUG
  printf("get byte main.arrw_%x[%d] %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// чтение строки из глобального массива word
void get_string_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  char *src = (char *)array_pos(seg_read(objects_table->object->data, w), 1, 1);
  char *dst = get_string;
  while (*dst++ = *src++) ;
  if (dst - get_string >= MAX_STR) {
    printf("MAX get string\n");
    exit(1);
  }
#ifdef DEBUG
  printf("get string main.arrsw_%x[%d] \"%s\"\n", w, idx, get_string);
#endif
}

/// чтение строки из массива по адресу байт
void get_string_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  char *src = (char *)array_pos(seg_read(run_object->data, w), 1, 1);
  char *dst = get_string;
  while (*dst++ = *src++) ;
  if (dst - get_string >= MAX_STR) {
    printf("MAX get string\n");
    exit(1);
  }
#ifdef DEBUG
  printf("get string arrsb_%x[%d] \"%s\"\n", w, idx, get_string);
#endif
}
