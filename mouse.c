/**
 * @file   mouse.c
 * @author alex <alex@localhost>
 * @date   Mon Apr  4 08:05:40 2022
 * 
 * @brief  Управление курсором мыши
 *
 * Системный курсор скрыт, вместо него рисуется изображение курсора
 */
#include <SDL.h>

int show_cursor = -1;	/**< флаг курсора мыши */
int mouse_x;			/**< координаты курсора */
int mouse_y;
int mouse_buttons;		/**< состояние кнопок мыши */

/** 
 * Установка параметров при событиях мыши
 * Может использоваться при установке курсора мыши в новую позицию
 *
 * @param x координата x
 * @param y координата y
 * @param buttons кнопки
 */
void mouse_event(int x, int y, int buttons)
{
  mouse_x = x;
  mouse_y = y;
  mouse_buttons = buttons;
}

/** 
 * Получение состояния мыши
 */
void mouse_get(int *x, int *y, int *buttons)
{
  *x = mouse_x;
  *y = mouse_y;
  *buttons = mouse_buttons;
}

void mouse_init()
{
  SDL_ShowCursor(SDL_FALSE);
}

void mouse_draw_cursor()
{
}
