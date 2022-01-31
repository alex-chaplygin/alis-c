/**
 * @file   call.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 12:33:58 2022
 * 
 * @brief  Функции интерпретатора - вызовы и переходы: условные и безусловные
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "memory.h"

/// вызов
void call(int pos)
{
  stack_push(run_thread->call_stack, (int)current_ip);
  current_ip += pos;
}

/// ближний вызов
void call_byte()
{
  char pos = (char)fetch_byte();
#ifdef DEBUG
  printf("call %x\n", pos);
#endif
  call(pos);
}

/// вызов с адресом word
void call_word()
{
  short pos = (short)fetch_word();
#ifdef DEBUG
  printf("callw %d\n", pos);
#endif
  call(pos);
}

/// вызов с адресом word + 1
void call_skip_word()
{
  short pos = (short)fetch_word();
  current_ip++;
#ifdef DEBUG
  printf("callsw %d\n", pos);
#endif
  call(pos);
}

/// ближний переход
void jump_byte()
{
  char p = (char)fetch_byte();
#ifdef DEBUG
  printf("jmp %d\n", p);
#endif
  current_ip += p;
}

/// дальний переход
void jump_word()
{
  short p = (short)fetch_word();
#ifdef DEBUG
  printf("jmpw %d\n", p);
#endif
  current_ip += p;
}

/// дальний переход + 1
void jump_skip_word()
{
  short p = (short)fetch_word();
#ifdef DEBUG
  printf("jmpsw %d\n", p);
#endif
  current_ip += p + 1;
}

/** 
 * Для дополнительных процедур сценария предусмотрен
 * возврат к сохраненному состоянию стека вызовов
 */
void saved_return()
{
#ifdef DEBUG
  printf("saved return\n");
#endif
  if (!no_saved_return) {
    if (run_thread->call_stack->sp >= run_thread->saved_sp) {
      interpreting = 0;
      return;
    }
  }
  current_ip = (byte *)stack_pop(run_thread->call_stack);
}

/// ближний переход если равно
void jump_byte_z()
{
#ifdef DEBUG
  printf("jz_b\n");
#endif
  if (current_value)
    current_ip++;
  else {
    char b = (char)fetch_byte();
    current_ip += b;
  }
}

/// дальний переход если равно
void jump_word_z()
{
#ifdef DEBUG
  printf("jz_w\n");
#endif
  if (current_value)
    current_ip += 2;
  else {
    short b = (short)fetch_word();
    current_ip += b;
  }
}

/// дальний переход + 1 если равно
void jump_word_skip_z()
{
#ifdef DEBUG
  printf("jz_sw\n");
#endif
  if (current_value)
    current_ip += 3;
  else {
    short b = (short)fetch_word();    
    current_ip += b + 1;
  }
}

/// ближний переход если не равно
void jump_byte_nz()
{
#ifdef DEBUG
  printf("jnz_b\n");
#endif
  if (!current_value)
    current_ip++;
  else {
    char b = (char)fetch_byte();
    current_ip += b;
  }
}

/// дальний переход если не равно
void jump_word_nz()
{
#ifdef DEBUG
  printf("jnz_w\n");
#endif
  if (!current_value)
    current_ip += 2;
  else {
    short b = (short)fetch_word();
    current_ip += b;
  }
}

/// дальний переход + 1 если не равно
void jump_word_skip_nz()
{
#ifdef DEBUG
  printf("jnz_sw\n");
#endif
  if (!current_value)
    current_ip += 3;
  else {
    short b = (short)fetch_word();
    current_ip += b + 1;
  }
}

/// сравнение и ближний переход если равно
void compare_jump_byte_z()
{
#ifdef DEBUG
  printf("cmp_jz_b\n");
#endif
  if (current_value != prev_value)
    current_ip++;
  else {
    char b = (char)fetch_byte();
    current_ip += b;
  }
}

/// сравнение и дальний переход если равно
void compare_jump_word_z()
{
#ifdef DEBUG
  printf("cmp_jz_w\n");
#endif
  if (current_value != prev_value)
    current_ip += 2;
  else {
    short b = (short)fetch_word();
    current_ip += b;
  }
}

/// сравнение и дальний переход + 1 если равно
void compare_jump_word_skip_z()
{
#ifdef DEBUG
  printf("cmp_jz_sw\n");
#endif
  if (current_value != prev_value)
    current_ip += 3;
  else {
    short b = (short)fetch_word();
    current_ip++;
    current_ip += b;
  }
}

/// сравнение и ближний переход если не равно
void compare_jump_byte_nz()
{
#ifdef DEBUG
  printf("cmp_jnz_b\n");
#endif
  if (current_value == prev_value)
    current_ip++;
  else {
    char b = (char)fetch_byte();
    current_ip += b;
  }
}

/// сравнение и ближний переход если не равно
void compare_jump_word_nz()
{
#ifdef DEBUG
  printf("cmp_jnz_w\n");
#endif
  if (current_value == prev_value)
    current_ip += 2;
  else {
    short b = (short)fetch_word();
    current_ip += b;
  }
}

/// сравнение и дальний переход + 1 если не равно
void compare_jump_word_skip_nz()
{
#ifdef DEBUG
  printf("cmp_jnz_sw\n");
#endif
  if (current_value == prev_value)
    current_ip += 3;
  else {
    short b = (short)fetch_word();
    current_ip++;
    current_ip += b;
  }
}