#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "get.h"

void  add_sound(int s1)
{
  if (s1 < 0) {
    printf("play sound s1 < 0\n");
    exit(1);
  }
}

void play_sound()
{
  new_get();
  int s1 = (char)current_value;
  new_get();
  int s2 = (char)current_value;
  new_get();
  int s3 = current_value;
  new_get();
  int s4 = current_value;
#ifdef DEBUG
  printf("play_sound %d %d %d %d\n", s1, s2, s3, s4);
#endif
  if (!s4)
    return;
  int s = (s2 << 8) % s4;
  add_sound(s1);
}
