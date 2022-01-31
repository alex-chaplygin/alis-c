/**
 * @file   threads.c
 * @author alex <alex@localhost>
 * @date   Mon Jan 31 09:25:35 2022
 * 
 * @brief  Функции для работы с потоками
 * Потоки организованы с кооперативной многозадачностью: 
 * в указанных местах передают управление на следующий поток
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threads.h"
#include "script.h"
#include "interpret.h"
#include "render.h"

thread_table_t *threads_table; /**< таблица потоков */
int max_threads;		/**< максимальное количество потоков */
thread_table_t *free_thread;	/**< голова списка свободных потоков */
thread_table_t *current_thread;	/**< текущий поток */
thread_t *main_thread;		/**< главный поток */
int num_run_threads;		/**< число рабочих потоков */
int no_saved_return;		/**< если 0, то позволяет возврат из сценария к сохраненному кадру стека */

/** 
 * Загрузка blancpc
 * Загрузка главного сценария
 */
void thread_init()
{
  load_blancpc();
  script_load(0, "MAIN.IO");
}

/** 
 * Инициализация таблицы потоков
 * Потоки организуются в список, в начале только один главный поток,
 * остальные - свободны
 * @param max - максимум потоков
 */
void thread_init_table(int max)
{
  max_threads = max;
  threads_table = xmalloc(max * sizeof(thread_table_t));
  thread_table_t *t = threads_table;
  for (int i = 0; i < max_threads - 1; i++) {
    t->next = t + 1;
    t->thread = 0;
    t->run_next = 0;
    t++;
  }
  t->next = 0;
  threads_table->next = 0;
  free_thread = threads_table + 1; // главный поток сразу запущен
  num_run_threads = 1;
  get_string = get_string_buf;
  string2 = string_buf2;
  store_string = store_string_buf;
  string4 = string_buf4;
}

/** 
 * Настройка потока для запуска сценария
 * Создается стек вызовов, стек параметров, сегмент данных для 
 * переменных, поток запускается.
 * @param script загруженный образ сценария
 * @param size размер образа
 */
void thread_setup(thread_table_t *tb, byte *script, int size)
{
  script_t *h = (script_t *)script;
  thread_t *t = xmalloc(sizeof(thread_t));
  memset(t, 0, sizeof(thread_t));
  tb->thread = t;
#ifdef DEBUG
  printf("Setup thread: id = %x, size = %d, stack = %d, data = %d, param = %d\n",
	 h->id, size, h->stack_size, h->data_size, h->param_size);
#endif
  t->call_stack = stack_new(h->stack_size);
  t->param_stack = stack_new(h->param_size);
  t->data = memory_alloc(h->data_size);
  t->ip = script + h->entry + 2;
  t->script = script;
  t->id = h->id;
  t->version = h->version;
  t->frames_to_skip = t->cur_frames_to_skip = 1;
  t->running = 0xff;
  t->flags = 0xffff;
  t->state = STATE_START3; // bit 1
  t->header = h;
  t->f2c = 0;
}

/// запуск главного потока
void thread_setup_main(byte *script, int size)
{
  thread_setup(threads_table, script, size);
  main_thread = threads_table->thread;
}

/** 
 * Добавление нового потока
 * Новый поток добавляется в конец списка потоков.
 * Текущий поток становится родителем нового.
 * Новый поток наследует трансформацию перемещения.
 * Новый поток начинает работу
 * @param script образ сценария
 * @param size размер сценария
 * 
 * @return 
 */
thread_t *thread_add(byte *script, int size)
{
  thread_table_t *next = current_thread->next;
  thread_table_t *new_thread = free_thread;
  current_thread->next = new_thread;
  free_thread = new_thread->next;
  new_thread->next = next;
  num_run_threads++;
  if (num_run_threads == max_threads) {
    printf("Max threads reached %d\n", max_threads);
    exit(1);
  }
  thread_setup(new_thread, script, size);
  current_value = (int)(new_thread - threads_table) * 6;
  thread_t *t = new_thread->thread;
  memcpy(t->data->data, run_thread->data->data, 6); /**< трансформация из текущего потока копируется в новый поток */
  memcpy(t->data->data + 9, run_thread->data->data + 9, 3); /**< копируются 3 байта начиная с 9-го */
  t->parent = run_thread;
  // добавления вектора перемещения к началу координат
  // нового потока
  t->current_scene = run_thread->current_scene;
  //   t->f22 = run_thread->f22
  // t->thread_table2 = t->thread_table
#ifdef DEBUG
  word *o = (word *)t->data->data;
  byte *o2 = t->data->data + 9;
  printf("Add thread %d origin (%d %d %d) data9 (%d %d %d)\n",
	 num_run_threads - 1, o[0], o[1], o[2], o2[0], o2[1], o2[2]);
#endif
}

/** 
 * Главный цикл потоков. Для всех потоков с состоянием запуска
 * запускается интерпретатор. Учитывается параметр пропуска
 * кадров перед тем как начать интерпретацию. Всего в потоке может
 * быть 3 сценария: 1-й основной, 2-й - дополнительный (запускается
 * всегда), 3-й - запускается если установлен соответствующий флаг.
 * После того как все потоки прошли цикл происходит обновление 
 * графики.
 */
void threads_run()
{
  thread_t *t;
  for (current_thread = threads_table; current_thread; ) {
    t = current_thread->thread;
#ifdef DEBUG
    printf("Run thread %x ip = %x frames_to_skip = %d cur_frames_to_skip = %d running = %x\n", t->id, (int)t->ip, t->frames_to_skip, t->cur_frames_to_skip, t->running);
#endif
    no_saved_return = 0;
    if (t->state & STATE_FLAG7) // bit 7
      if (t->state & STATE_START3) // bit 1
	if (t->header->entry3) {
	  t->saved_sp = t->call_stack->sp;
	  set_translate((word *)t->data->data);
	  interpret(t, t->script + t->header->entry3 + 0xa);
	  t->call_stack->sp = t->saved_sp;
	  sprites_translate((word *)t->data->data);
	}
    if (t->running != 0) {
      if ((char)t->running < 0)
	t->running = 1;
      t->cur_frames_to_skip--;
      if (!t->cur_frames_to_skip) {
	set_translate((word *)t->data->data);
	no_saved_return++;
	t->ip = interpret(t, t->ip);
	if (t->header->entry2) {
#ifdef DEBUG
	  printf("starting entry2: %x\n", t->header->entry2);
#endif
	  no_saved_return = 0;
	  t->saved_sp = t->call_stack->sp;
	  interpret(t, t->script + t->header->entry2);
	  t->call_stack->sp = t->saved_sp;
	}
	sprites_translate((word *)t->data->data);
	t->cur_frames_to_skip = t->frames_to_skip;
      }
    }
    current_thread = current_thread->next;
  }
  render_update();
}

/// команда - запрет на запуск сценария 3 в потоке
void thread_no_start3()
{
  run_thread->state &= ~STATE_START3;
#ifdef DEBUG
  printf("thread no start3: %x\n", run_thread->state);
#endif
}

/// очистка флага 0 в состоянии текущего потока
void thread_clear_state0()
{
  run_thread->state &= ~STATE_FLAG0;
#ifdef DEBUG
  printf("clear state 0: %x\n", run_thread->state);
#endif
}