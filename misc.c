/**
 * @file   misc.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 15:26:58 2022
 * 
 * @brief  Интерпретатор - прочие функции
 * 
 */

#include <stdio.h>
#include "interpret.h"
#include "class.h"
#include "file.h"
#include "key.h"

/// возвращает число свободных ресурсов
void get_free()
{
#ifdef DEBUG
  printf("get free resources %d: ", current_value);
#endif
  switch (current_value) {
  case 0:  current_value = 720; break; // свободная память в кб
  case 1:  current_value = 720; break; // число свободной памяти потоков в кб
  case 2: // число свободных сценариев
    current_value = total_classes - num_classes; break; 
  case 3: // число свободных потоков
    current_value = max_objects - num_run_objects; break; 
  case 4:
    current_value = num_free_sprites(); break; // число свободных спрайтов
  case 0x41:
  case 0x61:
    current_value = 1024; break; // свободных байт на диске A в кб
  case 0x42:
  case 0x62:
    current_value = 1024; break; // свободных байт на диске B в кб
  case 0x43:
  case 0x63:
    current_value = 1024; break; // свободных байт на диске C в кб
  case 0x44:
  case 0x64:
    current_value = 1024; break; // свободных байт на диске D в кб
  default:
    current_value = -1;
  }
#ifdef DEBUG
  printf("%d\n", current_value);
#endif
}

/// возвращает номер VGA порта
void get_video_port()
{
  current_value = 0x7d4;
#ifdef DEBUG
  printf("get video port: %x\n", current_value);
#endif
}

/// возвращает букву диска
void get_drive()
{
#ifdef DEBUG
  printf("get drive\n");
#endif
  current_value = (short)'C';
}

/// возвращает тип клавиатуры
void get_keyboard()
{
#ifdef DEBUG
  printf("get keyboard type\n");
#endif
  current_value = 1;
}

/// возвращает скорость процессора
void get_cpu_speed()
{
#ifdef DEBUG
  printf("get cpu speed\n");
#endif
  current_value = 100;
}

/// установка аккумулятора в 0
void set_0()
{
  current_value = 0;
}

/// возвращает флаги аппаратуры: клавиатура, мышь
void get_hardware()
{
  current_value = 0x4;
#ifdef DEBUG
  printf("get hardware: %x\n", current_value);
#endif
}

/** 
 * Пропуск 4-х параметров
 */
void skip4()
{
#ifdef DEBUG
  printf("skip4\n");
#endif
  new_get();
  new_get();
  new_get();
  new_get();
}
