/**
 * @file   get.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 14:11:19 2022
 * 
 * @brief  Интерпретатор - обработка выражений
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "get.h"
#include "var.h"
#include "store.h"
#include "math.h"
#include "file.h"
#include "script.h"
#include "threads.h"
#include "key.h"
#include "misc.h"

/// таблица подфункций для выражений
func get_op[] = {
  read_byte,//0
  read_word,//2
  read_string,//4
  get_byte_mem_word,//6
  get_word_mem_word,//8
  get_string_mem_word,//a
  nimp,//get_string_array_word,//c
  nimp,//get_byte_array_word,//e
  get_word_array_word,//10
  get_byte_mem_byte,//12
  get_word_mem_byte,//14
  get_string_mem_byte,//16
  nimp,//get_string_array_byte,//18
  nimp,//get_byte_array_byte,//1a
  nimp,//get_word_array_byte,//1c
  get_byte_global_word,//1e
  get_word_global_word,//20
  nimp,//get_string_global,//22
  get_string_global_array_word,//24
  get_byte_global_array_word,//26
  get_word_global_array_word,//28
  get_byte_from_thread, //2a
  nimp,//get_word_pointer,//2c
  nimp,//get_string_pointer,//2e
  nimp,//get_string_pointer_array,//30
  nimp,//get_byte_pointer_array,//32
  nimp,//get_word_pointer_array,//34
  get_pop,//36
  get_expression,//38
  null_op,//end_of_expr,//3a
  no_func,//3c
  no_func,//3e
  get_push,//40
  and, //42
  or, //44
  xor, //46
  xor_not, //48
  not_eq, //4a
  eq, //4c
  gt, //4e
  ls, //50
  gte, //52
  lse, //54
  add,//56
  sub, //58
  mod, //5a
  op_div, //5c
  mul, //5e
  neg, //60
  op_abs, //62
  op_random, //64
  sign, //66
  not, //68
  get_key, //6a
  null_op, //6c
  get_joy, //6e 
  nimp,//stack_random, //70
  get_message, //72
  get_keyboard_flags, //74
  get_free, //76
  get_video_port, //78
  nimp,//wait_key, //7a
  nimp,//str_trim, //7c
  nimp,//str_part, //7e
  nimp,//str_pos, //80
  nimp,//str_len, //82
  nimp,//str_char, //84
  nimp,//num_to_str, //86
  nimp,//str_cat, //88
  nimp,//str_neq, //8a
  nimp,//str_eq,//8c
  nimp,//str_gt,//8e
  nimp,//str_lt, //90
  nimp,//str_ge, //92
  nimp,//str_le, //94
  nimp,//str_to_stack, //96
  nimp,//prev_from_stack, //98
  nimp,//str_to_num, //9a
  file_not_exists, //9c
  nimp,//set_char, //9e
  nimp,//get_drive, //a0
  nimp,//get_keyboard, //a2
  nimp,//get_cpu_speed, //a4
  nimp,//set_0, //a6
  get_hardware, //a8
};

/** 
 * Переводит стек промежуточных результатов
 * в начальное состояние
 */
void init_stack()
{
  stack.data = stack_data;
  stack.size = MAX_STACK;
  stack.sp = stack.data + stack.size;
}

/** 
 * Вызов подфункции выражения
 * 
 * @param op код операции
 */
void get(byte op)
{
  if (op & 1) {
    printf("invalid get op %x\n", op);
    exit(1);
  }
  op >>= 1;
  if (op >= sizeof(get_op) / sizeof(get_op[0])) {
    printf("Unknown get op %x\n", op * 2);
    exit(1);
  }
  get_op[op]();
}

/// сохранение предыдущего значения и получение данных выражения
void save_get()
{
  prev_value = current_value;
  byte op = fetch_byte();
  get(op);
}

/// очистка стека, сохранение предыдущего значения и получение данных
void new_get()
{
#ifdef DEBUG
  printf("new get\n");
#endif
  init_stack();
  save_get();
}

/// получить данные, поменяв местами текущее и предыдущее
void switch_get()
{
  word d = current_value;
  new_get();
  prev_value = current_value;
  current_value = d;
}

/// меняет местами строки загрузки и сохранения
void switch_string()
{
  char *str = get_string;
  get_string = store_string;
  store_string = str;
  init_stack();
}

/// меняет местами строки и сохраняет значение выражения
void switch_string_store()
{
  switch_string();
#ifdef DEBUG
  printf("%04x:\t\t", (int)(current_ip - run_thread->script));
#endif
  store();
}

/// меняет местами строки и загружает строку
void switch_string_get()
{
  char * s = get_string;
  get_string = text_string;
  text_string = s;
  byte op = fetch_byte();
  get(op);
}

/** 
 * Оператор присваивания: вычисление вырыжения, сохранение
 * результата
 */
void assign()
{
#ifdef DEBUG
  printf("assign\n%04x:\t\t", (int)(current_ip - run_thread->script));
#endif
  new_get();
  switch_string_store();
}

 
// Новое выражение: данные преобразуются и накапливаются в стеке
// пока не конец выражения
void get_expression()
{
  byte op;
#ifdef DEBUG
  printf("%04x:\t\tget expression\n", (int)(current_ip - run_thread->script));
#endif
  while (1) {
    op = fetch_byte();
    if (op == 0x3a) 
      break;
#ifdef DEBUG
    printf("%04x:\t\t", (int)(current_ip - run_thread->script));
#endif
    get(op);
  };  
}

/// читает байт из кода в аккумулятор
void read_byte()
{
  current_value = (short)(char)fetch_byte();
#ifdef DEBUG
  printf("get byte: %x; %d\n", current_value, current_value);
#endif
}

/// читает слово из кода в аккумулятор
void read_word()
{
  current_value = (short)fetch_word();
#ifdef DEBUG
  printf("get word: %x; %d\n", current_value, current_value);
#endif
}

/// читает строку из кода
void read_string()
{
  char *c = current_ip;
  char *dst = get_string;
  do
    *dst++ = *c++;
  while (*(c - 1));
  if (dst - get_string >= MAX_STR) {
    printf("MAX get string\n");
    exit(1);
  }
#ifdef DEBUG
  printf("get string: %s\n", get_string);
#endif
  current_ip = c;
}

/// выталкивает значение из стека
/// стек -> current_value, prev_value, stack
void  get_pop()
{
  current_value = prev_value;
  prev_value = stack_pop(&stack);
  printf("pop %x %x\n", current_value, prev_value);
}

/// ошибочная функция для get и store
void no_func()
{
  printf("no func\n");
  exit(1);
}

/// текущее значение сохраняется в стек
void get_push()
{
#ifdef DEBUG
  printf("push %x\n", current_value);
#endif
  stack_push(&stack, current_value);
}
