/**
 * @file   blancpc.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 12:30:00 2022
 * 
 * @brief  Сценарий - пустой компьютер
 * 
 * 
 */

#include <stdio.h>
#include "file.h"
#include "types.h"

#define BLANCPC "BLANCPC.IO"	/**< имя файла сценария */
#define BLANCPC_SIZE 2500	/**< размер */
#define BLANCPC_HEADER 54	/**< размер заголовка - незакодированные данные */

byte buffer[BLANCPC_SIZE];	/**< буфер сценария - зашифрованный */
byte blancpc_buffer[BLANCPC_SIZE]; /**< разшифрованный */
byte *blancpc_data;

void load_blancpc()
{
  byte *src;
  byte *dst;
  FILE *f = file_open(BLANCPC, "rb");
  file_read(f, buffer, BLANCPC_SIZE);
  fclose(f);
  blancpc_data = buffer + BLANCPC_HEADER;
  src = blancpc_data;
  dst = blancpc_buffer;
  for (int i = 0; i < 116; i++)
    *dst++ = *src++;
  for (int i = 0; i < 2444; i++) {
    byte b = *src++;
    if (b) {
      b -= 32;
      b = (char)b / 4;
      b += 32;
    }
    *dst++ = b;
  }
}
