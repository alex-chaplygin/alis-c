/**
 * @file   text.c
 * @author alex <alex@localhost>
 * @date   Sat Mar  5 18:39:01 2022
 * 
 * @brief  Команды - работа с текстом
 * 
 */

#include <stdio.h>
#include "interpret.h"
#include "get.h"
#include "vector.h"
#include "sprite.h"
#include "res.h"

vec_t text_pos;			/**< текущая позиция вывода символа */
int text_tag;			/**< номер объекта для текста */
int text_height;		/**< высота строки текста при переводе строк */
int char_width;			/**< расстояние между левым краем символов */
int font_resource = -1;		/**< номер ресурса, откуда начинаются символы шрифта */
int min_char;			/**< минимальный код символа в шрифте */
int max_char;			/**< максимальный код символа в шрифте */

/** 
 * Установка параметров текста
 */
void set_text_params()
{
  new_get();
  min_char = current_value;
  new_get();
  font_resource = current_value;
  new_get();
  char_width = (byte)current_value;
  new_get();
  text_height = (byte)current_value;
  new_get();
  max_char = current_value;
#ifdef DEBUG
  printf("set text params min = %d font = %d width = %d height = %d max = %d\n", min_char, font_resource, char_width, text_height, max_char);
#endif
}

/** 
 * Установка позиции для вывода текста
 * Установка номера объекта текста
 */
void set_text_pos()
{
  new_get();
  text_pos.x = current_value;
  new_get();
  text_pos.y = current_value;
  new_get();
  text_pos.z = current_value;
  new_get();
  text_tag = (char)current_value;
#ifdef DEBUG
  printf("set text pos (%d %d %d) tag = %x\n", text_pos.x, text_pos.y, text_pos.z, text_tag);
#endif
}

/** 
 * Печать символа. Вычисляет номер спрайта из шрифта
 * Выполняет перемещение позиции печати
 * @param c код символа
 */
void print_char(char c)
{
#ifdef DEBUG
  printf("print char: %c\n", c);
#endif
  switch (c) {
  case '\n': text_pos.z -= text_height; break;
  case '\r': text_pos.x = 0; break;
  case ' ': text_pos.x += char_width; break;
  default:
    if (font_resource < 0)
      break;
    c -= min_char;
    if (c < 0)
      break;
    if (c >= max_char)
      break;
    load_main_res = 1;
    sprite_add(c + font_resource, &text_pos, 0, 0, text_tag);
    load_main_res = 0;
    text_pos.x += char_width;
  }
}

/** 
 * Печать строки на текущей позиции
 * 
 * @param c строка
 */
void print_string(char *c)
{
  while (*c)
    print_char(*c++);
}

/** 
 * Команда - печать сохраненной строки
 */
void print_get_string()
{
  init_stack();
  switch_string_get();
#ifdef DEBUG
  printf("print string: %s\n", get_string);
#endif
  print_string(get_string);
}

/** 
 * Команда - печать числа
 */
void print_number()
{
  char str[MAX_STR];
  new_get();
#ifdef DEBUG
  printf("print number: %d\n", current_value);
#endif
  sprintf(str, "%d", current_value);
  print_string(str);
}
