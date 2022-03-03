#include "threads.h"

#define MAX_STACK 20
#define MAX_STR 30
#define ASSERT(v1, v2) \
  {\
    if ((v1) != (v2)) {				\
      printf("Assertion failed %d != %d\n", (int)(v1), (int)(v2));	\
      exit(1);\
    }}	      
    
typedef int (*func)();

byte *interpret(thread_t *t, byte *ip);
byte fetch_byte();
word fetch_word();
void null_op();
void no_func();
void nimp();
void yield();
void mouse_read();

extern byte *current_ip;
extern thread_t *run_thread;
extern int no_saved_return;
extern int interpreting;
extern short current_value;		/**< аккумулятор */
extern short prev_value;		/**< второй регистр */
extern stack_t stack;			/**< стек для промежуточных результатов вычислений */
extern int stack_data[MAX_STACK];
extern char get_string_buf[MAX_STR];
extern char string_buf2[MAX_STR];
extern char store_string_buf[MAX_STR];
extern char string_buf4[MAX_STR];
extern char *get_string;
extern char *string2;
extern char *store_string;
extern word *thread_array_pos;

