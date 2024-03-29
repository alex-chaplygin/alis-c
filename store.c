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
#include "memory.h"

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
  set_word_global_word, //20
  set_string_global_word, //22
  set_string_global_array_word, //24
  set_byte_global_array_word,//26
  set_word_global_array_word,//28
  store_byte_object_word, //2a
  store_word_object_word, //2c
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
#ifdef DEBUG
  printf("%x\t", op);
#endif
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
#ifdef DEBUG
  printf("store expression\n");
#endif
  init_stack();
  stack_push(&stack, current_value);
  get_expression();
#ifdef DEBUG
  printf("%04x:\t\t", (int)(current_ip - run_object->class));
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
    run_object->frames_to_skip = (byte)current_value;
  } else if (ww < 0) {
    printf("set byte mem word < 0\n");
    exit(1);
  } else {
    seg_write_byte(run_object->data, w, (byte)current_value);
#ifdef DEBUG
    printf("store_b var_%x, %x; %d\n", w, (byte)current_value, (byte)current_value);
#endif
  }
}

/// запись переменной word по адресу word
void set_word_mem_word()
{
  word w = fetch_word();
  if ((short)w == -40) {// запись номера родительского объекта
    if (current_value == -1 || current_value == -2)
      run_object->parent = current_value;
    else if (current_value % 6 != 0) {
      printf("store word mem word parent object num = %x\n", current_value);
      exit(1);
    } else {
#ifdef DEBUG
      printf("store_w parent, %x; %d object class = %x\n", current_value, current_value, objects_table[current_value / 6].object->id);
#endif
      run_object->parent = objects_table[current_value / 6].object;
    }
  } else if ((short)w < 0) {
    printf("set word mem word < 0 %x\n", w);
    exit(1);
  } else {
    seg_write_word(run_object->data, w, current_value);
#ifdef DEBUG
    printf("store_w var_%x, %x; %d\n", w, current_value, current_value);
#endif
  }
}

/// запись строки по адресу word
void set_string_mem_word()
{
  word w = fetch_word();
  byte *pos = seg_read(run_object->data, w);
  byte *s = store_string;
  while (*pos++ = *s++);
#ifdef DEBUG
  printf("store_str var_%x, \"%s\"\n", w, (char *)seg_read(run_object->data, w));
#endif
}

/// запись переменной byte по адресу byte
void set_byte_mem_byte()
{
  byte w = fetch_byte();
  seg_write_byte(run_object->data, w, (byte)current_value);
#ifdef DEBUG
  printf("store_b var_%x, %x; %d\n", w, (char)current_value, (char)current_value);
#endif
}

/// запись переменной word по адресу byte
void set_word_mem_byte()
{
  byte w = fetch_byte();
  seg_write_word(run_object->data, w, current_value);
#ifdef DEBUG
  printf("store_w var_%x, %x; %d\n", w, current_value, current_value);
#endif
}

/// запись в массив byte по адресу word
void set_byte_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_object->data, w), 0, 1);
  current_value = (short)stack_pop(&stack);
  *pos = (byte)current_value;
#ifdef DEBUG
  printf("store_b arrw_%x[%d], %x; %d\n", w, idx, (byte)current_value, (byte)current_value);
#endif
}

/// запись в массив word по адресу word
void set_word_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  word *pos = (word *)array_pos(seg_read(run_object->data, w), 0, 2);
  current_value = (word)stack_pop(&stack);
  *pos = current_value;
#ifdef DEBUG
  printf("store_w arrw_%x[%d], %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// запись в массив byte по адресу byte
void set_byte_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_object->data, w), 0, 1);
  current_value = (short)stack_pop(&stack);
  *pos = (byte)current_value;
#ifdef DEBUG
  printf("store_b arrb_%x[%d], %x; %d\n", w, idx, (byte)current_value, (byte)current_value);
#endif
}

/// запись в массив word по адресу byte
void set_word_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  word *pos = (word *)array_pos(seg_read(run_object->data, w), 0, 2);
  current_value = (short)stack_pop(&stack);
  *pos = current_value;
#ifdef DEBUG
  printf("store_w arrb_%x[%d], %x; %d\n", w, idx, current_value, current_value);
#endif
}

/// запись в массив строк по адресу word
void set_string_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_object->data, w), 1, 1);
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
  byte *pos = seg_read(run_object->data, w);
  byte *s = store_string;
  while (*pos++ = *s++);
#ifdef DEBUG
  printf("store_str var_%x, \"%s\"\n", w, (char *)seg_read(run_object->data, w));
#endif
}

/// запись в массив строк по адресу byte
void set_string_array_byte()
{
  byte w = fetch_byte();
  int idx = current_value;
  byte *pos = array_pos(seg_read(run_object->data, w), 1, 1);
  byte *p = pos;
  byte *s = store_string;
  while (*p++ = *s++);
#ifdef DEBUG
  printf("store_str array_%x[%d], \"%s\"\n", w, idx, (char *)pos);
#endif
}

/// запись переменной byte по адресу word из другого потока
void store_byte_object_word()
{
  object_t *t;
  word thr = fetch_word();
  if ((short)thr == -40)
    t = run_object->parent;
  else {
    thr = *(word *)seg_read(run_object->data, thr);
    if (thr == 0xffff) {
#ifdef DEBUG
      printf("store byte object = -1\n");
#endif
      return;
    } else if (thr % 6 != 0) {
      printf("store byte object word: invalid object: %x\n", thr);
      exit(1);
    }
    t = objects_table[thr / 6].object;
  }
  word adr = fetch_word();
  if ((short)adr == -2)
    t->frames_to_skip = (byte)current_value;
  else if ((short)adr == -3)
    t->x_flip = (byte)current_value;
  else if ((short)adr < 0) {
    printf("set byte object word adr: %x\n", adr);
    exit(1);
  } else
    seg_write_byte(t->data, adr, (byte)current_value);
  #ifdef DEBUG
  printf("store_byte object: id = %x var_%x: %x; %d\n", t->id, adr, current_value, current_value);
  #endif
}

/// запись переменной word по адресу word из другого потока
void store_word_object_word()
{
  int thr = fetch_word();
  thr = *(word *)seg_read(run_object->data, thr);
  if (thr % 6 != 0) {
    printf("store word object word: invalid object\n");
    exit(1);
  }
  object_t *t = objects_table[thr / 6].object;
  word adr = fetch_word();
  seg_write_word(t->data, adr, current_value);
  #ifdef DEBUG
  printf("store_word object: id = %x var_%x: %x; %d\n", t->id, adr, current_value, current_value);
  #endif
}

/// запись глобальной переменной byte по адресу word
void set_byte_global_word()
{
  word w = fetch_word();
  seg_write_byte(objects_table->object->data, w, (byte)current_value);
#ifdef DEBUG
  printf("store_b main.varw_%x, %x; %d\n", w, (byte)current_value, (byte)current_value);
#endif
}

/// запись глобальной переменной word по адресу word
void set_word_global_word()
{
  word w = fetch_word();
  seg_write_word(objects_table->object->data, w, (word)current_value);
#ifdef DEBUG
  printf("store_w main.varw_%x, %x; %d\n", w, current_value, current_value);
#endif
}

/// запись слова в глобальный массив
void set_word_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  word *b = (word *)array_pos(seg_read(objects_table->object->data, w), 0, 2);
  current_value = stack_pop(&stack);
  *b = current_value;
#ifdef DEBUG
  printf("store_w main.arrw_%x[%d] %x; %d\n", w, idx, current_value, current_value);
#endif
}

/** 
 * Запись байта в глобальный массив
 * current_value - первый индекс в массиве
 * остальные индексы - в стеке
 */
void set_byte_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  byte *b = array_pos(seg_read(objects_table->object->data, w), 0, 1);
  current_value = stack_pop(&stack);
  *b = (byte)current_value;
#ifdef DEBUG
  printf("store_b main.arrw_%x[%d] %x; %d\n", w, idx, (char)current_value, (char)current_value);
#endif
}

/// запись в глобальный массив строк по адресу word
void set_string_global_array_word()
{
  word w = fetch_word();
  int idx = current_value;
  byte *pos = array_pos(seg_read(objects_table->object->data, w), 1, 1);
  byte *p = pos;
  byte *s = store_string;
  while (*p++ = *s++);
#ifdef DEBUG
  printf("store_str main.arrw_%x[%d], \"%s\"\n", w, idx, (char *)pos);
#endif
}

/// запись строки в глобальную переменную по адресу word
void set_string_global_word()
{
  word w = fetch_word();
  byte *pos = seg_read(objects_table->object->data, w);
  byte *s = store_string;
  while (*pos++ = *s++);
#ifdef DEBUG
  printf("store_str main.strw_%x, \"%s\"\n", w, (char *)seg_read(objects_table->object->data, w));
#endif
}
