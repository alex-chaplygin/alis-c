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
#include "store.h"

thread_table_t *threads_table; /**< таблица потоков */
int max_threads;		/**< максимальное количество потоков */
thread_table_t *free_thread;	/**< голова списка свободных потоков */
thread_table_t *current_thread;	/**< текущий поток */
thread_t *main_thread;		/**< главный поток */
int num_run_threads;		/**< число рабочих потоков */
int no_saved_return;		/**< если 0, то позволяет возврат из сценария к сохраненному кадру стека */
int thread_flag = 0;
word threads_list[256];		/**< список номеров потоков */
int kill_thread_flag = 1;		/**< если равен 0, то потоки не удаляются при освобождении сценария*/

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
 * Отладочная печать потоков
 */
void dump_threads()
{
  printf("threads: ");
  thread_table_t *t = threads_table;
  thread_t *th;
  while (t) {
    th = t->thread;
    if (!th) break;
    printf("table id: %x ", (int)(t - threads_table) * 6);
    printf("id: %x\n", th->id);
    printf("data:\n");
    dump_mem(th->data->data, th->data->size);
    printf("\n");
    t = t->next;
  }
  /*  
Ошибка : thread 48 class 41 adr: 16 must: 05 is: 00
thr: 1e class: 4 adr: 25 must 05 is 0
thr: 6 class: 1f
must
0000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 00  ................
0010: 00 01 16 02 01 02 00 0D 00 01 00 02 00 00 00 00  ................
0020: 00 00 46 52 41 47 4F 4E 49 52 00 00 00 00 00 00  ..FRAGONIR......
0030: FF FF
is:
0000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 
0010: 00 00 16 02 01 02 00 0d 00 01 00 03 00 00 00 00 
0020: 00 00 46 52 41 47 4f 4e 49 52 00 00 00 00 00 00 
0030: ff ff


printf("\n");
  printf("free threads: ");
  t = free_thread;
  while (t->next) {
    printf("->%x", (int)(t->next - threads_table) * 6);
    t = t->next;
  }
  printf("\n");*/
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
#ifdef DEBUG
  dump_threads();
#endif
  get_string = get_string_buf;
  text_string = text_string_buf;
  store_string = store_string_buf;
  threads_list_pos = threads_list;
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
  t->script = t->script2 = script;
  t->id = h->id;
  t->version = h->version;
  t->frames_to_skip = t->cur_frames_to_skip = 1;
  t->running = -1;
  t->flags2 = -1;
  t->flags = THREAD_NOSTART3; // bit 1
  t->header = h;
  t->f2c = 0;
  t->sprites_thread = 0;
  t->f26 = -1;
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
 * @param translate вектор перемещения
 * 
 * @return 
 */
thread_t *thread_add(byte *script, int size, vec_t *translate)
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
  // добавления вектора перемещения к началу координат нового потока
  short *coord = (short *)t->data->data;
  coord[0] += translate->x;
  coord[1] += translate->y;
  coord[2] += translate->z;
  t->current_scene = run_thread->current_scene;
  t->f22 = run_thread->f22;
  t->sprites_thread = current_value;
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
    printf("Run thread %x ip = %x frames_to_skip = %d cur_frames_to_skip = %d running = %x flags = %x\n", t->id, (int)(t->ip - t->script), t->frames_to_skip, t->cur_frames_to_skip, t->running, t->flags);
#endif
    thread_flag = no_saved_return = 0;
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
      if (t->running < 0)
	t->running = 1;
      t->cur_frames_to_skip--;
      if (!t->cur_frames_to_skip) {
	set_translate((word *)t->data->data);
	no_saved_return++;
	t->ip = interpret(t, t->ip);
#ifdef DEBUG
	printf("ip = %x\n", (int)(t->ip - t->script));
#endif
	if (interpreting == 2) {
	  current_thread = current_thread->next;
	  continue;
	}
	if (t->header->entry2) {
#ifdef DEBUG
	  printf("starting entry2: %x\n", t->header->entry2);
#endif
	  no_saved_return = 0;
	  t->saved_sp = t->call_stack->sp;
	  interpret(t, t->script + t->header->entry2 + 6);
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

/// команда - разрешение потоку принимать сообщения
void thread_ready_to_receive()
{
  run_thread->flags &= ~THREAD_NOMSG;
#ifdef DEBUG
  printf("thread ready to receive msg flags: %x\n", run_thread->flags);
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
  if (!t) {
    printf("thread is NULL\n");
    exit(1);
  }
  if (t->flags & THREAD_NOMSG) {
    printf("thread flag = no send\n");
    exit(1);
  }
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
  thread_t *t = threads_table[num / 6].thread;
  if (!t)
    return;
  thread_t *rt = run_thread;
#ifdef DEBUG
  printf("kill thread %x\n", *t->script);
  dump_threads();
#endif
  run_thread = t;
  remove_all_sprites(t->sprite_list, remove);
  run_thread = rt;
  // освобождение ресурсов
  stack_free(t->call_stack);
  stack_free(t->msg_stack);
  memory_free(t->data);
  thread_table_t *tab = threads_table->next;
  thread_table_t *prev = threads_table;
  while (tab) {
    if (tab->thread == t)
      break;
    prev = tab;
    tab = tab->next;
  }
  num_run_threads--;
  prev->next = tab->next;
  free(tab->thread);
  tab->thread = 0;
  tab->next = free_thread;
  free_thread = tab;
  if (run_thread == t) {
    interpreting = 2;
    // должен запуститься следующий поток или render
    current_thread = prev;
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
  if (stack_empty(run_thread->msg_stack)) {
    printf("get message stack is empty\n");
    exit(1); 
    current_value = -1;
  } else {
    current_value = stack_pop(run_thread->msg_stack);
    if (stack_empty(run_thread->msg_stack))
      run_thread->flags &= ~THREAD_MSG;
  }
  #ifdef DEBUG
  printf("get_message: %x; %d\n", current_value, current_value);
  printf("flags = %x\n", run_thread->flags);
  #endif
}

/** 
 * Пристановка выполнения основного скрипта потока
 * передача управления только если без сохранения стека вызовов
 */
void thread_pause_yield_no_saved()
{
#ifdef DEBUG
  printf("thread pause no saved yield\n");
#endif
  run_thread->running = 0;
  if (no_saved_return)
    yield();
}

/// Очищает стек сообщений потока
void thread_clear_messages()
{
  stack_clear(run_thread->msg_stack);
  run_thread->flags &= ~THREAD_MSG;
#ifdef DEBUG
  printf("clear messages flags = %x\n", run_thread->flags);
#endif
}

/** 
 * Останавливает все потоки сценария
 * 
 * @param id номер сценария
 */
void kill_thread_by_script(int id)
{
  int i;
  for (thread_table_t *t = threads_table->next; t;) {
#ifdef DEBUG
    printf("kill thread check = %x\n", t->thread->id);
#endif
    exit(1);
    if (t->thread->id == id && kill_thread_flag)
      thread_kill(thread_num(t->thread), 0);
    t = t->next;
  }
}

/** 
 * Останавливает текущий поток
 * Если текущий поток - главный, то - выход
 */
void thread_stop()
{
#ifdef DEBUG
  printf("thread_stop\n");
  printf("run thread = %x\n", *run_thread->script);
#endif
  if (run_thread == threads_table->thread)
    exit(0);
  thread_kill(thread_num(run_thread), 0);
}

/** 
 * Продолжает выполнение потока
 * Параметр - номер потока
 */
void thread_resume()
{
  new_get();
  if (current_value < 0)
    return;
  int thr = current_value;
  if (thr % 6 != 0) {
    printf("thread resume invalid thread: %x\n", thr);
    exit(1);
  }
  thread_t *t = threads_table[thr / 6].thread;
#ifdef DEBUG
  printf("resume thread %x num = %x\n", *t->script, current_value);
#endif
  t->running = 1;
}

/** 
 * Вычисляет номер потока в таблице потоков * 6
 * 
 * @param t указатель потока
 * 
 * @return номер в таблице * 6
 */
int thread_num(thread_t *t)
{
  thread_table_t *tab = threads_table;
  for (int i = 0; tab; i++, tab++)
    if (tab->thread == t)
      return i * 6;
  return -1;
}

/** 
 * Сохраняет очередной элемент списка потоков в переменную
 */
void store_thread_num()
{
  current_value = *(short *)threads_list_pos;
  if (current_value >= 0)
    threads_list_pos++;
  #ifdef DEBUG
  printf("store thread num: %x\n", current_value);
  #endif
  exchange_strings_store();
}

/** 
 * Читает номер сценария и ищет все потоки, которые выполняют этот
 * сценарий. Номера потоков записываются в список
 * и затем один поток сохраняется в переменную
 */
void script_num_to_thread_num()
{
  int num = fetch_word();
  thread_table_t *tab = threads_table;
  thread_t *t;
  word *pos = threads_list;
#ifdef DEBUG
  printf("script num: %x to thread num\n", num);
#endif
  while (tab) {
    t = tab->thread;
#ifdef DEBUG
    printf("check thread num: %x run_thread: %x\n", t->id, run_thread->id);
#endif
    if (t->id == num && t != run_thread) {
      pos[255] = 0;
      *pos = (word)thread_num(t);
#ifdef DEBUG
  printf("thread num: %x\n", *pos);
#endif
      pos++;
      if (!thread_flag)
	break;
      else {
	printf("thread flag\n");
	exit(1);
      }
    }
    tab = tab->next;
  }
  *pos = (short)-1;
  thread_flag = 0;
  threads_list_pos = threads_list;
  store_thread_num();
}

void set_thread_f25()
{
#ifdef DEBUG
  printf("set thread f25 -1\n");
#endif
  run_thread->f25 = -1;
}

/** 
 * Установка номера потока, которому будут принадлежать новые спрайты
 */
void set_sprites_thread()
{
  new_get();
#ifdef DEBUG
  printf("set sprites thread: %x prev: %x\n", current_value, run_thread->sprites_thread);
#endif
  run_thread->sprites_thread = current_value;
}

/** 
 * Загружает список номеров всех потоков кроме главного
 * Первый элемент списка сохраняет в переменную
 */
void get_threads_list()
{
  thread_table_t *tab = threads_table->next;
  thread_t *t;
  word *pos = threads_list;
  int num;
#ifdef DEBUG
  printf("get threads list:\n");
#endif
  while (tab) {
    t = tab->thread;
    num = thread_num(t);
    *pos++ = num;
#ifdef DEBUG
    printf("%x ", num);
#endif
    tab = tab->next;
  }
#ifdef DEBUG
    printf("\n");
#endif
  *pos = -1;
  thread_flag = 0;
  threads_list_pos = threads_list;
  store_thread_num();
}

void thread_set_flags2()
{
  new_get();
  run_thread->flags2 = current_value;
#ifdef DEBUG
  printf("set flags2 = %x\n", current_value);
#endif
}

void thread_set_flag()
{
  thread_flag = 1;
#ifdef DEBUG
  printf("set flag = 1\n");
#endif
}

/** 
 * Команда - установка слоя для новых объектов.
 * Объекты при отрисовке будут сортироваться по убыванию слоев.
 */
void set_thread_layer()
{
  new_get();
#ifdef DEBUG
  printf("set thread layer = %x; %d\n", (char)current_value, (char)current_value);
#endif
  run_thread->layer = (char)current_value;
}

void op_kill_thread()
{
#ifdef DEBUG
  printf("kill thread\n");
#endif
  op_thread_kill(0);
}
