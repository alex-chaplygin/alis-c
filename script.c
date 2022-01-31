/**
 * @file   script.c
 * @author alex <alex@localhost>
 * @date   Mon Jan 31 09:15:01 2022
 * 
 * @brief  Загрузка сценариев
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "file.h"
#include "threads.h"
#include "memory.h"
#include "sprite.h"
#include "interpret.h"
#include "get.h"

#define MAIN_TYPE 0		/**< главный сценарий */

#pragma pack(1)

/// заголовок io файла
typedef struct {
  word uncompressed_size;	/**< распакованный размер + 1 бит из flags */
  word flags;			/**< сжатый или нет */
  word type;			/**< 0 - главный сценарий */
} io_header_t;

/// заголовок главного файла сценариев
typedef struct {
  word total_scripts;		/**< всего сценариев */
  word total_threads;		/**< число одновременных потоков */
  int size;			/**< размер для теста памяти: не используется */
  int total_memory_size;	/**< сумма всех сегментов данных сценариев */
  int total_images;		/**< максимальное число одновременных изображений (спрайтов) */
} main_header_t;

FILE *io_file;			/**< текущий файл сценария */
int *script_sizes;		/**< таблица размеров файлов */
byte **script_table = 0;	/**< таблица загруженных сценариев */
int total_scripts;		/**< всего сценариев */
int num_scripts;		/**< число загруженных сценариев */
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
  if (!io_file) {
    fprintf(stderr, "Read bits: no file\n");
    exit(1);
  }
  if (feof(io_file))
    return 0;
  while (num > 0) {
    if (!bitcount) {
      cur_byte = fgetc(io_file);
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
  file_read(io_file, dict, sizeof(dict));
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
 * Загружает заголовок главного сценария
 * Инициализация памяти, потоков, спрайтов
 */
void load_main_script()
{
  main_header_t mh;

  file_read(io_file, &mh, sizeof(mh));
#ifdef DEBUG
    printf("Total scripts = %d\nTotal threads = %d\ntotal file size = %d\ntotal memory = %d\ntotal images = %d\n",
	   mh.total_scripts, mh.total_threads, mh.size, mh.total_memory_size, mh.total_images);
#endif
    total_scripts = mh.total_scripts;
    num_scripts = 0;
    script_table = xmalloc(total_scripts * sizeof(byte *));
    script_sizes = xmalloc(total_scripts * sizeof(int *));
    memset(script_table, 0, total_scripts * sizeof(byte *));
    thread_init_table(mh.total_threads);
    memory_init(mh.total_memory_size);
    sprites_init(mh.total_images);
}

/** 
 * Проверка был ли загружен сценарий
 * 
 * @param id номер сценария
 * 
 * @return позицию в таблице сценариев или -1, если не найдено
 */
int script_loaded(word id)
{
  if (!script_table)
    return -1;
  for (int i = 0; i < num_scripts; i++)
    if (*(word *)script_table[i] == id)
      return i;
  return -1;
}

/** 
 * Загрузка сценария
 * 
 * @param id внутренний номер сценария, 0 - главный сценарий
 * @param name имя файла сценария
 */
void script_load(int id, char *name)
{
  io_header_t h;
  int size;
  byte *script;

  if (script_loaded(id) != -1)
    return;
  io_file = file_open(name, "rb");
  file_read(io_file, &h, sizeof(h));
#ifdef DEBUG
  printf("Uncompressed size = %d flags = %x type = %d\n", h.uncompressed_size, h.flags, h.type);
#endif
  size = h.uncompressed_size + (h.flags & 0xff << 16);
  if (id == 0 || h.type == MAIN_TYPE)  {
    load_main_script();
    size -= 16;
  }
  script = xmalloc(size);
  num_scripts++;
  script_table[num_scripts - 1] = script;
  script_sizes[num_scripts - 1] = size;
  if ((h.flags >> 8 & 0xfe) == 0xa0)
    uncompress(script, size);
  fclose(io_file);
  if (id == 0)
    thread_setup_main(script, size);
}

/// команда: загрузка сценария
void op_script_load()
{
  word id = fetch_word();
#ifdef DEBUG
  printf("load script %x %s\n", id, current_ip);
#endif
  if (id) {
    script_load(id, current_ip);
    while (*current_ip++);
    return;
  }
  printf("load new main.io: not implemented\n");
  exit(1);
}

/// команда: запуск сценария
void run_script()
{
  word id = fetch_word();
  int i = script_loaded(id);
  if (i == -1) {
    printf("Script %x (total %d) is not loaded\n", id, total_scripts);
    exit(1);
  }
#ifdef DEBUG
  printf("run script %x size = %d\n", id, script_sizes[i]);
#endif
  thread_t *t = thread_add(script_table[i], script_sizes[i]);
  switch_string_store();
}
