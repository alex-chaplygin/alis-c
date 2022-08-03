/**
 * @file   call.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 12:33:58 2022
 * 
 * @brief  Функции интерпретатора - вызовы и переходы: условные и безусловные
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "memory.h"
#include "append.h"
#include "get.h"

/// вызов
void call(int pos)
{
  stack_push(run_thread->call_stack, (int)current_ip);
  current_ip += pos;
#ifdef DEBUG
  printf("new ip = %x\n", (int)(current_ip - run_thread->script));
#endif
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
 * Для обработчиков - возврат только один раз
 * Для главной программы - обычный возврат, с обнулением сохранения
 */
void saved_return()
{
#ifdef DEBUG
  printf("saved return stack pos = %d\n", (int)(run_thread->call_stack->sp - run_thread->saved_sp));
#endif
  if (!main_run) {
    if (run_thread->call_stack->sp >= saved_sp) {
      yield();
#ifdef DEBUG
      printf("main_run = 0 sp >= saved_sp\n");
#endif
      exit(1);
      return;
    }
#ifdef DEBUG
    printf("main_run = 0 sp < saved_sp\n");
#endif
  }
  current_ip = (byte *)stack_pop(run_thread->call_stack);
  if (run_thread->call_stack->sp - 1 == run_thread->saved_sp) {
    run_thread->saved_sp = 0;
#ifdef DEBUG
    printf("sp == saved_sp\n");
    printf("ip = %x\n", (int)(current_ip - run_thread->script));
#endif
  }
#ifdef DEBUG
  printf("new ip = %x\n", (int)(current_ip - run_thread->script));
#endif
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
#ifdef DEBUG
  printf("new ip = %x\n", (int)(current_ip - run_thread->script));
#endif
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
#ifdef DEBUG
  printf("new ip = %x\n", (int)(current_ip - run_thread->script));
#endif
  exit(1);
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

/** 
 * Уменьшает значение переменной на единицу,
 * если результат больше 0, то делает переход
 */
void loop_byte()
{
  byte *ip = current_ip++;
  current_value = -1;
  if (exchange_strings_append()) {
    current_ip = ip;
    char ofs = (char)fetch_byte();
    current_ip = ip + 1 + ofs;
#ifdef DEBUG
    printf("loop byte: ofs = %d ip = %x\n", ofs, (int)(current_ip - run_thread->script));
#endif
  } else {
#ifdef DEBUG
    printf("loop end ip = %x\n", (int)(current_ip - run_thread->script));
#endif
  }
}

/** 
 * команда таблица переходов
 * значения выражения - индекс в таблице
 */
void op_jump_table()
{
  new_get();
  int c = fetch_byte();
  if ((int)(current_ip - run_thread->script) & 1)
    current_ip++;
  current_value += *(short *)current_ip;
  if (current_value < 0 || current_value > c)
    current_ip += 4 + c * 2;
  else {
    current_ip += 2 + current_value * 2;
    current_ip += 2 + *(short *)current_ip;
  }
  #ifdef DEBUG
  printf("jump table count = %d cur_val = %x ip = %x\n", c, current_value, (int)(current_ip - run_thread->script));
  #endif
}

/** 
 * Команда - выбор перехода в зависимости
 * от значения выражения
 */
void op_switch_case()
{
  new_get();
  int count = fetch_byte() + 1;
  if ((int)(current_ip - run_thread->script) & 1)
    current_ip++;
#ifdef DEBUG
  printf("switch case (%x) ; %d\n", current_value, current_value);
  printf("ip = %x count = %d\n", (int)(current_ip - run_thread->script), count);
#endif
  for (int i = 0; i < count; i++) {
    short val = *(short *)current_ip;
#ifdef DEBUG
    printf("case %x ; %d\n", val, val);
#endif
    if (current_value == val){
      current_ip += 2;
      current_ip += *(short *)current_ip + 2;
      break;
    } else if (current_value < val) {
      current_ip += (count - i) * 4;
      break;
    }
    current_ip += 4;
  }
#ifdef DEBUG
  printf("ip = %x\n", (int)(current_ip - run_thread->script));
#endif
}

/** 
 * Для главной программы - вызов с сохранением (однократное 
 * сохранение в стеке.
 * Для обработчиков - откладывание вызова, до основной программы
 * @param s смещение вызова
 */
void call_resume(int s)
{
  if (main_run) { // главная программа потока
    if (run_thread->saved_sp) { // если был сохраненный sp
      run_thread->call_stack->sp = run_thread->saved_sp; // устанавливаем стек в сохраненный
      current_ip += s; // вызов без сохранения адреса возврата
#ifdef DEBUG
      printf("call resume main saved ip = %x\n", (int)(current_ip - run_thread->script));
#endif
      exit(1);
    } else {
      call(s); // вызов с сохранением
      run_thread->saved_sp = run_thread->call_stack->sp; // сохраняем sp
#ifdef DEBUG
      printf("call resume main not saved ip = %x\n", (int)(current_ip - run_thread->script));
#endif
    }
  } else { // обработка сообщений или клавиш
    if (run_thread->saved_sp) { // если был сохраененный sp
      saved_sp = run_thread->saved_sp; // модифицируем sp, который будет указателем стека вызовов
#ifdef DEBUG
      printf("call resume no main saved\n");
#endif
    } else {
      run_thread->saved_sp = --saved_sp; // текущий стек сохраняем
      saved_sp = run_thread->saved_sp;
      *saved_sp = (int)run_thread->ip; // записываем текущий ip
#ifdef DEBUG
      printf("call resume no main no saved\n");
#endif
    }
    run_thread->ip += s; // ip для основной программы модифицируется
    run_thread->running = 1; // программа запускается
    run_thread->cur_frames_to_skip = 1; // пропуска кадра не будет
#ifdef DEBUG
    printf("ip = %x\n", (int)(current_ip - run_thread->script));
#endif
    exit(1);
  }
}

/// вызов word + 1 с возможным возвратом к сохраненному значению
void call_skip_word_save()
{
  int s = (short)fetch_word();
  current_ip++;
  call_resume(s);
#ifdef DEBUG
  printf("call skip word save: %d ip = %x\n", s, (int)(current_ip - run_thread->script));
#endif
}
