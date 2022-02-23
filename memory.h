#ifndef __MEMORY__
#define __MEMORY__

#include "types.h"

typedef struct {
  int *data;			/**< данные стека */
  int size;			/**< размер стека */
  int *sp;			/**< указатель стека */
} stack_t;

typedef struct {
  byte *data;			/**< данные сегмента */
  int size;			/**< размер сегмента */
} seg_t;

void *xmalloc(int size);
void memory_init(int size);
seg_t *memory_alloc(int size);
stack_t *stack_new(int size);
void stack_push(stack_t *s, int val);
int stack_pop(stack_t *s);
byte *seg_read(seg_t * s, word adr);
void seg_write_byte(seg_t* s, word adr, byte b);
void seg_write_word(seg_t* s, word adr, word w);
byte *memory_read(int pos);
void stack_free(stack_t *s);
void memory_free(seg_t *s);
int stack_empty(stack_t *s);
void stack_clear(stack_t *s);

extern byte *memory;		/**< главная общая память данных потоков */

#endif
