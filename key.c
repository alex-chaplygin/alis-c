/**
 * @file   key.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 16:52:27 2022
 * 
 * @brief  Работа с клавиатурой
 * 
 */

#include <stdio.h>
#include <SDL2/SDL_keycode.h>
#include "types.h"
#include "interpret.h"

char key_symbol;		/**< символ текущей нажатой клавиши */
int key_mod = 0;		/**< нажатые модификаторы */

/** 
 * Обработка события нажатия клавиши.
 * Преобразует SDL scan код в символ
 * 
 * @param scan скан код клавиши
 * @param mod модификаторы Alt Ctrl Shift
 */
void set_key(int scan, int sym, int m)
{
  key_symbol = sym;
  key_mod = m;
  if (scan >= SDL_SCANCODE_F1 && scan <= SDL_SCANCODE_F10)
    key_symbol = 0xbb + scan - SDL_SCANCODE_F1;
  else if (scan == SDL_SCANCODE_KP_8)
    key_symbol = '8';
  else if (scan == SDL_SCANCODE_KP_2)
    key_symbol = '2';
  else if (scan == SDL_SCANCODE_KP_4)
    key_symbol = '4';
  else if (scan == SDL_SCANCODE_KP_6)
    key_symbol = '6';
  else if (scan == SDL_SCANCODE_KP_7)
    key_symbol = '7';
  else if (scan == SDL_SCANCODE_KP_1)
    key_symbol = '1';
  else if (scan == SDL_SCANCODE_KP_9)
    key_symbol = '9';
  else if (scan == SDL_SCANCODE_KP_3)
    key_symbol = '3';
  else if (scan == SDL_SCANCODE_KP_0)
    key_symbol = '0';
  else if (scan == SDL_SCANCODE_ESCAPE)
    key_symbol = 0x1b;
  else if (scan == SDL_SCANCODE_BACKSPACE)
    key_symbol = 0x8;
  else if (scan == SDL_SCANCODE_TAB)
    key_symbol = 0x9;
  else if (scan == SDL_SCANCODE_RETURN)
    key_symbol = 0x0d;
#ifdef DEBUG
  printf("press key %x '%c'; %x\n", scan, key_symbol, key_symbol);
#endif
}

/** 
 * Обработка события отпускания клавиши
 * 
 * @param scan скан код клавиши
 */
void release_key(int scan)
{
  key_symbol = 0;
#ifdef DEBUG
  printf("release key %x\n", scan);
#endif
}

/// возвращает символ нажатой клавишу или 0 - если не нажата
void get_key()
{
  current_value = (byte)key_symbol;
#ifdef DEBUG
  printf("get key: %x\n", current_value);
#endif
}

/// возвращает состояние кнопок джойстика
void get_joy()
{
  current_value = 0;
#ifdef DEBUG
  printf("get joy: %x\n", current_value);
#endif
}

/// возвращает состояние клавиш Shift, Alt, Ctrl
void get_keyboard_mod()
{
  current_value = (byte)(key_mod & 0x1f); /**< все Shift Alt Ctrl */
#ifdef DEBUG
  printf("get key_flags: %x\n", current_value);
#endif
}
