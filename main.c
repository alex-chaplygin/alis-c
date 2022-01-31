/**
 * @file   main.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 17:02:13 2022
 * 
 * @brief  Главная программа
 * 
 */

#include <stdio.h>
#include "threads.h"
#include "graphics.h"

int main(int argc, char *argv[])
{
  byte color;
  graphics_init();
  thread_init();
  while (1) {
    threads_run();
    for (int i = 0; i < 320 * 200; i++)
      video_buffer[i] = color;
      color++;
    if (!graphics_update())
      break;
  }
  graphics_close();
  return 0;
}
