/**
 * @file   memory.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 17:05:25 2022
 * 
 * @brief  Функции для работы с памятью, стеком
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "memory.h"

byte *memory;		/**< память главного потока */
int memory_size;		/**< размер памяти главного потока*/

/** 
 * Безопасное выделение памяти
 * 
 * @param size размер памяти
 * 
 * @return указатель на новую область
 */
void *xmalloc(int size)
{
  byte *p = malloc(size);
  if (!p) {
    printf("Cannot allocate %d bytes\n", size);
    exit(1);
  }
  return p;
}

/** 
 * Инициализация главной памяти
 * 
 * @param size размер памяти
 */
void memory_init(int size)
{
  memory_size = size;
  memory = 0;
}

/** 
 * выделяет сегмент памяти
 * 
 * @param size размер сегмента
 * 
 * @return структуру сегмента
 */
seg_t *memory_alloc(int size)
{
#ifdef DEBUG
  printf("Mem alloc: %d\n", size);
#endif
  seg_t *s = xmalloc(sizeof(seg_t));
  s->data = xmalloc(size);
  s->size = size;
  memset(s->data, 0, s->size);
  if (!memory) {
    memory = s->data;
    memory_size = s->size;
  }
  return s;
}

/** 
 * Создает новый стек
 * 
 * @param size размер стека
 * 
 * @return структура стека
 */
stack_t *stack_new(int size)
{
  stack_t *s = xmalloc(sizeof(stack_t));
  s->data = xmalloc(size * sizeof(int));
  s->size = size;
  s->sp = s->data + size;
  return s;
}

/** 
 * Запись значения в стек
 * 
 * @param s стек
 * @param val значение
 */
void stack_push(stack_t *s, int val)
{
  if (--s->sp < s->data) {
    printf("Stack %d is full\n", s->size);
    exit(1);
  }
  *s->sp = val;
}

/** 
 * Возврат значения из стека
 * 
 * @param s стек
 * 
 * @return значение
 */
int stack_pop(stack_t *s)
{
  int val = *s->sp;
  if (++s->sp > s->data + s->size) {
    printf("Stack %d is empty\n", s->size);
    exit(1);
  }
  return val;
}

/// проверка границ сегмента
void check_adr(seg_t * s, word adr)
{
  if (adr >= s->size) {
    printf("Segment: out of bounds adr = %x\n", adr);
    exit(1);
  }
}

/** 
 * Чтение из сегмента памяти. Проверка границ
 * 
 * @param s сегмент
 * @param adr адрес
 * 
 * @return указатель на ячейку памяти
 */
byte *seg_read(seg_t * s, word adr)
{
  check_adr(s, adr);
  return s->data + adr;
}

/// запись байта в сегмент
void seg_write_byte(seg_t* s, word adr, byte b)
{
  check_adr(s, adr);
  *(s->data + adr) = b;
}

/// запись слова в сегмент
void seg_write_word(seg_t* s, word adr, word w)
{
  check_adr(s, adr);
  *(word *)(s->data + adr) = w;
}

/// чтение из главной памяти с проверкой границ
byte *memory_read(int pos)
{
  if (pos >= memory_size) {
    printf("Memory out of bounds: %d > %d\n", pos, memory_size);
    exit(1);
  }
  return memory + pos;
}
