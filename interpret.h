#include "objects.h"

#define MAX_STACK 20
#define MAX_STR 130
#define ASSERT(v1, v2) \
  {\
    if ((v1) != (v2)) {				\
      printf("Assertion failed %d != %d\n", (int)(v1), (int)(v2));	\
      exit(1);\
    }}	      
    
typedef int (*func)();

byte *interpret(object_t *t, byte *ip);
byte fetch_byte();
word fetch_word();
void null_op();
void no_func();
void nimp();
void yield();

extern byte *current_ip;
extern object_t *run_object;
extern int main_run;
extern int interpreting;
extern short current_value;		/**< аккумулятор */
extern short prev_value;		/**< второй регистр */
extern stack_t stack;			/**< стек для промежуточных результатов вычислений */
extern int stack_data[MAX_STACK];
extern char get_string_buf[MAX_STR];
extern char text_string_buf[MAX_STR];
extern char store_string_buf[MAX_STR];
extern char string_buf4[MAX_STR];
extern char *get_string;
extern char *text_string;
extern char *store_string;
extern word *objects_list_pos;

