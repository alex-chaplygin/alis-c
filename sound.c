#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "get.h"
#include "image.h"

void  add_sound(int s1)
{
  if (s1 < 0) {
    printf("play sound s1 < 0\n");
    exit(1);
  }
}

/** 
 * Проигрывание звука через синтезатор (заглушка)
 * 
 */
void play_sound_synth()
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
  printf("play_sound_synth %d %d %d %d\n", s1, s2, s3, s4);
#endif
  if (!s4)
    return;
  int s = (s2 << 8) % s4;
  //  add_sound(s1);
}

/** 
 * Проигрывание звука из ресурса
 * 
 */
void play_sound()
{
  load_main_res = 0;
  new_get();
  int num = current_value;
  new_get();
  int s1 = current_value;
  new_get();
  int s2 = current_value;
  new_get();
  int s3 = current_value;
  new_get();
  int s4 = current_value;
#ifdef DEBUG
  printf("play_sound_res %d %d %d %d %d\n", num, s1, s2, s3, s4);
#endif
}
