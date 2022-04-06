/**
 * @file   vm.c
 * @author alex <alex@localhost>
 * @date   Wed Apr  6 18:07:27 2022
 * 
 * @brief  Интерпретатор виртуальной машины
 */

#define FREE_MEM 1024		/**< свободная память */
#define CPU_SPEED 0xca		/**< фиксированная скорость */
#define HARDWARE_FLAG 0x84	/**< флаги оборудования */
#define VIDEO_PORT 0x7d4	/**< VGA порт */

short accum;			/**< регистр аккумулятора */

/** 
 * возвращает число свободных ресурсов
 */
voi get_free()
{
  switch (accum) {
  case 0:  accum = FREE_MEM; break; // свободная память в кб
  case 1:  accum = FREE_MEM; break; // число свободной памяти потоков в кб
  case 2: // число свободных сценариев
    accum = scripts_free_num(); break; 
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
