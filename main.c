/**
 * @file   main.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 17:02:13 2022
 * 
 * @brief  Главная программа
 * 
 */

#include <stdio.h>
#include "objects.h"
#include "graphics.h"

int main(int argc, char *argv[])
{
  byte color;
  graphics_init();
  objects_init();
  while (1) {
    objects_run();
    if (!graphics_update())
      break;
  }
  graphics_close();
  return 0;
}
