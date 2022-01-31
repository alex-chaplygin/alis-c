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
  current_value = *seg_read(run_thread->data, w);
#ifdef DEBUG
  printf("get byte varw_%x: %x\n", w, current_value);
#endif
}

/// чтение переменной типа word по адресу word
void get_word_mem_word()
{
  word w = fetch_word();
  current_value = *(word *)seg_read(run_thread->data, w);
#ifdef DEBUG
  printf("get word varw_%x: %x\n", w, current_value);
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
  current_value = *seg_read(run_thread->data, w);
#ifdef DEBUG
  printf("get byte varb_%x: %x\n", w, current_value);
#endif
}

/// чтение переменной типа word по адресу byte
void get_word_mem_byte()
{
  byte w = fetch_byte();
  current_value = *(word *)seg_read(run_thread->data, w);
#ifdef DEBUG
  printf("get word varb_%x: %x\n", w, current_value);
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

/** 
 * Получение строки из массива
 * Индекс массива - в аккумуляторе (другие индексы если есть - в стеке)
 */
/*
void get_string_array_word()
{
  word w = fetch_word();
  byte *ip = current_ip;
  current_ip = get_array_pos(seg_read(run_thread->data, w));
  get_string();
#ifdef DEBUG
  printf("get string array_%x[%d, %d]: %s\n", w, current_value, *(stack.sp - 1), string1);
#endif
  current_ip = ip;
}

void get_byte_array_word()
{
  word w = fetch_word();
  byte *ip = current_ip;
  current_ip = get_array_pos(seg_read(run_thread->data, w));
  word w = WORD;
  int pos = get_array_pos(w, 0);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  current_value = current_script->mem[pos];
  printf("get byte array[%x]: %x\n", w, current_value);
}

void get_word_array_word()
{
  word w = WORD;
  int pos = get_array_pos(w, 1);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  current_value = *(word *)&current_script->mem[pos];
  printf("get word array[%x]: %x\n", w, current_value);
}

void get_byte_mem_byte()
{
  byte w = BYTE;
  if (w >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }  
  current_value = current_script->mem[w];
  printf("get byte mem[%x]: %x\n", w, current_value);
}

void get_word_mem_byte()
{
  byte w = BYTE;
  if (w >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }  
  current_value = *(word *)&current_script->mem[w];
  printf("get word mem[%x]: %x\n", w, current_value);
}

void get_string_mem_byte()
{
  byte w = BYTE;
  if (w >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }  
  char *c = &current_script->mem[w];
  char *dst = current_string;
  do *dst++ = *c++ while (*(c - 1));
  if (dst - current_string >= MAX_STR) {
    printf("MAX get string\n");
    exit(1);
  }
  printf("get string mem[%x]: %s\n", w, current_string);
}

void get_string_array_byte()
{
  byte w = BYTE;
  int pos = get_string_array_pos(w);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  char *c = &current_script->mem[pos];
  char *dst = current_string;
  do *dst++ = *c++ while (*(c - 1));
  if (dst - current_string >= MAX_STR) {
    printf("MAX get string\n");
    exit(1);
  }
  printf("get string array[%x]: %s\n", w, current_string);
}

void get_byte_array_byte()
{
  byte w = BYTE;
  int pos = get_array_pos(w, 0);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  current_value = current_script->mem[pos];
  printf("get byte array[%x]: %x\n", w, current_value);
}

void get_word_array_byte()
{
  byte w = BYTE;
  int pos = get_array_pos(w, 1);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  current_value = *(word *)&current_script->mem[pos];
  printf("get word array[%x]: %x\n", w, current_value);
}

void get_byte_global()
{
  word w = WORD;
  if (w >= main_memory_size) {
    printf("global memory out of range\n");
    exit(1);
  }  
  current_value = main_memory[w];
  printf("get byte global mem[%x]: %x\n", w, current_value);
}

void get_word_global()
{
  word w = WORD;
  if (w >= main_memory_size) {
    printf("global memory out of range\n");
    exit(1);
  }  
  current_value = *(word *)&main_memory[w];
  printf("get word global mem[%x]: %x\n", w, current_value);
}

void get_string_global()
{
  word w = WORD;
  if (w >= main_memory_size) {
    printf("memory out of range\n");
    exit(1);
  }  
  char *c = &main_memory[w];
  char *dst = current_string;
  do *dst++ = *c++ while (*(c - 1));
  if (dst - current_string >= MAX_STR) {
    printf("MAX get string\n");
    exit(1);
  }
  printf("get string global mem[%x]: %s\n", w, current_string);
}

void get_string_global_array_word()
{
  word w = WORD;
  struct script *s = current_script;
  current_script = script_list;
  int pos = get_string_array_pos(w);
  if (pos >= main_memory_size) {
    printf("memory out of range\n");
    exit(1);
  }
  char *c = &main_memory[pos];
  char *dst = current_string;
  do *dst++ = *c++ while (*(c - 1));
  if (dst - current_string >= MAX_STR) {
    printf("MAX get string\n");
    exit(1);
  }
  current_script = s;
  printf("get string global array[%x]: %s\n", w, current_string);
}

void get_byte_global_array_word()
{
  word w = WORD;
  struct script *s = current_script;
  current_script = script_list;
  int pos = get_array_pos(w, 0);
  if (w >= main_memory_size) {
    printf("memory out of range\n");
    exit(1);
  }
  current_value = main_memory[pos];
  current_script = s;
  printf("get byte global array[%x]: %x\n", w, current_value);
}

void get_word_global_array_word()
{
  word w = WORD;
  struct script *s = current_script;
  current_script = script_list;
  int pos = get_array_pos(w, 1);
  if (w >= main_memory_size) {
    printf("memory out of range\n");
    exit(1);
  }
  current_value = *(word *)&main_memory[pos];
  current_script = s;
  printf("get word global array[%x]: %x\n", w, current_value);
}

void get_byte_pointer()
{
  word scr = WORD;
  word adr = WORD;
  struct script *s = find_script(scr);
  if (!s) {
    printf("No script: %x\n", scr);
    exit(1);
  }
  if (adr >= s->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  current_value = s->mem[adr];
  printf("get byte pointer: [%x:%x] -> %x\n", scr, adr, current_value);
}

void get_word_pointer()
{
  word scr = WORD;
  word adr = WORD;
  struct script *s = find_script(scr);
  if (!s) {
    printf("No script: %x\n", scr);
    exit(1);
  }
  if (adr >= s->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  current_value = *(word *)&s->mem[adr];
  printf("get word pointer: [%x:%x] -> %x\n", scr, adr, current_value);
}

void get_string_pointer()
{
  word scr = WORD;
  word adr = WORD;
  struct script *s = find_script(scr);
  if (!s) {
    printf("No script: %x\n", scr);
    exit(1);
  }
  if (adr >= s->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  char *c = &s->mem[adr];
  char *dst = current_string;
  do *dst++ = *c++ while (*(c - 1));
  if (dst - current_string >= MAX_STR) {
    printf("MAX get string\n");
    exit(1);
  }
  printf("get string pointer [%x:%x]: %s\n", scr, adr, current_string);
}

void get_string_pointer_array()
{
  word scr = WORD;
  word adr = WORD;
  struct script *s = find_script(scr);
  struct script *s2;
  if (!s) {
    printf("No script: %x\n", scr);
    exit(1);
  }
  if (adr >= s->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  s2 = current_script;
  current_script = s;
  int pos = get_string_array_pos(adr);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  char *c = &current_script->mem[pos];
  char *dst = current_string;
  do *dst++ = *c++ while (*(c - 1));
  if (dst - current_string >= MAX_STR) {
    printf("MAX get string\n");
    exit(1);
  }
  printf("get string array pointer[%x:%x]: %s\n", scr, adr, current_string);
  current_script = s2;
}

void get_byte_pointer_array()
{
  word scr = WORD;
  word adr = WORD;
  struct script *s = find_script(scr);
  struct script *s2;
  if (!s) {
    printf("No script: %x\n", scr);
    exit(1);
  }
  if (adr >= s->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  s2 = current_script;
  current_script = s;
  int pos = get_array_pos(adr, 0);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  current_value = current_script->mem[pos];
  printf("get byte pointer array[%x:%x]: %x\n", scr, adr, current_value);
  current_script = s2;
}

void get_word_pointer_array()
{
  word scr = WORD;
  word adr = WORD;
  struct script *s = find_script(scr);
  struct script *s2;
  if (!s) {
    printf("No script: %x\n", scr);
    exit(1);
  }
  if (adr >= s->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  s2 = current_script;
  current_script = s;
  int pos = get_array_pos(adr, 1);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  current_value = *(word *)&current_script->mem[pos];
  printf("get byte pointer array[%x:%x]: %x\n", scr, adr, current_value);
  current_script = s2;
}

void set_byte_mem_word()
{
  word w = WORD;
  if (w >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }  
  current_script->mem[w] = (byte)current_value;
  printf("set byte %x %d\n", w, (byte)current_value);
}

void set_word_mem_word()
{
  word w = WORD;
  if (w >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }  
  *(short *)&current_script->mem[w] = current_value;
  printf("set word %x %d\n", w, current_value);
}

void set_string_mem_word()
{
  word w = WORD;
  if (w >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  strcpy(&current_script->mem[w], current_string);
  printf("set string %x %s\n", w, current_string);
}

void set_string_array_word()
{
  word w = WORD;
  int pos = get_string_array_pos(w);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  strcpy(&current_script->mem[pos], current_string);
  printf("set string array %x %d %s\n", w, pos, current_string);
}

void set_word_array_byte()
{
  word w = WORD;
  int pos = get_array_pos(w, 0);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  if (stack_pos >= MAX_STACK) {
    printf("Stack is empty\n");
    exit(1);
  }
  current_value = *(short *)&stack[stack_pos];
  stack_pos += 2;
  current_script->mem[pos] = (byte)current_value;
  printf("set word array %x %d %s\n", w, pos, current_value);  
}

void set_word_array_word()
{
  word w = WORD;
  int pos = get_array_pos(w, 1);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  if (stack_pos >= MAX_STACK) {
    printf("Stack is empty\n");
    exit(1);
  }
  current_value = *(short *)&stack[stack_pos];
  stack_pos += 2;
  current_script->mem[pos] = (byte)current_value;
  printf("set word array %x %d %s\n", w, pos, current_value);  
}
    
void set_byte_mem_byte()
{
  byte w = BYTE;
  if (w >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }  
  current_script->mem[w] = (byte)current_value;
  printf("set byte mem[%x]: %d\n", w, (byte)current_value);  
}

void set_word_mem_byte()
{
  byte w = BYTE;
  if (w >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }  
  *(short *)&current_script->mem[w] = current_value;
  printf("set byte mem[%x]: %d\n", w, current_value);  
}

void set_string_mem_byte()
{
  byte w = BYTE;
  if (w >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  strcpy(&current_script->mem[w], current_string);
  printf("set string mem[%x] %s\n", w, current_string);
}

void set_string_array_byte()
{
  word w = WORD;
  int pos = get_string_array_pos(w);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  strcpy(&current_script->mem[pos], current_string);
  printf("set string mem array [%x][%d] %s\n", w, pos, current_string);
}


void set_byte_stack_array_byte()
{
  byte w = BYTE;
  int pos = get_array_pos(w, 0);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  if (stack_pos >= MAX_STACK) {
    printf("Stack is empty\n");
    exit(1);
  }
  current_value = *(short *)&stack[stack_pos];
  stack_pos += 2;
  current_script->mem[pos] = (byte)current_value;
  printf("set byte array %x %d %s\n", w, pos, (byte)current_value);  
}

void set_word_stack_array_byte()
{
  byte w = BYTE;
  int pos = get_array_pos(w, 1);
  if (pos >= current_script->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  if (stack_pos >= MAX_STACK) {
    printf("Stack is empty\n");
    exit(1);
  }
  current_value = *(short *)&stack[stack_pos];
  stack_pos += 2;
  current_script->mem[pos] = (byte)current_value;
  printf("set byte array %x %d %s\n", w, pos, (byte)current_value);  
}

void set_byte_global()
{
  word w = WORD;
  if (w >= main_memory_size) {
    printf("global memory out of range\n");
    exit(1);
  }  
  main_memory[w] = (byte)current_value;
  printf("set byte global mem[%x]: %x\n", w, (byte)current_value);
}

void set_word_global()
{
  word w = WORD;
  if (w >= main_memory_size) {
    printf("global memory out of range\n");
    exit(1);
  }  
  *(short *)&main_memory[w] = current_value;
  printf("set word global mem[%x]: %x\n", w, current_value);
}

void set_string_global()
{
  word w = WORD;
  if (w >= main_memory_size) {
    printf("global memory out of range\n");
    exit(1);
  }  
  strcpy(&main_memory[w], current_string);
  printf("set word global mem[%x]: %x\n", w, current_string);
}

void set_string_global_array()
{
  word w = WORD;
  int pos = get_string_array_pos(w);
  if (pos >= main_memory_size) {
    printf("memory out of range\n");
    exit(1);
  }
  strcpy(&main_memory[pos], current_string);
  printf("set string global mem array [%x][%d] %s\n", w, pos, current_string);
}

void set_byte_global_array()
{
  word w = WORD;
  int pos = get_array_pos(w, 0);
  if (pos >= main_memory_size) {
    printf("memory out of range\n");
    exit(1);
  }
  if (stack_pos >= MAX_STACK) {
    printf("Stack is empty\n");
    exit(1);
  }
  current_value = *(short *)&stack[stack_pos];
  stack_pos += 2;
  main_memory[w] = (byte)current_value;
  printf("set byte global array[%x]: %d\n", w, (byte)current_value);
}

void set_word_global_array()
{
  word w = WORD;
  int pos = get_array_pos(w, 1);
  if (pos >= main_memory_size) {
    printf("memory out of range\n");
    exit(1);
  }
  if (stack_pos >= MAX_STACK) {
    printf("Stack is empty\n");
    exit(1);
  }
  current_value = *(short *)&stack[stack_pos];
  stack_pos += 2;
  main_memory[w] = current_value;
  printf("set word global array[%x]: %d\n", w, current_value);
}

void set_byte_far()
{
  word scr = WORD;
  word adr = WORD;
  struct script *s = find_script(scr);
  if (!s) {
    printf("No script: %x\n", scr);
    exit(1);
  }
  if (adr >= s->mem_size) {
    printf("memory out of range\n");
    exit(1);
  }
  s->mem[adr] = (byte)current_value;
  printf("set byte pointer: [%x:%x] -> %x\n", scr, adr, (byte)current_value);
}

void set_word_far()
{
  printf("no set w far\n");
  exit(1);
}

void set_string_far()
{
  printf("no set str far\n");
  exit(1);
}

void set_string_far_array()
{
  printf("no set str far arr far\n");
  exit(1);
}

void set_byte_far_array()
{
  printf("no set b arr far\n");
  exit(1);
}

void set_word_far_array()
{
  printf("no set w array far\n");
  exit(1);
}
*/
