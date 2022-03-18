/**
 * @file   mouse.c
 * @author alex <alex@localhost>
 * @date   Wed Mar  9 17:41:44 2022
 * 
 * @brief  Интерпретатор - команды работы с мышью
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "image.h"
#include "sprite.h"
#include "get.h"
#include "graphics.h"

#define CURSOR_LEN 8		/**< размер курсора мыши */

byte under_cursor_image[CURSOR_LEN * CURSOR_LEN]; /**< изображение под курсором мыши */
int show_cursor = -1;	/**< флаг курсора мыши */

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
  current_value = mouse_x;
  switch_string_store();
  current_value = mouse_y;
  switch_string_store();
  current_value = mouse_buttons;
  switch_string_store();
}

/** 
 * Команда - показать курсор мыши
 */
void show_mouse_cursor()
{
#ifdef DEBUG
  printf("show mouse cursor\n");
#endif
}

/** 
 * Отрисовка курсора мыши
 * 
 */
void draw_mouse_cursor()
{
  if (show_cursor == -1)
    return;
  printf("draw mouse cursor\n");
  exit(1);
}
