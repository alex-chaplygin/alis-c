/**
 * @file   key.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 16:52:27 2022
 * 
 * @brief  Работа с клавиатурой
 * 
 */

#include <stdio.h>
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
  't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',	/* Enter key */
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
void set_key(int scan, int mod)
{
  byte s = (scan & 0x7F) - 1;
  keypressed_table[s] = 1;
  key_symbol = symbol_table[s];
  key_mod = mod;
  //  if (scan == 29) ctrl_pressed = 1;
  //  if (s < 112) key_pressed = key_table[s];
  //printf("set key %x %c\n", scan, key_pressed);
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
