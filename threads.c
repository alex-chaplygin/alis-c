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
#include "get.h"

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
  printf("Setup thread: id = %x, size = %d, stack = %d, data = %d, msg = %d\n",
	 h->id, size, h->stack_size, h->data_size, h->msg_size);
#endif
  t->call_stack = stack_new(h->stack_size);
  t->msg_stack = stack_new(h->msg_size);
  t->data = memory_alloc(h->data_size);
  t->ip = script + h->entry + 2;
  t->script = script;
  t->id = h->id;
  t->version = h->version;
  t->frames_to_skip = t->cur_frames_to_skip = 1;
  t->running = 0xff;
  t->flags2 = 0xffff;
  t->flags = THREAD_NOSTART3; // bit 1
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
    printf("Run thread %x ip = %x frames_to_skip = %d cur_frames_to_skip = %d running = %x flags = %x\n", t->id, (int)t->ip, t->frames_to_skip, t->cur_frames_to_skip, t->running, t->flags);
#endif
    no_saved_return = 0;
    if (t->flags & THREAD_MSG) // bit 7
      if (!(t->flags & THREAD_NOSTART3)) // bit 1
	if (t->header->entry3) {
#ifdef DEBUG
	  printf("starting handle message: %x\n", t->header->entry3 + 0xa);
#endif
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

/// команда - разрешение обработки сообщений
void thread_receive_msg()
{
  run_thread->flags &= ~THREAD_NOSTART3;
#ifdef DEBUG
  printf("thread receive msg flags: %x\n", run_thread->flags);
#endif
}

/// очистка флага 0 в состоянии текущего потока
void thread_clear_flags0()
{
  run_thread->flags &= ~THREAD_FLAG0;
#ifdef DEBUG
  printf("clear flags 0: %x\n", run_thread->flags);
#endif
}

/** 
 * Передает сообщение потоку через стек сообщений.
 * Сообщение - это определенное количество данных,
 * которые заносятся в стек.
 */
void thread_send_message()
{
  int count = fetch_byte() + 1;
  new_get();
  if (current_value == -1) {
    printf("thread_state thread == -1\n");
    exit(1);
  }
#ifdef DEBUG
  printf("send message thread: %d count: %d\n", current_value, count);
#endif
  thread_t *t = threads_table[current_value / 6].thread;
  while (count--) {
    new_get();
#ifdef DEBUG
    printf("param = %x; %d\n", current_value, current_value);
#endif
    stack_push(t->msg_stack, current_value);
  }
  t->flags |= THREAD_MSG;
#ifdef DEBUG
    printf("flags = %x\n", t->flags);
#endif
}

/** 
 * Удаление потока
 * 
 * @param num номер потока * 6
 * @param remove если 1, то полное удаление спрайтов
 */
void thread_kill(int num, int remove)
{
  if (num < 0)
    return;
  thread_t *t = threads_table[current_value / 6].thread;
  if (!t)
    return;
  thread_t *rt = run_thread;
#ifdef DEBUG
  printf("kill thread %x\n", *t->script);
#endif
  run_thread = t;
  remove_all_sprites(t->sprite_list, remove);
  run_thread = rt;
  // освобождение ресурсов
  stack_free(t->call_stack);
  stack_free(t->msg_stack);
  memory_free(t->data);
  thread_table_t *tab = threads_table;
  thread_table_t *cur;
  for (int i = 0; i < num_run_threads; i++, tab++)
    if (tab->next->thread == t)
      break;
  num_run_threads--;
  cur = tab->next;
  tab->next = cur->next;
  cur->next = free_thread;
  free_thread = cur;
  if (run_thread == t) {
    printf("kill thread run_thread == t\n");
    exit(1);
    run_thread = tab->thread;
  }
}

/** 
 * Удаление потока, номер - параметр
 * 
 * @param remove если 1, то полное удаление спрайтов
 */
void op_thread_kill(int remove)
{
  save_get();
  if (current_value <= 0) {
    printf(" thread <= 0\n");
    exit(1);
    return;
  }
  thread_kill(current_value, remove);
}

/// удаление потока с очисткой всех изображений
void op_thread_kill_remove_all()
{
#ifdef DEBUG
  printf("thread kill remove all\n");
#endif
  op_thread_kill(1);
}

/** 
 * Читает сообщение для текущего потока
 * Сбрасывает флаг, если больше нет сообщений
 */
void get_message()
{
  stack_t *s = run_thread->msg_stack;
  if (stack_empty(run_thread->msg_stack))
    current_value = -1;
  else {
    current_value = stack_pop(run_thread->msg_stack);
    if (stack_empty(run_thread->msg_stack))
      run_thread->flags &= ~THREAD_MSG;
  }
  #ifdef DEBUG
  printf("get_message: %x; %d\n", current_value, current_value);
  printf("flags = %x\n", run_thread->flags);
  #endif
}
