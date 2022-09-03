/**
 * @file   io.c
 * @author alex <alex@localhost>
 * @date   Fri Aug  5 17:13:03 2022
 * 
 * @brief  Работа с IO файлом. Разпаковка.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "file.h"
#include "class.h"
#include "memory.h"

#define MAIN_TYPE 0		/**< главный класс */

#pragma pack(1)

/// заголовок io файла
typedef struct {
  word uncompressed_size;	/**< распакованный размер + 1 бит из flags */
  word flags;			/**< сжатый или нет */
  word type;			/**< 0 - главный класс */
} io_header_t;

extern FILE *handle;			/**< текущий файл */
static byte cur_byte;		/**< текущий прочитанный байт */
static byte bitcount = 0;	/**< счетчик бит */

/** 
 * чтение бит из текущего файла
 * 
 * @param num число бит
 * 
 * @return биты из потока
 */
word get_bits(int num)
{
  word bits = 0;
  //  printf("get_bits %d", num);
  if (feof(handle))
    return 0;
  while (num > 0) {
    if (!bitcount) {
      cur_byte = fgetc(handle);
      bitcount = 8;
      //printf("cur_byte = %x count = %d\n", cur_byte, count);
    }
    bits <<= 1;
    bits += cur_byte >> 7;
    //printf("cur_byte = %x bits = %x count = %d\n", cur_byte, bits, count);
    cur_byte <<= 1;
    bitcount--;
    num--;
  }
  //  printf(" %x\n", bits);
  return bits;
}

/** 
 * чтение количества последовательностей
 * 
 * @param num число бит
 * @param cond условие останова
 * 
 * @return количество
 */
int read_count(int num, int cond)
{
  int count = 0;
  int bits;
  
  while (1) {
    bits = get_bits(num);
    count += bits;
    if (bits != cond)
      break;
  }
  return count;
}

/** 
 * Разпаковка потока из файла
 * Вариант кодирования LZ77: часть байт - не сжатые,
 * часть упаковывается через длину и смещение к предыдущим байтам
 * @param out адрес буфера, куда распаковывается
 * @param size размер буфера
 */
void uncompress(byte *out, int size)
{
  byte dict[8];
  byte *o = out;
  int i;
  int count;
  int bits;
  int num;
  int offset;

  bitcount = 0;
  file_read(dict, sizeof(dict));
  while (o < out + size) {
    i = get_bits(1);
    if (i) {
      count = read_count(2, 3) + 1;
      for (i = 0; i < count; i++)
	*o++ = get_bits(8);
      if (o >= out + size)
	return;
    }
    bits = get_bits(3);
    num = dict[bits];
    bits &= 3;
    if (!bits) {
      offset = get_bits(num);
      count = read_count(3, 7) + 4;
    } else {
      count = bits;
      offset = get_bits(num);
    }
    count++;
    for (i = 0; i < count; i++) {
      bits = *(o - offset - 1);
      *o++ = bits;
      if (o >= out + size)
	return;
    }
  }
}

/** 
 * Разпаковка потока из файла
 * Вариант кодирования RLE
 * @param out адрес буфера, куда распаковывается
 * @param size размер буфера
 */
void uncompress_rle(byte *out, int size)
{
  byte *o = out;
  byte b;
  byte d;
  int count;
  while (o < out + size) {
    file_read(&b, 1);
    if ((char)b < 0) {
      count = b & 0x7f;
      file_read(&d, 1);
      memset(o, d, count);
    } else {
      count = b;
      file_read(o, count);
    }
    o += count;
  }
}

/** 
 * Загрузка IO файла
 * 
 * @param id номер класса
 * @param name имя файла
 * @param size возвращается размер буфера
 * @return буфер
 */
byte *io_load(int id, char *name, int *size)
{
    io_header_t h;
    byte *data;
    extern FILE *handle;
    file_open(name, FILE_RW);
    file_read(&h, sizeof(h));
    *size = h.uncompressed_size + ((h.flags & 0xff) << 16);
#ifdef DEBUG
    printf("IO load uncompressed size = %d flags = %x type = %d\n", *size, h.flags, h.type);
#endif
    if (id == 0 || h.type == MAIN_TYPE)  {
      load_main_class();
      *size -= 16;
    }
    data = xmalloc(*size);
    if ((h.flags >> 8 & 0xfe) == 0xa0)
      uncompress(data, *size);
    else if ((h.flags >> 8 & 0xf0) == 0x80){
      uncompress_rle(data, *size);
    } else {
      printf("Unknown type %x\n", h.flags);
      exit(1);
    }
    file_close();
    return data;
}
