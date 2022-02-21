#ifndef __THREADS__
#define __THREADS__

#include "types.h"
#include "memory.h"
#include "sprite.h"

#define STATE_FLAG0 (1 << 0)
#define STATE_START3 (1 << 1)	/**< запуск сценария 3 каждый фрейм */
#define STATE_MSG (1 << 7) /**< есть сообщения для потока */

#pragma pack(1)

/// заголовок сценария
typedef struct {
  word id;			/**< номер сценария */
  word entry;			/**< адрес начала сценария */
  byte control;
  byte version;			/**< версия сценария */
  word entry2;
  byte u1;
  byte u2;
  word entry3;
  byte u4;
  byte u5;
  dword resources;		/**< адрес таблицы ресурсов */
  word stack_size;		/**< размер стека вызовов */
  word data_size;		/**< размер сегмента данных */
  word msg_size;		/**< размер стека сообщений */
} script_t;

/// структура потока
typedef struct thread_s {
  int id;			/**< номер сценария */
  stack_t *call_stack;		/**< стек вызовов */
  int *saved_sp;		/**< сохраненный указатель стека */
  stack_t *msg_stack;		/**< стек сообщений потоку */
  seg_t *data;			/**< сегмент данных */
  byte *ip;			/**< текущий указатель команд */
  byte *script;			/**< образ сценария (содержит код и ресурсы) */
  int version;			/**< версия сценария */
  byte frames_to_skip;		/**< число кадров через сколько выполняется сценарий */
  byte cur_frames_to_skip;		/**< текущий отсчет кадров для выполнения */
  byte running;			/**< поток запущен */
  word flags;			/**< флаги потока */
  int state;			/**< состояние потока */
  int x_flip;			/**< если 1, то все спрайты будут повернуты по горизонтали */
  int layer;			/**< слой отрисовки для всех новых спрайтов */
  int f2c;
  sprite_t *sprite_list;	/**< список спрайтов потока */
  scene_t *current_scene;		/**< текущая сцена */
  script_t *header;		/**< заголовок сценария */
  struct thread_s *parent;	/**< родительский поток */
} thread_t;

/// запись таблицы потоков
typedef struct threads_table_s {
  thread_t *thread;		/**< структура потока */
  struct threads_table_s *run_next; /**< следующий поток на запуск */
  struct threads_table_s *next;	/**< следующий поток в списке в запуске */
} thread_table_t;

void thread_init();
void thread_init_table();
void thread_setup_main(byte *script, int size);
void thread_setup(thread_table_t *tb, byte *script, int size);
thread_t *thread_add(byte *script, int size);
void threads_run();

void thread_no_start3();
void thread_clear_state0();
void thread_send_message();

extern int max_threads;		/**< максимальное количество потоков */
extern int num_run_threads;		/**< число рабочих потоков */
extern thread_t *main_thread;		/**< главный поток */

#endif
