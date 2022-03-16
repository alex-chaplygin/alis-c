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

char key_symbol;
bool keypressed_table[256];

int key_mod = 0;		/**< нажатые модификаторы */

char qwerty_caps[] = {
  0x0F, 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P',
  '¦', '*', 0x0D, '¦', 'A', 'S', 'D', 'F', 'G', 'H', 'J',
  'K', 'L', ':', '%', 'Ь', '¦', '>', 'Y', 'X', 'C', 'V',
  'B', 'N', 'M', '<', '>', '?', 0xFE, 0
};

char qwerty_chars[] = {
  '\t', 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p',
  '^', '$', 0x0D, '¦', 'a', 's', 'd', 'f', 'g', 'h', 'j',
  'k', 'l', ';', 'Ч', 'ц', '¦', '<', 'y', 'x', 'c',
  'v', 'b', 'n', 'm', ';', '.', '/'
};

byte symbol_table[128] =
{
   0x1b, '&', 'B', '"', 0x27, '(', '`', 'K', '!',	/* 9 */
  '3', 'E', ')', '-', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '^', '$', 0x0d,	/* Enter key */
    '|',			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   '|',		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   '|',				/* Right shift */
  '*',
    '|',	/* Alt */
  ' ',	/* Space bar */
    '|',	/* Caps lock */
    0xbb,	/* 59 - F1 key ... > */
    0xbc,   0xbd,   0xbe,   0xbf,   0xc0,   0xc1,   0xc2,   0xc3,
    0xc4,	/* < ... F10 */
    0xff,	/* 69 - Num lock*/
    0xff,	/* Scroll Lock */
    0x37,	/* Home key */
    '8',	/* Up Arrow */
    '9',	/* Page Up */
  '-',
    '4',	/* Left Arrow */
    '5',
    '6',	/* Right Arrow */
  '+',
    '1',	/* 79 - End key*/
    '2',	/* Down Arrow */
    '3',	/* Page Down */
   '0',	/* Insert Key */
    '.',	/* Delete Key */
    0xff,   0xff,   0xff,
    0xff,	/* F11 Key */
    0xff,	/* F12 Key */
    0xff,	/* All other keys are undefined */
};

/** 
 * Обработка события нажатия клавиши
 * 
 * @param scan скан код клавиши
 * @param mod модификаторы Alt Ctrl Shift
 */
void set_key(int scan, int sym, int mod)
{
  byte s = (scan & 0x7F) - 1;
  keypressed_table[s] = 1;
  key_symbol = sym;//symbol_table[s];
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
#ifdef DEBUG
  printf("press key %x s = %x '%c'; %x\n", scan, s, key_symbol, key_symbol);
#endif
}

/** 
 * Обработка события отпускания клавиши
 * 
 * @param scan скан код клавиши
 */
void release_key(int scan)
{
  byte s = (scan & 0x7F) - 1;
  keypressed_table[s] = 0;
  key_symbol = 0;
#ifdef DEBUG
  printf("release key %x %x\n", scan, s);
#endif
}

/// возвращает символ нажатой клавишу или 0 - если не нажата
void get_key()
{
  // get pressed key
  current_value = key_symbol;
#ifdef DEBUG
  printf("get key: %x\n", current_value);
#endif
}

/// возвращает состояние кнопок джойстика
void get_joy()
{
  // get pressed key
  current_value = 0;
#ifdef DEBUG
  printf("get joy: %x\n", current_value);
#endif
}
