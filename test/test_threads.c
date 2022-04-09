#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threads.h"
#include "test.h"
#include "io.h"

extern threads_table_t *threads_table;

threads_table_t table;
thread_t t;
io_header_t h;

byte *vm_run(byte *ip)
{
  word i = (word)(ip - (byte *)&h);
  printf("vm run %x\n", i);
  if (i == 0x18)
    return (byte *)&h + 0x5cc;
  return ip;
}

void sprites_set_translate(word *vec)
{
}

void sprites_translate(sprite_t *list, word *vec)
{
}

void view_update()
{
}

void test_run()
{
  threads_table = &table;
  table.next = 0;
  table.thread = &t;
  memset(&t, 0, sizeof(thread_t));
  t.io_file = (byte *)&h;
  t.flags = 2;
  t.running = -1;
  t.frame_num = 1;
  t.num_frames = 1;
  t.ip = 0x18;
  h.key_entry = 0x4dc9;
  threads_run();
  ASSERT(t.running, 1);
  ASSERT(t.ip, 0x5cc);
  ASSERT(t.frame_num, 1);
  t.flags |= THREAD_MSG;
  threads_run();
  ASSERT(t.running, 1);
  ASSERT(t.ip, 0x5cc);
  ASSERT(t.frame_num, 1);
  t.flags |= THREAD_NOMSG;
  h.message_entry = 0x16;
  threads_run();
  ASSERT(t.running, 1);
  ASSERT(t.ip, 0x5cc);
  ASSERT(t.frame_num, 1);
  t.flags = 0x80;
  t.running = 0;
  t.ip = 1;
  threads_run();
  ASSERT(t.running, 0);
  ASSERT(t.ip, 1);
  ASSERT(t.frame_num, 1);
  t.running = 1;
  t.num_frames = 3;
  t.frame_num = 3;
  threads_run();
  ASSERT(t.running, 1);
  ASSERT(t.ip, 1);
  ASSERT(t.frame_num, 2);
}

void main()
{
  test_run();
}
