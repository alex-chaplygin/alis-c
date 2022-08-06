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
#include "sprite.h"
#include "get.h"
#include "res.h"
#include "draw.h"
#include "graphics.h"

#define CURSOR_LEN 16		/**< размер курсора мыши */

int mouse_x;			/**< координаты мыши текущие */
int mouse_y;
int screen_mouse_x = 0;		/**< координаты мыши для отрисовки курсора */
int screen_mouse_y = 0;
int mouse_buttons;
byte under_cursor_image[CURSOR_LEN * CURSOR_LEN]; /**< изображение под курсором мыши */
int cursor_width;		/**< ширина и высота изображения под курсором */
int cursor_height;
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
  vec_t v = {screen_mouse_x, screen_mouse_y};
#ifdef DEBUG
  printf("show mouse cursor\n");
#endif
  show_cursor = 1;
  // сохранить ихображение под курсором
  graphics_read_buffer(screen_mouse_x, screen_mouse_y, CURSOR_LEN, CURSOR_LEN,
		       under_cursor_image, &cursor_width, &cursor_height);
  // нарисовать курсор
  rectangle_t r = {screen_mouse_x, screen_mouse_y,
		   screen_mouse_x + cursor_width - 1, screen_mouse_y + cursor_height - 1};
  draw_image(&v, (image_t *)cursor_sprite->image, 0, &r);
}

/** 
 * Отрисовка курсора мыши
 */
void draw_mouse_cursor()
{
  if (show_cursor < 0)
    return;
  if (screen_mouse_x == mouse_x && screen_mouse_y == mouse_y)
    return;
  // стираем старую позицию курсора, рисуем сохраненное изображение
  graphics_write_buffer(screen_mouse_x, screen_mouse_y, cursor_width, cursor_height, under_cursor_image);
  cursor_sprite->origin.x = screen_mouse_x = mouse_x;
  cursor_sprite->origin.y = screen_mouse_y = mouse_y;
  cursor_sprite->origin.z = 0;
  // запоминаем новое изображение
  graphics_read_buffer(screen_mouse_x, screen_mouse_y, CURSOR_LEN, CURSOR_LEN,
		       under_cursor_image, &cursor_width, &cursor_height);
  // рисуем курсор
  vec_t v = {screen_mouse_x, screen_mouse_y};
  rectangle_t r = {screen_mouse_x, screen_mouse_y,
		   screen_mouse_x + cursor_width - 1, screen_mouse_y + cursor_height - 1};
  draw_image(&v, (image_t *)cursor_sprite->image, 0, &r);
}
