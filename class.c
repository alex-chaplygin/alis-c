/**
 * @file   script.c
 * @author alex <alex@localhost>
 * @date   Mon Jan 31 09:15:01 2022
 * 
 * @brief  Модуль работы с классами
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "file.h"
#include "objects.h"
#include "io.h"
#include "sprite.h"
#include "interpret.h"
#include "get.h"

/// заголовок программы главного класса
typedef struct {
  word total_classes;		/**< всего классов */
  word total_objects;		/**< максимум объектов */
  int size;			/**< размер для теста памяти: не используется */
  int total_memory_size;	/**< сумма всех данных объектов */
  int total_sprites;		/**< максимальное число спрайтов */
} main_header_t;

int *class_sizes;		/**< таблица размеров файлов */
byte **class_table = 0;	/**< таблица загруженных классов */
int total_classes;		/**< всего классов */
int num_classes;		/**< число загруженных классов */

/** 
 * Загружает главный класс
 * Инициализация памяти, потоков, спрайтов
 */
void load_main_class()
{
  main_header_t mh;

  file_read(&mh, sizeof(mh));
#ifdef DEBUG
    printf("Total classes = %d\nTotal objects = %d\ntotal file size = %d\ntotal memory = %d\ntotal sprites = %d\n",
	   mh.total_classes, mh.total_objects, mh.size, mh.total_memory_size, mh.total_sprites);
#endif
    total_classes = mh.total_classes;
    num_classes = 0;
    class_table = xmalloc(total_classes * sizeof(byte *));
    class_sizes = xmalloc(total_classes * sizeof(int *));
    memset(class_table, 0, total_classes * sizeof(byte *));
    objects_init_table(mh.total_objects);
    memory_init(mh.total_memory_size);
    sprites_init(mh.total_sprites);
}

/** 
 * Проверка был ли загружен класс
 * 
 * @param id номер класса
 * 
 * @return позицию в таблице классов или -1, если не найдено
 */
int class_loaded(word id)
{
  if (!class_table)
    return -1;
  for (int i = 0; i < num_classes; i++)
    if (*(word *)class_table[i] == id)
      return i;
  return -1;
}

/** 
 * Загрузка класса из IO файла
 * 
 * @param id внутренний номер класс, 0 - главный класс
 * @param name имя файла класса
 */
void class_load(int id, char *name)
{
  byte *data;
  int size;
  if (class_loaded(id) != -1)
    return;
  data = io_load(id, name, &size);
  num_classes++;
  class_table[num_classes - 1] = data;
  class_sizes[num_classes - 1] = size;
  if (id == 0)
    object_setup_main(data, size);
}

/// команда: загрузка класса
void op_class_load()
{
  word id = fetch_word();
#ifdef DEBUG
  printf("load class %x %s\n", id, current_ip);
#endif
  if (id) {
    class_load(id, current_ip);
    while (*current_ip++);
    return;
  }
  printf("load new main.io: not implemented\n");
  exit(1);
}

byte *class_get(int i)
{
  return class_table[i];
}

int class_size(int i)
{
  return class_sizes[i];
}

/** 
 * Команда - удаление класса
 * Освобождается память, удаляются все объекты этого класса
 */
void class_free()
{
  int num = fetch_word();
#ifdef DEBUG
  printf("class free: %x\n", num);
  printf("cur class id = %x\n", run_object->id);
#endif
  if (num == -1 || num == run_object->id)
    return;
  num = class_loaded(num); // позиция в таблице сценариев
  if (num == -1)
    return;
#ifdef DEBUG
  printf("free class pos = %d\n", num);
#endif
  // play sound
  free(class_table[num]);
  for (int i = num; i < num_classes; i++)
    if (i + 1 == total_classes) {
      class_table[i] = 0;
      class_sizes[i] = 0;
    } else {
      class_table[i] = class_table[i + 1];
      class_sizes[i] = class_sizes[i + 1];
    }
  num_classes--;
  objects_kill_by_class(num);
  window_free_sprites();
}
