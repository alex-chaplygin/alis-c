/**
 * @file   key.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 16:52:27 2022
 * 
 * @brief  Работа с клавиатурой
 * 
 */

#include <stdio.h>
#include <SDL.h>
#include <SDL2/SDL_keycode.h>
#include "types.h"

byte key_symbol;

int key_mod = 0;		/**< нажатые модификаторы */

/** 
 * Обработка события нажатия клавиши
 * 
 * @param scan скан код клавиши
 * @param mod модификаторы Alt Ctrl Shift
 */
void set_key(int scan, int sym, int mod)
{
  key_symbol = sym;
  key_mod = mod;
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
}

/** 
 * Обработка события отпускания клавиши
 * 
 * @param scan скан код клавиши
 */
void release_key(int scan)
{
  key_symbol = 0;
}

/// возвращает символ нажатой клавишу или 0 - если не нажата
byte get_key()
{
  return key_symbol;
}

/// возвращает состояние клавиш Shift, Alt, Ctrl
byte get_key_mod()
{
  return (byte)(key_mod & 0x1f); /**< все Shift Alt Ctrl */
}

byte wait_for_key()
{
  SDL_Event e;

  while (1)
    if (SDL_PollEvent(&e))
      if (e.type == SDL_KEYDOWN) {
	set_key(e.key.keysym.scancode, e.key.keysym.sym, e.key.keysym.mod);
	break;
      }
  return get_key();
}
