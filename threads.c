/**
 * @file   threads.c
 * @author alex <alex@localhost>
 * @date   Thu Apr  7 10:23:17 2022
 * 
 * @brief  Модуль работы с потоками
 * 
 */
#include "types.h"
#include "threads.h"
#include "io.h"
#include "sprites.h"
#include "view.h"
#include "vm.h"

word *threads_list_pos;		/**< указатель на массив номеров потоков */
word threads_list[256];		/**< список номеров потоков */
int max_threads;		/**< максимальное число потоков */
int loaded_threads;		/**< число загруженных потоков */
threads_table_t *threads_table;	/**< таблица потоков */
thread_t *current_thread;	/**< текущий выполняемый поток */
int thread_flag;
int no_save;

void threads_init()
{
  threads_list_pos = threads_list;
}

/** 
 * Запуск потоков
 * Цикл по всем потокам из таблицы потоков
 * Запускается обработка сообщения, если есть сообщения, они не запрещены и у потока есть обработчик
 * Основная программа (вместе с обработчиком клавиш) потока не запускается, если он на паузе или не на своем кадре
 * В основной программе меняются ip и указатель стека, обработка сообщений и клавиш начинается всегда с одного места
 * После запуска виртуальной машины происходит перемещение спрайтов относительно нового начала координат
 * После того как все потоки отработали происходит обновление экрана
 */
void threads_run()
{
  threads_table_t *tt;
  int ip;
  word sp;
  io_header_t *h;
  for (tt = threads_table; tt; tt = tt->next) {
    current_thread = tt->thread;
    thread_flag = 0;
    no_save = 0;
    h = (io_header_t *)current_thread->io_file;
    if (current_thread->flags & THREAD_MSG && !(current_thread->flags & THREAD_NOSTART3) && h->message_entry) {
      sp = current_thread->call_sp;
      sprites_set_translate((word *)current_thread->memory);
      vm_run(current_thread->io_file + h->message_entry + 0xa);
      current_thread->call_sp = sp;
      sprites_translate(current_thread->sprites, (word *)current_thread->memory);
    }
    if (!current_thread->running)
      continue;
    if (current_thread->running < 0)
      current_thread->running = 1;
    --current_thread->frame_num;
    if (current_thread->frame_num)
      continue;
    sprites_set_translate((word *)current_thread->memory);
    no_save++;
    current_thread->ip = vm_run(current_thread->io_file + current_thread->ip) - current_thread->io_file;
    no_save = 0;
    if (h->key_entry) {
      sp = current_thread->call_sp;
      vm_run(current_thread->io_file + h->key_entry);
      current_thread->call_sp = sp;
    }
    sprites_translate(current_thread->sprites, (word *)current_thread->memory);
    current_thread->frame_num = current_thread->num_frames;
  }
  view_update();
}

/** 
 * Возвращает свободное число ячеек в таблице потоков
 */
int threads_free_num()
{
  return max_threads - loaded_threads;
}
