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
#include <string.h>
#include "interpret.h"
#include "sprite.h"
#include "get.h"
#include "res.h"
#include "draw.h"
#include "graphics.h"

#define CURSOR_LEN 16		/**< размер курсора мыши */

int mouse_x;			/**< координаты мыши текущие */
int mouse_y;
int mouse_buttons;
byte cursor_image[CURSOR_LEN * CURSOR_LEN]; /**< изображение курсора мыши */
int show_cursor = -1;	/**< флаг курсора мыши */

/** 
 * Установка курсора мыши как изображение из ресурсов
 */
void set_mouse_cursor()
{
  load_main_res = 0;
  new_get();
  cursor_sprite->image = res_get_image(current_value);
  cursor_sprite->render_image = cursor_sprite->image;
  image_t *im = (image_t *)cursor_sprite->image;
  memset(cursor_image, 0, CURSOR_LEN * CURSOR_LEN);
  draw_cursor(im, cursor_image, CURSOR_LEN, CURSOR_LEN);
  graphics_set_cursor(cursor_image, CURSOR_LEN, CURSOR_LEN); 
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
  show_cursor = 1;
}
