#include <stdio.h>
#include <stdlib.h>
#include "vm.h"
#include "threads.h"
#include "types.h"
#include "test.h"

extern byte *current_ip;
extern thread_t *current_thread;

thread_t thread;
word stack[20];

void sprites_set_translate(word *vec)
{
}

void sprites_translate(sprite_t *list, word *vec)
{
}

void view_update()
{
}

int io_num_free_files()
{
  return 0;
}

int sprites_free_num()
{
  return 0;
}

void video_update_events()
{
}

struct {
  word start_ip;
  byte prog[100];
  word end_ip;
} call_table[] = {
  {0x70 - 0x42, {
      0x42, 00, 00, 0x14, 0x14, 0x1E, 0, 0, 0x14, 0x16, 0xCD,
      0x14, 0x16, 0x14, 0x16, 0x14, 0x14, 0x20, 0, 01, 14, 16, 0x1F,
      38, 14, 16, 0x4E, 00, 9, 0x3A, 15, 0xEA, 11, 0xD5, 00, 01, 0x1E,
      0, 20, 14, 10, 0x1E, 0, 0x7F, 14, 0x0E, 0x5, 0xd0},
   0x42 - 0x42 + 1},
  {0, {
      0x6, 0x3, 0x0, 0x14, 0x14, 0x1E, 0x42,},
   7},
  {1, {
      0x42, 0x6, 0xfc, 0xff, 0x14, 0x14, 0x1E, 0x42,},
   1},
  {0, {
      0x7, 0x3, 0x00, 0x00, 0x7, 0x14, 0x1E, 0x42,},
   8},
  {1, {
      0x42, 0x7, 0xfb, 0xff, 0, 0x1, 0x14, 0x1E, 0x42,},
   1},
  {0x440d - 0x4401, {
      0x42, 38, 6, 4, 0, 0, 0x3, 0x1, 0x3, 0, 0, 42, 8, 0xf2},
   1},
};

void test_call_jmp()
{
  current_thread = &thread;
  current_thread->call_stack = stack;
  current_thread->call_sp = 5;
  printf("prog = %x\n", call_table[0].prog[0]);
  for (int i = 0; i < sizeof(call_table) / sizeof(call_table[0]); i++) {
    current_thread->io_file = call_table[i].prog;
    ASSERT(vm_run(call_table[i].prog + call_table[i].start_ip) - call_table[i].prog, call_table[i].end_ip);
  }
  ASSERT(stack[4], 0x72 - 0x42);
  ASSERT(stack[3], 3);
  ASSERT(stack[2], 4);
  ASSERT(stack[1], 4);
  ASSERT(stack[0], 5);
}

void main()
{
  test_call_jmp();
}
