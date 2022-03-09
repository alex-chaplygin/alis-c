/**
 * @file   mouse.c
 * @author alex <alex@localhost>
 * @date   Wed Mar  9 17:41:44 2022
 * 
 * @brief  Интерпретатор - команды работы с мышью
 * 
 */

#include <stdio.h>
#include "interpret.h"
#include "image.h"
#include "sprite.h"
#include "get.h"

/** 
 * Установка курсора мыши как изображение из ресурсов
 */
void set_mouse_cursor()
{
  load_main_res = 0;
  new_get();
  cursor_sprite->image = get_resource(current_value);
#ifdef DEBUG
  printf("set mouse cursor resource = %x\n", current_value);
#endif
}

/// Чтение состояния мыши в 3 переменные
void mouse_read()
{
#ifdef DEBUG
  printf("set mouse_x mouse_y buttons:\n");
#endif
  // get mouse x, y, buttons
  current_value = 0;
  switch_string_store();
  current_value = 0;
  switch_string_store();
  current_value = 0;
  switch_string_store();
}
