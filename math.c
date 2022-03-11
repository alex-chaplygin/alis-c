/**
 * @file   math.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 17:02:40 2022
 * 
 * @brief  Интерпретатор - математические функции
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "get.h"
#include "memory.h"

word random_seed;		/**< псевдослучайное зерно */

/// побитовое и
void and()
{
  save_get();
#ifdef DEBUG
  printf("\t\tand %x %x\n", current_value, prev_value);
#endif
  current_value &= prev_value;
}

/// побитовое или
void or()
{
  save_get();
#ifdef DEBUG
  printf("\t\tor %x %x\n", current_value, prev_value);
#endif
  current_value |= prev_value;
}

/// побитовое исключающее или
void xor()
{
  save_get();
#ifdef DEBUG
  printf("\t\txor %x %x\n", current_value, prev_value);
#endif
  current_value ^= prev_value;
}

/// исключающее или - не
void xor_not()
{
  save_get();
#ifdef DEBUG
  printf("\t\txor_not %d %d\n", current_value, prev_value);
#endif
  current_value ^= prev_value;
  current_value = ~current_value;
}

/// не равно
void not_eq()
{
  save_get();
#ifdef DEBUG
  printf("\t\tnot_eq %d %d\n", current_value, prev_value);
#endif
  current_value = (current_value != prev_value) ? 0: -1;
}

/// равно
void eq()
{
  save_get();
#ifdef DEBUG
  printf("\t\teq %d %d\n", current_value, prev_value);
#endif
  current_value = (current_value == prev_value) ? 0: -1;
}

/// больше
void gt()
{
  save_get();
#ifdef DEBUG
  printf("\t\tgt %d %d\n", prev_value, current_value);
#endif
  current_value = (prev_value > current_value) ? 0: -1;
}

/// меньше
void ls()
{
  save_get();
#ifdef DEBUG
  printf("\t\tls %d %d\n", prev_value, current_value);
#endif
  current_value = (prev_value < current_value) ? 0: -1;
}

/// больше или равно
void gte()
{
  save_get();
#ifdef DEBUG
  printf("\t\tgte %d %d\n", prev_value, current_value);
#endif
  current_value = (prev_value >= current_value) ? 0: -1;
}

/// меньше или равно
void lse()
{
  save_get();
#ifdef DEBUG
  printf("\t\tlse %d %d\n", prev_value, current_value);
#endif
  current_value = (prev_value <= current_value) ? 0: -1;
}

/// сложение
void add()
{
  save_get();
#ifdef DEBUG
  printf("\t\tadd %d %d\n", current_value, prev_value);
#endif
  current_value += prev_value;
}

/// вычитание
void sub()
{
  save_get();
#ifdef DEBUG
  printf("\t\tsub %d %d\n", prev_value, current_value);
#endif
  prev_value -= current_value;
  current_value = prev_value;
}

/// остаток от деления
void mod()
{
  save_get();
#ifdef DEBUG
  printf("\t\tmod %d %d\n", prev_value, current_value);
#endif
  word w = prev_value;
  prev_value = current_value;
  if (!prev_value)
    prev_value = 1;
  current_value = w % prev_value;
}

/// целочисленное деление
void op_div()
{
  save_get();
#ifdef DEBUG
  printf("\t\tdiv %d %d\n", prev_value, current_value);
#endif
  word w = prev_value;
  prev_value = current_value;
  if (!prev_value)
    prev_value = 1;
  current_value = w / prev_value;
}

/// умножение
void mul()
{
  save_get();
#ifdef DEBUG
  printf("\t\tmul %d %d\n", prev_value, current_value);
#endif
  current_value *= prev_value;
}

/// унарный минус
void neg()
{
#ifdef DEBUG
  printf("\t\tneg %d\n", current_value);
#endif
  current_value = -current_value;
}

/// модуль
void op_abs()
{
#ifdef DEBUG
  printf("\t\tabs %d\n", current_value);
#endif
  if (current_value < 0)
    current_value = -current_value;
}

/// случайное число
void op_random()
{
  random_seed = (random_seed * 31415 & 0xffff) + 63617;
  current_value = random_seed * current_value >> 16;
#ifdef DEBUG
  printf("seed = %x random: %d\n", random_seed, current_value);
#endif
}

/// знак
void sign()
{
#ifdef DEBUG
  printf("gt0 %d\n", current_value);
#endif
  if (current_value)
    current_value = (current_value > 0) ? 1 : -1;
}

/// побитовое не
void not()
{
#ifdef DEBUG
  printf("not %d\n", current_value);
#endif
  current_value = ~current_value;
}

/// установка зерна для случайных чисел
extern long current_time;
void set_random_seed()
{
  byte op = fetch_byte();
  get(op);
  if (!current_value)
    random_seed = 0x352e;//(word)current_time;
  else
    random_seed = current_value;
#ifdef DEBUG
  printf("set random seed %d %d\n", current_value, random_seed);
#endif
}

/// циклический сдвиг вправо
word ror(word num, int count)
{
  int bit;
  for (int i = 0; i < count; i++) {
    bit = num & 1;
    num = (num >> 1) + (bit << 15);
  }
  return num;
}

/** 
 * Псевдослучайное число
 * зерно берется из стека
 */
void random_with_seed()
{
  short w = stack_pop(&stack);
  w = ror(w, 6);
#ifdef DEBUG
  printf("random with seed: max = %d new seed = %x\n", current_value, w);
#endif
  word s = random_seed;
  random_seed = w;
  op_random();
#ifdef DEBUG
  printf("res: %d\n", current_value);
#endif
  random_seed = s;
}

