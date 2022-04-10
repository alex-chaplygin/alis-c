/**
 * @file   vm.c
 * @author alex <alex@localhost>
 * @date   Wed Apr  6 18:07:27 2022
 * 
 * @brief  Интерпретатор виртуальной машины
 */
#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "threads.h"
#include "sprites.h"
#include "video.h"

#define FREE_MEM 1024		/**< свободная память */
#define CPU_SPEED 0xca		/**< фиксированная скорость */
#define HARDWARE_FLAG 0x84	/**< флаги оборудования */
#define VIDEO_PORT 0x7d4	/**< VGA порт */
#define MAX_STR 100

#define LODB (*(char *)current_ip++) /**< чтение байта */
#define LODW (*(short *)current_ip++) /**< чтение слова */

short accum;			/**< регистр аккумулятора */
char get_string_buf[MAX_STR];	/**< буфер строки загрузки */
char text_string_buf[MAX_STR];
char store_string_buf[MAX_STR];	/**< буфер строки сохранения */
char *get_string;		/**< указатель строки для загрузки */
char *text_string;		/**< указатель на строку для вывода */
char *store_string;		/**< указатель строки сохранения */
byte *current_ip;		/**< текущий указатель команд */

/** 
 * возвращает число свободных ресурсов
 */
void get_free()
{
  switch (accum) {
  case 0:  accum = FREE_MEM; break; // свободная память в кб
  case 1:  accum = FREE_MEM; break; // число свободной памяти потоков в кб
  case 2: // число свободных сценариев
    accum = io_num_free_files(); break; 
  case 3: // число свободных потоков
    accum = threads_free_num(); break; 
  case 4:
    accum = sprites_free_num(); break; // число свободных спрайтов
  case 'a':
  case 'A':
    accum = FREE_MEM; break; // свободных байт на диске A в кб
  case 'b':
  case 'B':
    accum = FREE_MEM; break; // свободных байт на диске B в кб
  case 'c':
  case 'C':
    accum = FREE_MEM; break; // свободных байт на диске C в кб
  case 'd':
  case 'D':
    accum = FREE_MEM; break; // свободных байт на диске D в кб
  default:
    accum = -1;
  }
}

void vm_init()
{
  get_string = get_string_buf;
  text_string = text_string_buf;
  store_string = store_string_buf;
}

/** 
 * Интерпретатор виртуальной машины
 * 
 * @param ip начальный указатель команд
 * 
 * @return указатель команд при завершении интерпретации
 */
byte *vm_run(byte *ip)
{
  byte op;
  current_ip = ip;
  while (1) {
    video_update_events();
    op = *current_ip++;
    printf("op = %x\n", op);
    switch (op) {
    case 0: case 1: case 2: case 3: case 4: break;
    case 5: current_ip = threads_call(LODB, current_ip + 1); break;
    case 6: current_ip = threads_call(LODW, current_ip + 2); break;
    case 7: current_ip = threads_call(LODW, current_ip + 3); break;
    case 8: current_ip += LODB; break;
    case 9: current_ip += LODW; break;
    case 0xa: current_ip += LODW + 1; break;
    case 0x42: goto out;
    default:
      printf("Unknown vm op: %x\n", op);
      exit(1);
    }
  }
 out:
  return current_ip;
}
