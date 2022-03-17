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
#include <stdlib.h>
#include "file.h"
#include "types.h"

#define BLANCPC "blancpc.io"	/**< имя файла сценария */
#define BLANCPC_SIZE 2500	/**< запакованный размер */
#define BLANCPC_USIZE 2500	/**< разпакованный размер */
#define BLANCPC_HEADER 54	/**< размер заголовка - незакодированные данные */

byte buffer[BLANCPC_SIZE];	/**< буфер сценария - зашифрованный */
byte blancpc_buffer[2560]; /**< разшифрованный */
byte *blancpc_data;

int sar(int a, int c);

void load_blancpc()
{
  byte *src;
  byte *dst;
  FILE *f;
  file_open(BLANCPC, FILE_RW);
  file_read(buffer, BLANCPC_SIZE);
  file_close();
  blancpc_data = buffer + BLANCPC_HEADER;
  src = blancpc_data;
  dst = blancpc_buffer + BLANCPC_HEADER;
  for (int i = 0; i < 116; i++)
    *dst++ = *src++;
  for (int i = 0; i < 2444; i++) {
    byte b = *src++;
    if (b) {
      b -= 32;
      byte b2 = (b >> 1) + (b & 0x80);
      b = b2;
      b2 = (b >> 1) + (b & 0x80);
      b = b2 + 32;
    }
    *dst++ = b;
  }
#ifdef DEBUG
  printf("loaded blancpc\n");
  for (int i = 0; i < 2560 / 16; i++) {
    for (int j = 0; j < 16; j++)
      printf("%02x ", blancpc_buffer[i * 16 + j]);
    printf("\n");
  }
#endif
}
